#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "UAX.h"

using namespace UAX;
using namespace Bidi;

struct BidiAlgorithm {
  const uint32_t *text;
  long length;
  EmbeddingLevel paragraph_embedding_level;
  EmbeddingLevel *resolved_embedding_levels;
  struct Metadata { // 12 bytes per character
    int matching_index; // -1 for none, valid for matching brackets, isolate initiators <-> PDIs
    int next_isolating_run_sequence; // index of first character in next seq (-1 if none), makes a linked-list of IRSs
    Bidi_Class bidi_class;
    unsigned is_isolate_bridge:1; // 1 if isolate initiator or PDI with matching index (can use relative direction of matching_index to determine if initiator or PDI)
    unsigned sos:1; // sos+eos are set for the first character in each IRS (0 and all the linked characters chained by next_isolating_run_sequence). L=0, R=1
    unsigned eos:1;
  } *metadata;

  void run(const Codepoint *_text, const size_t _length, const BaseDirection base_direction, Metadata *_metadata, EmbeddingLevel &resolved_paragraph_embedding_level, EmbeddingLevel *resolved_embedding_levels);

  struct IsolatingRunSequenceIterator;
  void Initializaton();
  void The_Paragraph_Level(const BaseDirection base_direction);
  void Explicit_Levels_and_Directions();
  void Preparations_for_Implicit_Processing();
  void Resolving_Isolating_Run_Sequences();
  void Resolving_Weak_Types(IsolatingRunSequenceIterator &iterator);
  void Resolving_Neutral_and_Isolate_Formatting_Types(IsolatingRunSequenceIterator &iterator);
  void Resolving_Implicit_Levels(IsolatingRunSequenceIterator &iterator);
  
  Bidi_Class embedding_direction_for_embedding_level(EmbeddingLevel level) const;
  int irs_skip_ignores(int i, const int direction) const;
  int irs_step(EmbeddingLevel irs_embedding_level, int i, const int direction, int &did_jump, EmbeddingLevel &found_bordering_level) const;
  void assign_bracket_pairs(const int irs_start);
  void append_next_isolating_run_sequence(int head, int next);
  void prepare_isolating_run_sequence(int irs_start);
  EmbeddingLevel paragraph_embedding_level_for_strong_character_index(const int first_strong_index) const;
  int find_first_strong_index(const int irs_start, const int end_index) const;
  
  #if UAX_BIDI_ENABLE_DEBUG_TRACE
  #define DEBUG_TRACE(...) trace(__VA_ARGS__)
  bool debug_trace = false;
  void trace(const char *step, bool headers) const;
  #else
  #define DEBUG_TRACE(...)
  #endif
};

static const EmbeddingLevel EMBEDDING_LEVEL_IGNORE = 255;
static const EmbeddingLevel MAX_DEPTH = 125;

bool Bidi::RequiresAlgorithm(const Codepoint *text, const size_t length) {
  for (int i = 0; i < (int)length; ++i) {
    switch (Get_Bidi_Class(text[i])) {
      case Bidi_Class::Right_To_Left:
      case Bidi_Class::Right_To_Left_Embedding:
      case Bidi_Class::Right_To_Left_Override:
      case Bidi_Class::Right_To_Left_Isolate:
      case Bidi_Class::Arabic_Letter:
        return true;
      default:
        break;
    }
  }
  return false;
}

size_t Bidi::ScratchBufferSize(const size_t text_length) {
  return text_length * sizeof(BidiAlgorithm::Metadata);
}

void Bidi::Run(const Codepoint *text, const size_t length, const BaseDirection base_direction, EmbeddingLevel &resolved_paragraph_embedding_level, EmbeddingLevel *resolved_embedding_levels, void *scratch_buffer
#if UAX_BIDI_ENABLE_DEBUG_TRACE
               , bool debug_trace
#endif
               ) {
  BidiAlgorithm a;
  #if UAX_BIDI_ENABLE_DEBUG_TRACE
  a.debug_trace = debug_trace;
  #endif
  a.run(text, length, base_direction, (BidiAlgorithm::Metadata *)scratch_buffer, resolved_paragraph_embedding_level, resolved_embedding_levels);
}

void BidiAlgorithm::run(const uint32_t *_text, const size_t _length, const BaseDirection base_direction, Metadata *_metadata, EmbeddingLevel &resolved_paragraph_embedding_level, EmbeddingLevel *_resolved_embedding_levels) {
  if (_length < 1) {
    resolved_paragraph_embedding_level = 0;
    return;
  }
  text = _text;
  length = _length;
  metadata = _metadata;
  resolved_embedding_levels = _resolved_embedding_levels;
  Initializaton();
  The_Paragraph_Level(base_direction);    DEBUG_TRACE("Initializaton+The_Paragraph_Level", true);
  Explicit_Levels_and_Directions();       DEBUG_TRACE("Explicit_Levels_and_Directions", false);
  Preparations_for_Implicit_Processing(); DEBUG_TRACE("Preparations_for_Implicit_Processing", false);
  Resolving_Isolating_Run_Sequences();    DEBUG_TRACE("Resolving_Isolating_Run_Sequences", false);
  resolved_paragraph_embedding_level = paragraph_embedding_level;
}

#define BIDI_CLASS(I) metadata[I].bidi_class
#define EMBEDDING_LEVEL(I) resolved_embedding_levels[I]
#define MATCHING_INDEX(I) metadata[I].matching_index
#define IS_ISOLATE_BRIDGE(I) metadata[I].is_isolate_bridge
#define NEXT_ISOLATING_RUN_SEQUENCE(I) metadata[I].next_isolating_run_sequence
#define SOS(I) metadata[I].sos
#define EOS(I) metadata[I].eos
#define IGNORE_BY_X9(I) (BIDI_CLASS(I) == Bidi_Class::Boundary_Neutral)

void BidiAlgorithm::Initializaton() {
  int initiators[MAX_DEPTH];
  int count = 0;
  int overflow = 0;
  for (int i = 0; i < length; ++i) {
    BIDI_CLASS(i) = Get_Bidi_Class(text[i]);
    EMBEDDING_LEVEL(i) = 0;
    IS_ISOLATE_BRIDGE(i) = 0;
    MATCHING_INDEX(i) = -1;
    NEXT_ISOLATING_RUN_SEQUENCE(i) = -1;
    if (Is_Isolate_Initiator(BIDI_CLASS(i))) {
      if (count < MAX_DEPTH) {
        initiators[count] = i;
        ++count;
      } else {
        ++overflow;
      }
    } else if (BIDI_CLASS(i) == Bidi_Class::Pop_Directional_Isolate) {
      if (overflow > 0) {
        --overflow;
      } else {
        if (count > 0) {
          IS_ISOLATE_BRIDGE(i) = 1;
          IS_ISOLATE_BRIDGE(initiators[count - 1]) = 1;
          MATCHING_INDEX(i) = initiators[count - 1];
          MATCHING_INDEX(initiators[count - 1]) = i;
          --count;
        }
      }
    }
  }
}

void BidiAlgorithm::The_Paragraph_Level(const BaseDirection base_direction) {
  switch (base_direction) {
    case BaseDirection::Auto:  paragraph_embedding_level = paragraph_embedding_level_for_strong_character_index(find_first_strong_index(0, int(length)-1)); break; // P2, P3
    case BaseDirection::Left:  paragraph_embedding_level = 0; break; // HL1 override
    case BaseDirection::Right: paragraph_embedding_level = 1; break; // HL1 override
    default: paragraph_embedding_level = 0; break;
  }
}

void BidiAlgorithm::Explicit_Levels_and_Directions() { // 3.3.2
  enum class DirectionalOverrideStatus {
    Neutral,
    RightToLeft,
    LeftToRight,
  };
  struct {
    struct Item {
      EmbeddingLevel embedding_level;
      DirectionalOverrideStatus directional_override_status;
      bool directional_isolate_status;
    } stack[MAX_DEPTH + 2];
    int count;
    inline void set_empty() { count = 0; }
    inline const Item &top() const { return stack[count - 1]; }
    inline EmbeddingLevel next_embedding_level(int even_odd) const { // even_odd: 0=even, 1=odd
      int n = top().embedding_level;
      return even_odd ? ((n+1)|1) : ((n+2)&~1);
    }
    inline void push(EmbeddingLevel embedding_level, DirectionalOverrideStatus directional_override_status, bool directional_isolate_status) {
      assert(count < MAX_DEPTH + 2);
      stack[count] = Item {
        .embedding_level = embedding_level,
        .directional_override_status = directional_override_status,
        .directional_isolate_status = directional_isolate_status,
      };
      ++count;
    }
    inline void pop() {
      assert(count > 0);
      --count;
    }
  } directional_status_stack;
  
  directional_status_stack.set_empty(); // X1
  directional_status_stack.push(paragraph_embedding_level, DirectionalOverrideStatus::Neutral, false);
  int overflow_isolate_count = 0;
  int overflow_embedding_count = 0;
  int valid_isolate_count = 0;

  #define VALID_EMBEDDING_LEVEL(E) (((E) <= MAX_DEPTH) && (overflow_embedding_count == 0) && (overflow_isolate_count == 0))

  for (int i = 0; i < length; ++i) { // X2-X8
    int even_odd;
    DirectionalOverrideStatus directional_override_status;
    
    switch (BIDI_CLASS(i)) {
      case Bidi_Class::Right_To_Left_Embedding: // X2
        even_odd = 1;
        directional_override_status = DirectionalOverrideStatus::Neutral;
        goto Handle_Embedding_and_Overrides;
      case Bidi_Class::Left_To_Right_Embedding: // X3
        even_odd = 0;
        directional_override_status = DirectionalOverrideStatus::Neutral;
        goto Handle_Embedding_and_Overrides;
      case Bidi_Class::Right_To_Left_Override: // X4
        even_odd = 1;
        directional_override_status = DirectionalOverrideStatus::RightToLeft;
        goto Handle_Embedding_and_Overrides;
      case Bidi_Class::Left_To_Right_Override: // X5
        even_odd = 0;
        directional_override_status = DirectionalOverrideStatus::LeftToRight;
        goto Handle_Embedding_and_Overrides;
      case Bidi_Class::Right_To_Left_Isolate: treat_FSI_as_RLI: // X5a
        even_odd = 1;
        goto Handle_Isolate;
      case Bidi_Class::Left_To_Right_Isolate: treat_FSI_as_LRI: // X5b
        even_odd = 0;
        goto Handle_Isolate;
      case Bidi_Class::First_Strong_Isolate: // X5c
        if (paragraph_embedding_level_for_strong_character_index(find_first_strong_index(i+1, MATCHING_INDEX(i)-1)) == 1)
          goto treat_FSI_as_RLI;
        else
          goto treat_FSI_as_LRI;
        
      default: // X6. For all types besides B, BN, RLE, LRE, RLO, LRO, PDF, RLI, LRI, FSI, and PDI:
        EMBEDDING_LEVEL(i) = directional_status_stack.top().embedding_level;
        switch (directional_status_stack.top().directional_override_status) {
          case DirectionalOverrideStatus::Neutral: break; // do nothing
          case DirectionalOverrideStatus::LeftToRight: BIDI_CLASS(i) = Bidi_Class::Left_To_Right; break; // override character type
          case DirectionalOverrideStatus::RightToLeft: BIDI_CLASS(i) = Bidi_Class::Right_To_Left; break; // override character type
        }
        break;
        
      case Bidi_Class::Pop_Directional_Isolate: // X6a
        if (overflow_isolate_count > 0) { // this PDI matches an overflow isolate initiator
          --overflow_isolate_count;
        } else if (valid_isolate_count == 0) { // this PDI does not match any isolate initiator, valid or overflow
          // do nothing
        } else { // this PDI matches a valid isolate initiator
          overflow_embedding_count = 0;
          while (directional_status_stack.top().directional_isolate_status == false) {
            directional_status_stack.pop();
          }
          directional_status_stack.pop();
          --valid_isolate_count;
        }
        EMBEDDING_LEVEL(i) = directional_status_stack.top().embedding_level;
        break;
        
      case Bidi_Class::Pop_Directional_Format: // X7
        if (overflow_isolate_count > 0) {
          // do nothing
        } else if (overflow_embedding_count > 0) {
          --overflow_embedding_count;
        } else if ((directional_status_stack.top().directional_isolate_status == false) && (directional_status_stack.count >= 2)) {
          directional_status_stack.pop();
        } else {
          // do nothing
        }
        break;
        
      case Bidi_Class::Paragraph_Separator: // X8
        EMBEDDING_LEVEL(i) = paragraph_embedding_level;
        break;
      case Bidi_Class::Boundary_Neutral: // needed so 'default:' doesn't catch BN
        break; // ignore
        
      Handle_Embedding_and_Overrides: {
        auto next_embedding_level = directional_status_stack.next_embedding_level(even_odd);
        if (VALID_EMBEDDING_LEVEL(next_embedding_level)) {
          directional_status_stack.push(next_embedding_level, directional_override_status, false);
        } else {
          if (overflow_isolate_count == 0)
            ++overflow_embedding_count;
        }
        break;
      }
      Handle_Isolate: {
        EMBEDDING_LEVEL(i) = directional_status_stack.top().embedding_level;
        auto next_embedding_level = directional_status_stack.next_embedding_level(even_odd);
        if (VALID_EMBEDDING_LEVEL(next_embedding_level)) {
          directional_status_stack.push(next_embedding_level, DirectionalOverrideStatus::Neutral, true);
          ++valid_isolate_count;
        } else {
          ++overflow_isolate_count;
        }
        break;
      }
    }
  }
}

void BidiAlgorithm::Preparations_for_Implicit_Processing() { // 3.3.3
  for (int i = 0; i < length; ++i) { // X9
    switch (BIDI_CLASS(i)) {
      case Bidi_Class::Right_To_Left_Embedding:
      case Bidi_Class::Left_To_Right_Embedding:
      case Bidi_Class::Right_To_Left_Override:
      case Bidi_Class::Left_To_Right_Override:
      case Bidi_Class::Pop_Directional_Format:
      case Bidi_Class::Boundary_Neutral:
        BIDI_CLASS(i) = Bidi_Class::Boundary_Neutral; // 5.2 - Retaining Explicit Formatting Characters - reclassify to BN
        EMBEDDING_LEVEL(i) = EMBEDDING_LEVEL_IGNORE;
        break;
      default:
        break;
    }
  }
}

struct BidiAlgorithm::IsolatingRunSequenceIterator {
  const BidiAlgorithm &bidi;
  const BidiAlgorithm::Metadata *metadata;
  const EmbeddingLevel *resolved_embedding_levels;
  const Bidi_Class sos, eos;
  const int direction;
  int index, reset_index;
  EmbeddingLevel embedding_level;
  Bidi_Class previous_type, current_type, last_strong_type;
  bool NI_context;
  IsolatingRunSequenceIterator(const BidiAlgorithm &_bidi, int _index, int _direction, Bidi_Class _sos = Bidi_Class::Left_To_Right, Bidi_Class _eos = Bidi_Class::Left_To_Right): bidi(_bidi), metadata(_bidi.metadata), resolved_embedding_levels(_bidi.resolved_embedding_levels), sos(_sos), eos(_eos), direction(_direction), NI_context(false) {
    reset_index = bidi.irs_skip_ignores(_index, direction);
    reset();
  }
  void reset() {
    index = reset_index;
    if (!end()) {
      previous_type = sos;
      last_strong_type = sos;
      embedding_level = EMBEDDING_LEVEL(index);
      current_type = BIDI_CLASS(index);
    }
  }
  bool end() const {
    return index == -1;
  }
  void next() {
    if (end()) return;
    current_type = BIDI_CLASS(index); // current_type may have been mutated after the previous step, grab a fresh one here
    if (Is_Strong(current_type))
      last_strong_type = current_type;
    if (NI_context && Is_Strong_in_NI_context(current_type))
      last_strong_type = NI_influencing_direction(current_type);
    
    int ignore_jump;
    EmbeddingLevel ignore_level;
    index = bidi.irs_step(embedding_level, index, direction, ignore_jump, ignore_level);
    if (end()) return;
    
    previous_type = current_type;
    current_type = BIDI_CLASS(index);
  }
  template<typename F> void each(F f) { // may be called after a next()
    while (!end()) {
      if (!f()) break;
      next();
    }
  }
  template<typename F> void all(F f) {
    reset();
    each([&]{ f(); return true; });
  }
  template<typename F> void lookahead(F f) const {
    IsolatingRunSequenceIterator iterator(bidi, index, direction, sos, eos);
    iterator.next(); // step over current
    iterator.each([&]{ return f(iterator); });
  }
};

void BidiAlgorithm::Resolving_Isolating_Run_Sequences() { // X10
  int irs_start = 0;
  do prepare_isolating_run_sequence(irs_start); // will set SOS, EOS and NEXT_ISOLATING_RUN_SEQUENCE
  while ((irs_start = NEXT_ISOLATING_RUN_SEQUENCE(irs_start)) != -1);
  
  irs_start = 0;
  do {
    IsolatingRunSequenceIterator iterator(*this, irs_start, +1, SOS(irs_start) ? Bidi_Class::Right_To_Left : Bidi_Class::Left_To_Right, EOS(irs_start) ? Bidi_Class::Right_To_Left : Bidi_Class::Left_To_Right);
    Resolving_Weak_Types(iterator);                             DEBUG_TRACE("Resolving_Weak_Types", false);
    Resolving_Neutral_and_Isolate_Formatting_Types(iterator);   DEBUG_TRACE("Resolving_Neutral_and_Isolate_Formatting_Types", false);
    Resolving_Implicit_Levels(iterator);                        DEBUG_TRACE("Resolving_Implicit_Levels", false);
  } while ((irs_start = NEXT_ISOLATING_RUN_SEQUENCE(irs_start)) != -1);
}

void BidiAlgorithm::Resolving_Weak_Types(IsolatingRunSequenceIterator &iterator) { // W1-W7
  iterator.all([&]{ // W1
    if (iterator.current_type == Bidi_Class::Nonspacing_Mark) {
      if (Is_Isolate_Initiator(iterator.previous_type) || (iterator.previous_type == Bidi_Class::Pop_Directional_Isolate)) {
        BIDI_CLASS(iterator.index) = Bidi_Class::Other_Neutral;
      } else {
        BIDI_CLASS(iterator.index) = iterator.previous_type;
      }
    }
  });
  iterator.all([&]{ // W2
    if (iterator.current_type == Bidi_Class::European_Number) {
      if (iterator.last_strong_type == Bidi_Class::Arabic_Letter) {
        BIDI_CLASS(iterator.index) = Bidi_Class::Arabic_Number;
      }
    }
  });
  iterator.all([&]{ // W3
    if (iterator.current_type == Bidi_Class::Arabic_Letter) {
      BIDI_CLASS(iterator.index) = Bidi_Class::Right_To_Left;
    }
  });
  iterator.all([&]{ // W4
    Bidi_Class next_type = iterator.eos;
    iterator.lookahead([&](IsolatingRunSequenceIterator &lookahead) {
      if (!IS_ISOLATE_BRIDGE(lookahead.index)) {
        next_type = lookahead.current_type;
        return false; // stop
      }
      return true;
    });
    if (iterator.current_type == Bidi_Class::European_Separator && iterator.previous_type == Bidi_Class::European_Number && next_type == Bidi_Class::European_Number) {
      BIDI_CLASS(iterator.index) = Bidi_Class::European_Number;
    } else if (iterator.current_type == Bidi_Class::Common_Separator && iterator.previous_type == next_type) { // common separator surrounded by two of the same type
      if (iterator.previous_type == Bidi_Class::European_Number || iterator.previous_type == Bidi_Class::Arabic_Number) { // and those sandwiching types are number types
        BIDI_CLASS(iterator.index) = iterator.previous_type;
      }
    }
  });
  iterator.all([&]{ // W5
    if (iterator.current_type == Bidi_Class::European_Number) {
      auto i = iterator.index;
      for (int j = i - 1; j >= 0 && ((BIDI_CLASS(j) == Bidi_Class::European_Terminator) || IGNORE_BY_X9(j)); --j)
        BIDI_CLASS(j) = Bidi_Class::European_Number;
      for (int j = i + 1; j < length && ((BIDI_CLASS(j) == Bidi_Class::European_Terminator) || IGNORE_BY_X9(j)); ++j)
        BIDI_CLASS(j) = Bidi_Class::European_Number;
    }
  });
  iterator.all([&]{ // W6
    if (iterator.current_type == Bidi_Class::Common_Separator || iterator.current_type == Bidi_Class::European_Terminator || iterator.current_type == Bidi_Class::European_Separator) {
      BIDI_CLASS(iterator.index) = Bidi_Class::Other_Neutral;
    }
  });
  iterator.all([&]{ // W7
    if (iterator.current_type == Bidi_Class::European_Number && iterator.last_strong_type == Bidi_Class::Left_To_Right) {
      BIDI_CLASS(iterator.index) = Bidi_Class::Left_To_Right;
    }
  });
}

void BidiAlgorithm::Resolving_Neutral_and_Isolate_Formatting_Types(IsolatingRunSequenceIterator &iterator) { // N0-N2
  assign_bracket_pairs(iterator.reset_index); // N0

  iterator.NI_context = true;
  iterator.all([&]{
    int i = iterator.index;    
    if ((iterator.current_type == Bidi_Class::Other_Neutral) && (MATCHING_INDEX(i) > i)) { // open half of a matched bracket pair (ONs with a matching != -1 can only be brackets)
      auto open_index = i;
      auto close_index = MATCHING_INDEX(i);
      auto embedding_direction = embedding_direction_for_embedding_level(EMBEDDING_LEVEL(i));
      
      bool did_find_any_strong_types_within_bracket_pair = false;
      IsolatingRunSequenceIterator withinbrackets(*this, open_index, +1);
      withinbrackets.next(); // step past open bracket
      withinbrackets.each([&]{
        if (withinbrackets.index >= close_index)
          return false;
        if (Is_Strong_in_NI_context(withinbrackets.current_type)) {
          did_find_any_strong_types_within_bracket_pair = true;
          if (NI_influencing_direction(withinbrackets.current_type) == embedding_direction) {
            BIDI_CLASS(open_index) = embedding_direction;
            BIDI_CLASS(close_index) = embedding_direction;
            return false;
          }
        }
        return true;
      });
      
      if (BIDI_CLASS(open_index) == Bidi_Class::Other_Neutral) { // withinbrackets iteration didn't set bracket class
        if (did_find_any_strong_types_within_bracket_pair) {
          if (iterator.last_strong_type != embedding_direction) { // c.1 - last_strong_type is guaranteed to be only L or R at this point (previous rules replaced ALs, and we're using NI_context which will return ENs and ANs as Rs)
            BIDI_CLASS(open_index) = iterator.last_strong_type;
            BIDI_CLASS(close_index) = iterator.last_strong_type;
          } else { // c.2
            BIDI_CLASS(open_index) = embedding_direction;
            BIDI_CLASS(close_index) = embedding_direction;
          }
        } else {
          // no strong types within bracket pair - do nothing
        }
      }
    }
  });
  DEBUG_TRACE("N0", false);

  iterator.all([&]{ // N1
    if (Is_Neutral_or_Isolate(iterator.current_type)) {
      auto next_strong_type = iterator.eos;
      iterator.lookahead([&](IsolatingRunSequenceIterator &lookahead) {
        if (Is_Strong_in_NI_context(lookahead.current_type)) {
          next_strong_type = NI_influencing_direction(lookahead.current_type);
          return false; // stop
        }
        return true;
      });
      if (iterator.last_strong_type == next_strong_type)
        BIDI_CLASS(iterator.index) = next_strong_type;
    }
  });
  DEBUG_TRACE("N1", false);
  
  iterator.all([&]{ // N2
    if (Is_Neutral_or_Isolate(iterator.current_type)) {
      BIDI_CLASS(iterator.index) = embedding_direction_for_embedding_level(EMBEDDING_LEVEL(iterator.index));
    }
  });
  DEBUG_TRACE("N2", false);
  
  iterator.NI_context = false;
}

void BidiAlgorithm::Resolving_Implicit_Levels(IsolatingRunSequenceIterator &iterator) { // I1-I2
  iterator.all([&]{
    if (EMBEDDING_LEVEL(iterator.index) & 1) { // odd embedding level
      switch (iterator.current_type) {
        case Bidi_Class::Left_To_Right:
        case Bidi_Class::European_Number:
        case Bidi_Class::Arabic_Number:
          EMBEDDING_LEVEL(iterator.index) += 1;
          break;
        default:
          break;
      }
    } else { // even embedding level
      switch (iterator.current_type) {
        case Bidi_Class::Right_To_Left:
          EMBEDDING_LEVEL(iterator.index) += 1;
          break;
        case Bidi_Class::Arabic_Number:
        case Bidi_Class::European_Number:
          EMBEDDING_LEVEL(iterator.index) += 2;
          break;
        default:
          break;
      }
    }
  });
}

Bidi_Class BidiAlgorithm::embedding_direction_for_embedding_level(EmbeddingLevel level) const {
  return (level & 1) ? Bidi_Class::Right_To_Left : Bidi_Class::Left_To_Right;
}

int BidiAlgorithm::irs_skip_ignores(int i, const int direction) const {
  while ((i >= 0) && (i < length) && IGNORE_BY_X9(i))
    i += direction;
  if (i < 0 || i >= length)
    return -1;
  return i;
}

int BidiAlgorithm::irs_step(EmbeddingLevel irs_embedding_level, int i, const int direction, int &did_jump, EmbeddingLevel &found_bordering_level) const {
  i = irs_skip_ignores(i, direction);
  if (i == -1)
    return -1;
  if (IS_ISOLATE_BRIDGE(i) && 
       (((direction == +1) && (MATCHING_INDEX(i) > i)) || 
       ((direction == -1) && (MATCHING_INDEX(i) < i))) ) {
    int next = MATCHING_INDEX(i);
    if (abs(next - i) > 1) // jumped at least one character
      did_jump = i + direction;
    return irs_skip_ignores(next, direction);
  } else {
    int next = irs_skip_ignores(i + direction, direction);
    if (next == -1)
      return -1;
    if (irs_embedding_level != EMBEDDING_LEVEL(next))  { // at end if the level changed
      found_bordering_level = EMBEDDING_LEVEL(next);
      did_jump = next;
      return -1;
    }
    return next;
  }
}

void BidiAlgorithm::assign_bracket_pairs(const int irs_start) {
  int count = 0;
  int open_brackets[MAX_DEPTH]; // stack of indices of open brackets
  IsolatingRunSequenceIterator iterator(*this, irs_start, +1);
  iterator.all([&]{
    int i = iterator.index;
    if (iterator.current_type == Bidi_Class::Other_Neutral) { // brackets are a proper subset of ONs
      Bidi_Paired_Bracket_Type paired_bracket_type;
      uint32_t code = Get_Bidi_Paired_Bracket(text[i], paired_bracket_type);
      if (paired_bracket_type == Bidi_Paired_Bracket_Type::Open) {
        if (count < MAX_DEPTH) {
          open_brackets[count] = i;
          ++count;
        }
      } else if (paired_bracket_type == Bidi_Paired_Bracket_Type::Close) {
        for (int m = count - 1; m >= 0; --m) { // search stack from top down to find matching
          if (code == text[open_brackets[m]]) { found_match:
            MATCHING_INDEX(open_brackets[m]) = i; // one-way link (open->close) because we can do all processing upon discovering an open bracket
            count = m; // pop down past the one we just matched to
            break;
          } else { // http://www.unicode.org/L2/L2013/13123-norm-and-bpa.pdf ...In practice this amounts to running the algorithm as before but including the following additional pairs: U+2329 with U+3009, and U+3008 with U+232A...
            auto a = text[open_brackets[m]];
            auto b = text[i];
            if ((a == 0x2329 && b == 0x3009) || (a == 0x3008 && b == 0x232A))
              goto found_match;
          }
        }
      }
    }
  });
}

void BidiAlgorithm::append_next_isolating_run_sequence(int head, int next) {
  next = irs_skip_ignores(next, +1);
  if (next == -1) // this potential IRS was really just empty
    return;
  if (IS_ISOLATE_BRIDGE(next) && (MATCHING_INDEX(next) < next)) // if the first valid character of this potential IRS is a PDI, it means it's already claimed by a different IRS altogether
    return;
  retry: if (NEXT_ISOLATING_RUN_SEQUENCE(head) == -1) { // 'head' is the current end of the chain, may append
    NEXT_ISOLATING_RUN_SEQUENCE(head) = next; // append
  } else if (NEXT_ISOLATING_RUN_SEQUENCE(head) == next) { // irs already exists in the chain, ignore
    return;
  } else {
    head = NEXT_ISOLATING_RUN_SEQUENCE(head);
    goto retry; // try appending to the end of the next item in the chain
  }
}

void BidiAlgorithm::prepare_isolating_run_sequence(int irs_start) {
  EmbeddingLevel previous_embedding_level = paragraph_embedding_level;
  EmbeddingLevel current_embedding_level = paragraph_embedding_level;
  EmbeddingLevel next_embedding_level = paragraph_embedding_level;
  int i = irs_skip_ignores(irs_start, +1); // find current_embedding_level
  if (i != -1) {
    current_embedding_level = EMBEDDING_LEVEL(i);
    i = irs_skip_ignores(irs_start, -1); // walk backwards to find previous_embedding_level
    while (i != -1) {
      int ignore_jump;
      i = irs_step(current_embedding_level, i, -1, ignore_jump, previous_embedding_level); // will set previous_embedding_level if found (otherwise will stay paragraph_embedding_level)
    }
    i = irs_skip_ignores(irs_start, +1); // walk forwards, find next_embedding_level, link together NEXT_ISOLATING_RUN_SEQUENCEs
    while (i != -1) {
      int did_jump = -1;
      i = irs_step(current_embedding_level, i, +1, did_jump, next_embedding_level);
      if (did_jump != -1) {
        append_next_isolating_run_sequence(irs_start, did_jump);
      }
    }
  }
#define SEQUENCE_BOUNDARY_TYPE(A, B) ((((A) > (B)) ? (A) : (B)) & 1)
  SOS(irs_start) = SEQUENCE_BOUNDARY_TYPE(previous_embedding_level, current_embedding_level);
  EOS(irs_start) = SEQUENCE_BOUNDARY_TYPE(current_embedding_level, next_embedding_level);
}

EmbeddingLevel BidiAlgorithm::paragraph_embedding_level_for_strong_character_index(const int first_strong_index) const {
  if (first_strong_index < 0) {
    return 0; // no strong character found, default to L
  } else {
    switch (BIDI_CLASS(first_strong_index)) { // P3
      case Bidi_Class::Right_To_Left:
      case Bidi_Class::Arabic_Letter:
        return 1;
      default:
        return 0;
    }
  }
}

int BidiAlgorithm::find_first_strong_index(const int irs_start, const int end_index) const {
  IsolatingRunSequenceIterator iterator(*this, irs_start, +1);
  int first_strong_index = -1;
  iterator.each([&]{
    if (iterator.index > end_index)
      return false;
    if (Is_Strong(iterator.current_type)) {
      first_strong_index = iterator.index;
      return false; // stop
    }
    return true;
  });
  return first_strong_index;
}

#if UAX_BIDI_ENABLE_DEBUG_TRACE
static const char *BIDI_CODE(Bidi_Class cls) {
  switch (cls) {
    #define X(CODE, NAME) case Bidi_Class::NAME: return #CODE;
    BIDI_CLASS_LIST
    #undef X
  }
  return "?";
}

void BidiAlgorithm::trace(const char *step, bool headers) const {
  if (!debug_trace)
    return;
  auto label = [](const char *s) { printf("%*s: ", 9, s); };
  printf("[%s]\n", step);
  if (headers) {
    label("pe");
    printf("%d\n", paragraph_embedding_level);
    label("index");
    for (int i = 0; i < length; ++i) {
      printf("% 4d ", i);
    }
    printf("\n");
    label("character");
    for (int i = 0; i < length; ++i) {
      printf("%04X ", text[i]);
    }
    printf("\n");
  }
  label("matching");
  for (int i = 0; i < length; ++i) {
    if (MATCHING_INDEX(i) != -1) {
      printf("% 4d ", MATCHING_INDEX(i));
    } else {
      printf("   - ");
    }
  }
  printf("\n");
  label("nextirs");
  for (int i = 0; i < length; ++i) {
    if (NEXT_ISOLATING_RUN_SEQUENCE(i) != -1) {
      printf("% 4d ", NEXT_ISOLATING_RUN_SEQUENCE(i));
    } else {
      printf("   - ");
    }
  }
  printf("\n");
  label("bidiclass");
  for (int i = 0; i < length; ++i) {
    printf("%*s ", 4, BIDI_CODE(BIDI_CLASS(i)));
  }
  printf("\n");
  label("embedding");
  for (int i = 0; i < length; ++i) {
    printf("% 4d ", EMBEDDING_LEVEL(i));
  }
  printf("\n");
}
#endif

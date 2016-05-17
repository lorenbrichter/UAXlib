#include "UCDReaderUtil.h"
#include "UCDEnums.h"

using namespace UCD;

#define UNKNOWN_CODE assert(0 && "unknown code")
#define RET_IF_EQ(CODE, NAME, PREFIX) if (strncmp(#CODE, text, len) == 0) return PREFIX::NAME;

Bidi_Paired_Bracket_Type text_to_Bidi_Paired_Bracket_Type(const char *text, size_t len) {
  #define X(CODE, NAME) RET_IF_EQ(CODE, NAME, Bidi_Paired_Bracket_Type)
  BIDI_PAIRED_BRACKET_TYPE_LIST
  #undef X
  UNKNOWN_CODE;
  return Bidi_Paired_Bracket_Type::None;
}

Case_Folding_Status text_to_Case_Folding_Status(const char *text, size_t len) {
  #define X(CODE, NAME) RET_IF_EQ(CODE, NAME, Case_Folding_Status)
  CASE_FOLDING_STATUS_LIST
  #undef X
  UNKNOWN_CODE;
  return Case_Folding_Status::Common;
}

East_Asian_Width text_to_East_Asian_Width(const char *text, size_t len) {
  #define X(CODE, NAME) RET_IF_EQ(CODE, NAME, East_Asian_Width)
  EAST_ASIAN_WIDTH_LIST
  #undef X
  UNKNOWN_CODE;
  return East_Asian_Width::Neutral;
}

Line_Break text_to_Line_Break(const char *text, size_t len) {
  #define X(CODE, NAME) RET_IF_EQ(CODE, NAME, Line_Break)
  LINE_BREAK_LIST
  #undef X
  UNKNOWN_CODE;
  return Line_Break::Unknown;
}

Script text_to_Script(const char *text, size_t len) {
  #define X(NAME) RET_IF_EQ(NAME, NAME, Script)
  SCRIPT_LIST
  #undef X
  UNKNOWN_CODE;
  return Script::Unknown;
}

Bidi_Class text_to_Bidi_Class(const char *text, size_t len) {
  #define X(CODE, NAME) RET_IF_EQ(CODE, NAME, Bidi_Class)
  BIDI_CLASS_LIST
  #undef X
  UNKNOWN_CODE;
  return Bidi_Class::Left_To_Right;
}

const char *Bidi_Paired_Bracket_Type_to_string(Bidi_Paired_Bracket_Type type) {
  switch (type) {
    #define X(CODE, NAME) case Bidi_Paired_Bracket_Type::NAME: return "Bidi_Paired_Bracket_Type::" #NAME;
    BIDI_PAIRED_BRACKET_TYPE_LIST
    #undef X
  }
  UNKNOWN_CODE;
  return "";
}

const char *Script_to_string(Script script) {
  switch (script) {
    #define X(NAME) case Script::NAME: return "Script::" #NAME;
    SCRIPT_LIST
    #undef X
  }
  UNKNOWN_CODE;
  return "";
}

const char *Bidi_Class_to_string(Bidi_Class cls) {
  switch (cls) {
    #define X(CODE, NAME) case Bidi_Class::NAME: return "Bidi_Class::" #NAME;
    BIDI_CLASS_LIST
    #undef X
  }
  UNKNOWN_CODE;
  return "";
}

const char *Line_Break_to_string(Line_Break line_break) {
  switch (line_break) {
    #define X(CODE, NAME) case Line_Break::NAME: return "Line_Break::" #NAME;
    LINE_BREAK_LIST
    #undef X
  }
  UNKNOWN_CODE;
  return "";
}

struct Field {
  const char *text;
  size_t length;
  codepoint asCodepoint() { return hex_to_codepoint(text, length); }
  codepoint_range asCodepointRange() { return hex_to_codepoint_range(text, length); }
  int asDecimal() { return str_to_dec(text, length); }
  template<typename F> void asSequence(F f) { split(text, length, ' ', f); }
  template<typename F> void asCodepointSequence(F f) { asSequence([&](const char *s, size_t l) { f(hex_to_codepoint(s, l)); }); }
  template<typename F> void asDecimalSequence(F f) { asSequence([&](const char *s, size_t l) { f(str_to_dec(s, l)); }); }
  void print() { printf("%.*s", (int)length, text); }
};

struct Fields {
  Field *fields;
  int count;
  void print() {
    for (int i = 0; i < count; ++i) {
      fields[i].print();
      printf(" : ");
    }
    printf("\n");
  }
  
  /**
   Interpretations of fields
   These correspond to UCD/MethodName.txt
   Called from the lambda passed to withUCDFormattedText/File with the given 'Field'
   */
  
  void BidiBrackets(codepoint &bracket, codepoint &paired_bracket, Bidi_Paired_Bracket_Type &bracket_type) {
    assert(count == 3);
    bracket = fields[0].asCodepoint();
    paired_bracket = fields[1].asCodepoint();
    bracket_type = text_to_Bidi_Paired_Bracket_Type(fields[2].text, fields[2].length);
  }
  
  void BidiMirroring(codepoint &code, codepoint &mirror) {
    assert(count == 2);
    code = fields[0].asCodepoint();
    mirror = fields[1].asCodepoint();
  }
  
  void Blocks(codepoint_range &range, const char * &name, size_t &len) {
    assert(count == 2);
    range = fields[0].asCodepointRange();
    name = fields[1].text;
    len = fields[1].length;
  }
  
  void CaseFolding(codepoint &code, Case_Folding_Status &status, codepoint *codepoints, int &codepoint_count, int codepoints_capacity) {
    assert(count == 3);
    code = fields[0].asCodepoint();
    status = text_to_Case_Folding_Status(fields[1].text, fields[1].length);
    codepoint_list(fields[2].text, fields[2].length, codepoints, codepoint_count, codepoints_capacity);
  }
  
  void CJKRadicals(const char * &radical_number, size_t &len, codepoint &cjk_radical, codepoint &cjk_unified_ideograph) {
    assert(count == 3);
    radical_number = fields[0].text;
    len = fields[0].length;
    cjk_radical = fields[1].asCodepoint();
    cjk_unified_ideograph = fields[2].asCodepoint();
  }
  
  void CompositionExclusions(codepoint &code) {
    assert(count == 1);
    code = fields[0].asCodepoint();
  }
  
  void EastAsianWidth(codepoint_range &range, East_Asian_Width &width) {
    assert(count == 2);
    range = fields[0].asCodepointRange();
    width = text_to_East_Asian_Width(fields[1].text, fields[1].length);
  }
  
  void LineBreak(codepoint_range &range, Line_Break &line_break) {
    assert(count == 2);
    range = fields[0].asCodepointRange();
    line_break = text_to_Line_Break(fields[1].text, fields[1].length);
  }
  
  void Scripts(codepoint_range &range, Script &script) {
    range = fields[0].asCodepointRange();
    script = text_to_Script(fields[1].text, fields[1].length);
  }
  
  void DerivedBidiClass(codepoint_range &range, Bidi_Class &cls) {
    assert(count == 2);
    range = fields[0].asCodepointRange();
    cls = text_to_Bidi_Class(fields[1].text, fields[1].length);
  }
  
  void DerivedEastAsianWidth(codepoint_range &range, East_Asian_Width &width) {
    EastAsianWidth(range, width);
  }
  
  void DerivedLineBreak(codepoint_range &range, Line_Break &line_break) {
    LineBreak(range, line_break);
  }
};

/**
 Calls back each line of the input with an argument of type 'Field'
 Use the methods of Field (e.g. 'BidiMirroring()' to request a particular interpretation)
 */
template<typename F> void withUCDFormattedText(const char *bytes, size_t len, F func) {
  const int MaxFields = 32;
  Field info[MaxFields];
  eachLineAfterStrippingComments(bytes, len, [&](const char *line, size_t len) {
    trim(line, len, trim_whitespace);
    if (len > 0) {
      int n = 0;
      split(line, len, ';', [&](const char *field, size_t len) {
        if (n < MaxFields) {
          info[n].text = field;
          info[n].length = len;
          ++n;
        }
      });
      Fields fields {
        .fields = info,
        .count = n,
      };
      func(fields);
    }
  });
}

template<typename F> void withUCDFormattedFile(const char *path, F func) {
  withFile(path, [&](const void *bytes, size_t len) {
    withUCDFormattedText((const char *)bytes, len, func);
  });
}

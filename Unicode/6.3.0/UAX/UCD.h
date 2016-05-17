#ifndef UCD_H
#define UCD_H

#include "UCDEnums.h"

namespace Unicode {
  typedef uint32_t Codepoint;
};

namespace UCD {
  using namespace Unicode;
  
  Codepoint Get_Bidi_Paired_Bracket(const Codepoint code, Bidi_Paired_Bracket_Type &bracket_type);
  Codepoint Get_Bidi_Mirroring(const Codepoint code);
  Script Get_Script(const Codepoint code);
  Bidi_Class Get_Bidi_Class(const Codepoint code);
  Line_Break Get_Line_Break(const Codepoint code);

  inline bool Is_Isolate_Initiator(const Bidi_Class cls) {
    switch (cls) {
      case Bidi_Class::Left_To_Right_Isolate:
      case Bidi_Class::Right_To_Left_Isolate:
      case Bidi_Class::First_Strong_Isolate:
        return true;
      default:
        return false;
    }
  }
  inline bool Is_Neutral_or_Isolate(const Bidi_Class cls) {
    switch (cls) {
      case Bidi_Class::Paragraph_Separator:
      case Bidi_Class::Segment_Separator:
      case Bidi_Class::White_Space:
      case Bidi_Class::Other_Neutral:
      case Bidi_Class::First_Strong_Isolate:
      case Bidi_Class::Left_To_Right_Isolate:
      case Bidi_Class::Right_To_Left_Isolate:
      case Bidi_Class::Pop_Directional_Isolate:
        return true;
      default:
        return false;
    }
  }
  inline bool Is_Strong(const Bidi_Class cls) {
    switch (cls) {
      case Bidi_Class::Left_To_Right:
      case Bidi_Class::Right_To_Left:
      case Bidi_Class::Arabic_Letter:
        return true;
      default:
        return false;
    }
  }
  inline bool Is_Strong_in_NI_context(const Bidi_Class cls) {
    switch (cls) {
      case Bidi_Class::Left_To_Right:
      case Bidi_Class::Right_To_Left:
      case Bidi_Class::Arabic_Letter:
      case Bidi_Class::European_Number:
      case Bidi_Class::Arabic_Number:
        return true;
      default:
        return false;
    }
  }
  inline Bidi_Class NI_influencing_direction(const Bidi_Class cls) {
    switch (cls) {
      default:
      case Bidi_Class::Left_To_Right:
        return Bidi_Class::Left_To_Right;
      case Bidi_Class::Right_To_Left:
      case Bidi_Class::Arabic_Letter:
      case Bidi_Class::European_Number:
      case Bidi_Class::Arabic_Number:
        return Bidi_Class::Right_To_Left;
    }
  }
  
};

#endif

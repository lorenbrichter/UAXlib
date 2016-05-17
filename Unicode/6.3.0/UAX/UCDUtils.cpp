#include <stdint.h>
#include "UCD.h"

using namespace UCD;

Codepoint UCD::Get_Bidi_Paired_Bracket(const Codepoint code, Bidi_Paired_Bracket_Type &bracket_type) {
  switch (code) {
    #include "_Derived/BidiBrackets.h"
    default: bracket_type = Bidi_Paired_Bracket_Type::None; return 0;
  }
}

Codepoint UCD::Get_Bidi_Mirroring(const Codepoint code) {
  switch (code) {
    #include "_Derived/BidiMirroring.h"
    default: return 0;
  }
}

Script UCD::Get_Script(const Codepoint code) {
  switch (code) {
    #include "_Derived/Scripts.h"
    default: return Script::Unknown;
  }
}

Bidi_Class UCD::Get_Bidi_Class(const Codepoint code) {
  switch (code) {
    #include "_Derived/DerivedBidiClass.h"
    default: return Bidi_Class::Left_To_Right;
  }
}

Line_Break UCD::Get_Line_Break(const Codepoint code) {
  switch (code) {
    #include "_Derived/DerivedLineBreak.h"
    default: return Line_Break::Unknown;
  }
}

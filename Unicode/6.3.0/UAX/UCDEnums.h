#ifndef UCDENUMS_H
#define UCDENUMS_H

#define BIDI_PAIRED_BRACKET_TYPE_LIST \
  X( o , Open  ) \
  X( c , Close ) \
  X( n , None  )

#define CASE_FOLDING_STATUS_LIST \
  X( C , Common  ) \
  X( F , Full    ) \
  X( S , Simple  ) \
  X( T , Special )

#define LINE_BREAK_LIST                                \
  X( BK , Mandatory_Break              ) \
  X( CR , Carriage_Return              ) \
  X( LF , Line_Feed                    ) \
  X( CM , Combining_Mark               ) \
  X( SG , Surrogate                    ) \
  X( GL , Glue                         ) \
  X( CB , Contingent_Break             ) \
  X( SP , Space                        ) \
  X( ZW , ZWSpace                      ) \
  X( NL , Next_Line                    ) \
  X( WJ , Word_Joiner                  ) \
  X( JL , JL                           ) \
  X( JV , JV                           ) \
  X( JT , JT                           ) \
  X( H2 , H2                           ) \
  X( H3 , H3                           ) \
  X( XX , Unknown                      ) \
  X( OP , Open_Punctuation             ) \
  X( CL , Close_Punctuation            ) \
  X( CP , Close_Parenthesis            ) \
  X( QU , Quotation                    ) \
  X( NS , Nonstarter                   ) \
  X( EX , Exclamation                  ) \
  X( SY , Break_Symbols                ) \
  X( IS , Infix_Numeric                ) \
  X( PR , Prefix_Numeric               ) \
  X( PO , Postfix_Numeric              ) \
  X( NU , Numeric                      ) \
  X( AL , Alphabetic                   ) \
  X( ID , Ideographic                  ) \
  X( IN , Inseparable                  ) \
  X( HY , Hyphen                       ) \
  X( BB , Break_Before                 ) \
  X( BA , Break_After                  ) \
  X( SA , Complex_Context              ) \
  X( AI , Ambiguous                    ) \
  X( B2 , Break_Both                   ) \
  X( HL , Hebrew_Letter                ) \
  X( CJ , Conditional_Japanese_Starter ) \
  X( RI , Regional_Indicator           )

#define BIDI_CLASS_LIST              \
  X( L   , Left_To_Right           ) \
  X( R   , Right_To_Left           ) \
  X( EN  , European_Number         ) \
  X( ES  , European_Separator      ) \
  X( ET  , European_Terminator     ) \
  X( AN  , Arabic_Number           ) \
  X( CS  , Common_Separator        ) \
  X( B   , Paragraph_Separator     ) \
  X( S   , Segment_Separator       ) \
  X( WS  , White_Space             ) \
  X( ON  , Other_Neutral           ) \
  X( BN  , Boundary_Neutral        ) \
  X( NSM , Nonspacing_Mark         ) \
  X( AL  , Arabic_Letter           ) \
  X( LRO , Left_To_Right_Override  ) \
  X( RLO , Right_To_Left_Override  ) \
  X( LRE , Left_To_Right_Embedding ) \
  X( RLE , Right_To_Left_Embedding ) \
  X( PDF , Pop_Directional_Format  ) \
  X( LRI , Left_To_Right_Isolate   ) \
  X( RLI , Right_To_Left_Isolate   ) \
  X( FSI , First_Strong_Isolate    ) \
  X( PDI , Pop_Directional_Isolate )

#define EAST_ASIAN_WIDTH_LIST \
  X( N  , Neutral   ) \
  X( A  , Ambiguous ) \
  X( H  , Halfwidth ) \
  X( W  , Wide      ) \
  X( F  , Fullwidth ) \
  X( Na , Narrow    )

#define SCRIPT_LIST           \
  X( Unknown                ) \
  X( Arabic                 ) \
  X( Armenian               ) \
  X( Avestan                ) \
  X( Balinese               ) \
  X( Bamum                  ) \
  X( Batak                  ) \
  X( Bengali                ) \
  X( Bopomofo               ) \
  X( Brahmi                 ) \
  X( Braille                ) \
  X( Buginese               ) \
  X( Buhid                  ) \
  X( Canadian_Aboriginal    ) \
  X( Carian                 ) \
  X( Chakma                 ) \
  X( Cham                   ) \
  X( Cherokee               ) \
  X( Common                 ) \
  X( Coptic                 ) \
  X( Cuneiform              ) \
  X( Cypriot                ) \
  X( Cyrillic               ) \
  X( Deseret                ) \
  X( Devanagari             ) \
  X( Egyptian_Hieroglyphs   ) \
  X( Ethiopic               ) \
  X( Georgian               ) \
  X( Glagolitic             ) \
  X( Gothic                 ) \
  X( Greek                  ) \
  X( Gujarati               ) \
  X( Gurmukhi               ) \
  X( Han                    ) \
  X( Hangul                 ) \
  X( Hanunoo                ) \
  X( Hebrew                 ) \
  X( Hiragana               ) \
  X( Imperial_Aramaic       ) \
  X( Inherited              ) \
  X( Inscriptional_Pahlavi  ) \
  X( Inscriptional_Parthian ) \
  X( Javanese               ) \
  X( Kaithi                 ) \
  X( Kannada                ) \
  X( Katakana               ) \
  X( Kayah_Li               ) \
  X( Kharoshthi             ) \
  X( Khmer                  ) \
  X( Lao                    ) \
  X( Latin                  ) \
  X( Lepcha                 ) \
  X( Limbu                  ) \
  X( Linear_B               ) \
  X( Lisu                   ) \
  X( Lycian                 ) \
  X( Lydian                 ) \
  X( Malayalam              ) \
  X( Mandaic                ) \
  X( Meetei_Mayek           ) \
  X( Meroitic_Cursive       ) \
  X( Meroitic_Hieroglyphs   ) \
  X( Miao                   ) \
  X( Mongolian              ) \
  X( Myanmar                ) \
  X( New_Tai_Lue            ) \
  X( Nko                    ) \
  X( Ogham                  ) \
  X( Old_Italic             ) \
  X( Old_Persian            ) \
  X( Old_South_Arabian      ) \
  X( Old_Turkic             ) \
  X( Ol_Chiki               ) \
  X( Oriya                  ) \
  X( Osmanya                ) \
  X( Phags_Pa               ) \
  X( Phoenician             ) \
  X( Rejang                 ) \
  X( Runic                  ) \
  X( Samaritan              ) \
  X( Saurashtra             ) \
  X( Sharada                ) \
  X( Shavian                ) \
  X( Sinhala                ) \
  X( Sora_Sompeng           ) \
  X( Sundanese              ) \
  X( Syloti_Nagri           ) \
  X( Syriac                 ) \
  X( Tagalog                ) \
  X( Tagbanwa               ) \
  X( Tai_Le                 ) \
  X( Tai_Tham               ) \
  X( Tai_Viet               ) \
  X( Takri                  ) \
  X( Tamil                  ) \
  X( Telugu                 ) \
  X( Thaana                 ) \
  X( Thai                   ) \
  X( Tibetan                ) \
  X( Tifinagh               ) \
  X( Ugaritic               ) \
  X( Vai                    ) \
  X( Yi                     )

namespace UCD {
  enum class Bidi_Class : uint8_t {
    #define X(CODE, NAME) NAME,
    BIDI_CLASS_LIST
    #undef X
  };

  enum class Bidi_Paired_Bracket_Type : uint8_t {
    #define X(CODE, NAME) NAME,
    BIDI_PAIRED_BRACKET_TYPE_LIST
    #undef X
  };

  enum class Line_Break : uint8_t {
    #define X(CODE, NAME) NAME,
    LINE_BREAK_LIST
    #undef X
  };

  enum class Script : uint8_t {
    #define X(NAME) NAME,
    SCRIPT_LIST
    #undef X
  };

  enum class Case_Folding_Status : uint8_t {
    #define X(CODE, NAME) NAME,
    CASE_FOLDING_STATUS_LIST
    #undef X
  };

  enum class East_Asian_Width : uint8_t {
    #define X(CODE, NAME) NAME,
    EAST_ASIAN_WIDTH_LIST
    #undef X
  };
};

#endif

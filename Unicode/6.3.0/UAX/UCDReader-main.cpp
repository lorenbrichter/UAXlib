#include "UCDReader.h"
#include <limits.h>
#include <stdarg.h>

using namespace UCD;

const char *scratch_str(const char *fmt, ...) {
  static char buffer[2048];
  va_list list;
  va_start(list, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, list);
  va_end(list);
  return buffer;
}

template<typename F> void withOutputFile(const char *path, F func) {
  FILE *f = fopen(path, "w");
  assert(f);
  func(f);
  fclose(f);
}

int main(int argc, char const *argv[]) {

  assert(argc == 3);
  const char *input_path = argv[1];
  const char *output_path = argv[2];
  
  #define HEX_FMT "0x%04X"
  #define RANGE_STR(R) ((R.first == R.last) ? scratch_str(HEX_FMT, R.first) : scratch_str(HEX_FMT " ... " HEX_FMT, R.first, R.last))
  #define UCD_FILE_PATH(X) scratch_str("%s/%s.txt", input_path, #X)
  #define OUTPUT_PATH(X) scratch_str("%s/%s.h", output_path, #X)
  
  #define PROCESS2(IN, OUT) withOutputFile(OUT, [&](FILE *out) { withUCDFormattedFile(IN, [&](Fields fields) {
  #define PROCESS(IN, OUT) PROCESS2(UCD_FILE_PATH(IN), OUTPUT_PATH(OUT))
  // between PROCESS and DONE, `Fields fields` and `FILE *out` will be in scope
  #define DONE }); })
  
#if 0
  PROCESS2(UCD_FILE_PATH(Blocks), OUTPUT_PATH(Blocks-dump)) {
    fprintf(out, "X(%.*s) \\\n", (int)fields.fields[1].length, fields.fields[1].text);
  } DONE;
  PROCESS2(UCD_FILE_PATH(Scripts), OUTPUT_PATH(Scripts-dump)) {
    fprintf(out, "X(%.*s) \\\n", (int)fields.fields[1].length, fields.fields[1].text);
  } DONE;
#endif
  
  PROCESS(BidiCharacterTest, BidiCharacterTest) {
    assert(fields.count == 5);
    
    fprintf(out, "X(");
    
    fprintf(out, "{");
    fields.fields[0].asCodepointSequence([&](codepoint c) {
      fprintf(out, HEX_FMT ",", c);
    });
    fprintf(out, "},");
    
    int paragraph_direction = fields.fields[1].asDecimal();
    fprintf(out, "%d,", paragraph_direction);
    int resolved_paragraph_embedding_level = fields.fields[2].asDecimal();
    fprintf(out, "%d,", resolved_paragraph_embedding_level);
    
    fprintf(out, "{");
    fields.fields[3].asDecimalSequence([&](int level) {
      fprintf(out, "%d,", level);
    });
    fprintf(out, "},");
    
    fprintf(out, "{");
    fields.fields[4].asDecimalSequence([&](int order) {
      fprintf(out, "%d,", order);
    });
    fprintf(out, "}");
    
    fprintf(out, ")\n");
  } DONE;
  
  PROCESS(BidiBrackets, BidiBrackets) {
    codepoint bracket, paired_bracket;
    Bidi_Paired_Bracket_Type bracket_type;
    fields.BidiBrackets(bracket, paired_bracket, bracket_type);
    fprintf(out, "case " HEX_FMT ": bracket_type = %s; return " HEX_FMT ";\n", bracket, Bidi_Paired_Bracket_Type_to_string(bracket_type), paired_bracket);
  } DONE;
  
  PROCESS(BidiMirroring, BidiMirroring) {
    codepoint code, mirror;
    fields.BidiMirroring(code, mirror);
    fprintf(out, "case " HEX_FMT ": return " HEX_FMT ";\n", code, mirror);
  } DONE;
  
  PROCESS(Scripts, Scripts) {
    codepoint_range range;
    Script script;
    fields.Scripts(range, script);
    fprintf(out, "case %s: return %s;\n", RANGE_STR(range), Script_to_string(script));
  } DONE;
  
  PROCESS(extracted/DerivedBidiClass, DerivedBidiClass) {
    codepoint_range range;
    Bidi_Class cls;
    fields.DerivedBidiClass(range, cls);
    fprintf(out, "case %s: return %s;\n", RANGE_STR(range), Bidi_Class_to_string(cls));
  } DONE;
  
  PROCESS(extracted/DerivedLineBreak, DerivedLineBreak) {
    codepoint_range range;
    Line_Break line_break;
    fields.DerivedLineBreak(range, line_break);
    fprintf(out, "case %s: return %s;\n", RANGE_STR(range), Line_Break_to_string(line_break));
  } DONE;
  
#if 0
  PROCESS(Blocks) {
    codepoint_range range;
    const char *text;
    size_t len;
    fields.Blocks(range, text, len);
    fprintf(out, "case %s: return \"%.*s\";\n", RANGE_STR(range), (int)len, text);
  } DONE;
#endif

#if 0
  withUCDFormattedFile(UCD_FILE_PATH(CaseFolding), [](Fields fields) {
    codepoint code;
    Case_Folding_Status status;
    const int Capacity = 16;
    codepoint codepoints[Capacity];
    int codepoint_count;
    fields.CaseFolding(code, status, codepoints, codepoint_count, Capacity);
    printf("%04X %d -> %d ", code, status, codepoint_count);
    for (int i = 0; i < codepoint_count; ++i) {
      printf("[%04X] ", codepoints[i]);
    }
    printf("\n");
  });
#endif

#if 0
  withUCDFormattedFile(UCD_FILE_PATH(CJKRadicals), [](Fields fields) {
    codepoint cjk_radical, cjk_unified_ideograph;
    const char *radical_number;
    size_t len;
    fields.CJKRadicals(radical_number, len, cjk_radical, cjk_unified_ideograph);
    printf("<%.*s> radical:%04X ideograph:%04X\n", (int)len, radical_number, cjk_radical, cjk_unified_ideograph);
  });
#endif

#if 0
  withUCDFormattedFile(UCD_FILE_PATH(CompositionExclusions), [](Fields fields) {
    codepoint code;
    fields.CompositionExclusions(code);
    printf("%04X\n", code);
  });
#endif

#if 0
  withUCDFormattedFile(UCD_FILE_PATH(LineBreak), [](Fields fields) {
    codepoint_range range;
    Line_Break line_break;
    fields.LineBreak(range, line_break);
    print_range(range); printf(" %d\n", line_break);
  });
#endif

#if 0
  withUCDFormattedFile(UCD_FILE_PATH(extracted/DerivedEastAsianWidth), [](Fields fields) {
    codepoint_range range;
    East_Asian_Width width;
    fields.DerivedEastAsianWidth(range, width);
    if (range.first == range.last)
      printf("%04X %d\n", range.first, width);
    else
      printf("%04X..%04X %d\n", range.first, range.last, width);
  });
#endif

  return 0;
}

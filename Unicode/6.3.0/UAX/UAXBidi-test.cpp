#include <stdlib.h>
#include <stdint.h>
#include "UAX.h"
#include "UCDReader.h"

#define ANSI_FOREGROUND_RED     "\x1b[31m"
#define ANSI_FOREGROUND_GREEN   "\x1b[32m"
#define ANSI_FOREGROUND_DEFAULT "\x1b[39m"

template<typename T>
struct GrowingScratchBuffer {
  T *buffer = nullptr;
  size_t size = 0;
  ~GrowingScratchBuffer() {
    free(buffer);
  }
  void ensureSize(size_t s) {
    if (s > size) {
      // printf("%d -> %d\n", (int)size, (int)s);
      size = s;
      free(buffer);
      buffer = (T *)malloc(size);
    }
  }
};

int main (int argc, char const *argv[]) {
  int passed = 0;
  int failed = 0;
  int total = 0;
  
  GrowingScratchBuffer<void> scratch;
  GrowingScratchBuffer<UAX::Bidi::EmbeddingLevel> embedding_levels;
  
  withUCDFormattedFile("../UCD/BidiCharacterTest.txt", [&](Fields fields) {
    
    int i = 0;
    int length = 0;
    fields.fields[0].asCodepointSequence([&](codepoint c) { ++length; });
    uint32_t _buffer[length];
    uint32_t *text = _buffer;
    fields.fields[0].asCodepointSequence([&](codepoint c) { text[i++] = c; });
    
    auto dir = UAX::Bidi::BaseDirection::Auto;
    int paragraph_direction = fields.fields[1].asDecimal();
    switch (paragraph_direction) {
      case 0: dir = UAX::Bidi::BaseDirection::Left; break;
      case 1: dir = UAX::Bidi::BaseDirection::Right; break;
    }    

    scratch.ensureSize(UAX::Bidi::ScratchBufferSize(length));
    embedding_levels.ensureSize(length * sizeof(UAX::Bidi::EmbeddingLevel));
    UAX::Bidi::EmbeddingLevel resolved_paragraph_embedding_level;
    UAX::Bidi::Run(text, length, dir, resolved_paragraph_embedding_level, embedding_levels.buffer, scratch.buffer);
    
    auto fail = [&] { 
      ++failed;
      printf(ANSI_FOREGROUND_RED "FAIL" ANSI_FOREGROUND_DEFAULT "\n");
      UAX::Bidi::Run(text, length, dir, resolved_paragraph_embedding_level, embedding_levels.buffer, scratch.buffer, true);
    };
    
    if (resolved_paragraph_embedding_level != fields.fields[2].asDecimal()) {
      fail();
    }
    
    i = 0;
    fields.fields[3].asDecimalSequence([&](unsigned char level) {
      if (level != embedding_levels.buffer[i]) {
        fail();
        printf("correct embedding levels: " ANSI_FOREGROUND_GREEN);
        fields.fields[3].asDecimalSequence([&](unsigned char level) {
          printf("%d ", level);
        });
        printf(ANSI_FOREGROUND_DEFAULT "\n\n");
      }
      ++i;
    });
    
    fields.fields[4].asDecimalSequence([&](int order) {
      // todo - reordering (assume no line breaking)
    });
    
    ++total;
  });
  
  printf("failed %d / %d\n", failed, total);
  if (failed > 0)
    return -1;
  return 0;
}

#ifndef UAX_H
#define UAX_H

#include "UCD.h"

namespace UAX {
  using namespace UCD;

  namespace Bidi {
    /**
     ** Unicode Bidirectional Algorithm -- UAX #9 -- http://www.unicode.org/reports/tr9/
     **/
    enum class BaseDirection {
      Auto,
      Left,
      Right,
    };
    typedef uint8_t EmbeddingLevel;
    
    /**
     ** Returns 'true' if the full Bidi Algorithm is required, 'false' if not (text purely LTR)
     **/
    bool RequiresAlgorithm(const Codepoint *text, const size_t length);
    
    /**
     ** Given a length of text, Run() requires a scratch buffer of this size. A scratch buffer allocation may be reused across Run()'s
     **/
    size_t ScratchBufferSize(const size_t text_length);
    
    /**
     ** Determine the paragraph embedding level and find the nested embedding level for all input characters. resolved_embedding_levels must hold 'length' x sizeof(EmbeddingLevel). scratch_buffer must be an allocation of ScratchBufferSize(length) bytes.
     **/
    void Run(const Codepoint *text, const size_t length, const BaseDirection base_direction, EmbeddingLevel &resolved_paragraph_embedding_level, EmbeddingLevel *resolved_embedding_levels, void *scratch_buffer
      #if UAX_BIDI_ENABLE_DEBUG_TRACE
      ,bool debug_trace = false
      #endif
    );
  };
  
  namespace Normalization {
    /**
     ** Unicode Normalization Forms -- UAX #15 -- http://unicode.org/reports/tr15/
     **/
    
    size_t BufferSizeForCanonicalDecomposition(size_t length);
    
    /**
     ** 
     **/
    enum class QuickCheckResult {
      Normalized,
      MaybeNormalized,
      NotNormalized,
    };
    inline QuickCheckResult QuickCheck(const Codepoint *text, const size_t length) {
      for (int i = 0; i < length; ++i) {
        // Codepoint ch = text[i];
        // todo
      }
      return QuickCheckResult::NotNormalized;
    }
    
    void CanonicalDecomposition();
    void CanonicalComposition();
    void CompatibilityDecomposition();
    
    void NFD(); // CanonicalDecomposition
    void NFC(); // CanonicalDecomposition -> CanonicalComposition
    void NFKD(); // CompatibilityDecomposition
    void NFKC(); // CompatibilityDecomposition -> CanonicalComposition    
  };
};

namespace Unicode {
  void UTF8_to_UTF32();
  void UTF32_to_UTF8();
};

#endif

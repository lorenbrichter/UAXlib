#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

bool trim_whitespace(char c) { return c == ' '; }
template<typename F> void trim(const char * &text, size_t &len, F should_trim) {
  int i = 0;
  while ((i < len) && should_trim(text[i])) ++i;
  int start = i;
  i = len - 1;
  while ((i > start) && should_trim(text[i])) --i;
  text += start;
  len = (i - start + 1);
}

template<typename F> void split(const char *text, size_t len, char delim, F func) {
  int i = 0;
  int start = 0;
  while (i < len) {
    while ((i < len) && (text[i] != delim)) ++i;
    const char *chunk = &text[start];
    size_t chunk_len = i - start;
    trim(chunk, chunk_len, trim_whitespace);
    func(chunk, chunk_len);
    ++i;
    start = i;
  }
}

template<typename F> void eachLineAfterStrippingComments(const char *bytes, size_t len, F func) {
  int i = 0;
  int start = -1;
  for (;;) {
    auto current = [&] { return i < len ? bytes[i] : 0; };
    auto skip_till = [&] (char till) { char c; do { ++i; c = current(); } while (c && c != till); };
    auto start_line = [&] {
      if (start == -1) {
        start = i;
      }
    };
    auto end_line = [&] {
      if (start != -1) {
        func(&bytes[start], i - start);
        start = -1;
      }
    };
    switch (current()) {
      case '#': end_line(); skip_till('\n'); break;
      case '\n': end_line(); ++i; break;
      case 0: end_line(); return;
      default: start_line(); ++i; break;
    }
  }
}

int hexchar(char c) {
  switch (c) {
    case '0':           return 0x0;
    case '1':           return 0x1;
    case '2':           return 0x2;
    case '3':           return 0x3;
    case '4':           return 0x4;
    case '5':           return 0x5;
    case '6':           return 0x6;
    case '7':           return 0x7;
    case '8':           return 0x8;
    case '9':           return 0x9;
    case 'a': case 'A': return 0xA;
    case 'b': case 'B': return 0xB;
    case 'c': case 'C': return 0xC;
    case 'd': case 'D': return 0xD;
    case 'e': case 'E': return 0xE;
    case 'f': case 'F': return 0xF;
    case 'x':           return -1;
  }
  return 0;
}

typedef uint32_t codepoint;
struct codepoint_range {
  codepoint first;
  codepoint last;
};

codepoint hex_to_codepoint(const char *text, size_t len) {
  codepoint n = 0;
  for (int i = 0; i < len; ++i) {
    n += hexchar(text[len - i - 1]) << (i * 4);
  }
  return n;
}

int str_to_dec(const char *text, size_t len) {
  int n = 0;
  int place = 1;
  for (int i = 0; i < len; ++i) {
    n += hexchar(text[len - i - 1]) * place;
    place *= 10;
  }
  return n;
}

codepoint_range hex_to_codepoint_range(const char *text, size_t len) {
  int n = 0;
  codepoint_range range;
  split(text, len, '.', [&](const char *field, size_t len) {
    switch (n) {
      case 0: range.first = hex_to_codepoint(field, len); break;
      case 1: break; // empty between ..
      case 2: range.last = hex_to_codepoint(field, len); break;
    }
    ++n;
  });
  if (n != 3) // this wasn't a .. range, just a regular number, set first and last to same
    range.last = range.first = hex_to_codepoint(text, len);
  return range;
}

void codepoint_list(const char *text, size_t len, codepoint *codepoints, int &codepoint_count, int codepoints_capacity) {
  int n = 0;
  split(text, len, ' ', [&](const char *field, size_t len) {
    if (n < codepoints_capacity) {
      codepoints[n] = hex_to_codepoint(field, len);
      ++n;
    }
  });
  codepoint_count = n;
}

template<typename F> void withFile(const char *path, F func) {
  int fd = open(path, O_RDONLY);
  if (fd < 0)
    perror(NULL);
  struct stat st;
  if (fstat(fd, &st) < 0)
    perror(NULL);
  void *bytes = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
  func(bytes, st.st_size);
  munmap(bytes, st.st_size);
  close(fd);
}

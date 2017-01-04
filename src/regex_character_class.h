#ifndef REGEX_CHARACTER_CLASS_H
#define REGEX_CHARACTER_CLASS_H

#include <limits.h>

typedef char RegexCharacterClass[(UCHAR_MAX + CHAR_BIT - 1) / CHAR_BIT];

static inline int regexCharacterClassContains(const RegexCharacterClass klass,
  int ch) {
  return klass[ch / CHAR_BIT] & (1 << ch % CHAR_BIT);
}
static inline int regexCharacterClassAdd(RegexCharacterClass klass, int ch) {
  klass[ch / CHAR_BIT] |= 1 << (ch % CHAR_BIT);
  return ch;
}

#endif // !REGEX_CHARACTER_CLASS_H

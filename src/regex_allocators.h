#ifndef REGEX_ALLOCATORS_H
#define REGEX_ALLOCATORS_H

#include <stddef.h>

typedef struct RegexAllocators {
  void *(*malloc)(size_t);
  void *(*realloc)(void *, size_t);
  void (*free)(void *);
} RegexAllocators;

void regexAllocatorsSet(const RegexAllocators *allocators);
const RegexAllocators *regexAllocatorsGet(void);

#endif // !REGEX_ALLOCATORS_H

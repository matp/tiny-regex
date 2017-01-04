#include <stdlib.h>

#include "regex_allocators.h"

RegexAllocators regexAllocators = {
  .malloc  = malloc,
  .realloc = realloc,
  .free    = free
};

void regexAllocatorsSet(const RegexAllocators *allocators) {
  regexAllocators = *allocators;
}

const RegexAllocators *regexAllocatorsGet(void) {
  return &regexAllocators;
}

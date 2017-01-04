#include "regex_program.h"
#include "regex_vm_pike.h"

int regexProgramRun(const RegexProgram *program, const char *string,
  const char **matches, int nmatches) {
  return regexVmPikeRun(program, string, matches, nmatches);
}

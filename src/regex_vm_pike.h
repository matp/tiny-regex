#ifndef REGEX_VM_PIKE_H
#define REGEX_VM_PIKE_H

#include "regex_program.h"

#define REGEX_VM_PIKE_MAX_MATCHES 20

typedef struct RegexVmPikeThread {
  int visited;
  const RegexProgramInstruction *pc;
  const char *matches[REGEX_VM_PIKE_MAX_MATCHES];
} RegexVmPikeThread;

/* Run program on string. */
int regexVmPikeRun(const RegexProgram *program, const char *string,
  const char **matches, int nmatches);

/* Run program on string (using a previously allocated buffer of at least
 * regexVmPikeEstimateThreads(program) threads). */
int regexVmPikeRunWithThreads(const RegexProgram *program, const char *string,
  const char **matches, int nmatches, RegexVmPikeThread *threads);

/* Upper bound of number of threads required to run program. */
int regexVmPikeEstimateThreads(const RegexProgram *program);

#endif // !REGEX_VM_PIKE_H

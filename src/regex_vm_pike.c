#include <stdlib.h>
#include <string.h>

#include "regex_allocators.h"
#include "regex_vm_pike.h"

typedef struct RegexVmPikeContext {
  const char *string, *sp;
} RegexVmPikeContext;

typedef struct RegexVmPikeThreadList {
  int nthreads;
  RegexVmPikeThread *threads;
} RegexVmPikeThreadList;

// THREAD MANAGEMENT //////////////////////////////////////////////////////////

void regexVmPikeAddThread(RegexVmPikeThreadList *list,
  const RegexProgram *program,
  const RegexProgramInstruction *pc,
  const char *string, const char *sp,
  const char **matches, int nmatches) {

  if (list->threads[pc - program->instructions].visited == sp - string + 1)
    return;
  list->threads[pc - program->instructions].visited = sp - string + 1;

  switch (pc->opcode) {
  case REGEX_PROGRAM_OPCODE_MATCH:
    // fall-through

  /* Characters */
  case REGEX_PROGRAM_OPCODE_CHARACTER:
  case REGEX_PROGRAM_OPCODE_ANY_CHARACTER:
  case REGEX_PROGRAM_OPCODE_CHARACTER_CLASS:
  case REGEX_PROGRAM_OPCODE_CHARACTER_CLASS_NEGATED:
    list->threads[list->nthreads].pc = pc;
    memcpy(list->threads[list->nthreads].matches, matches,
      sizeof (matches[0]) * ((nmatches <= REGEX_VM_PIKE_MAX_MATCHES)
        ? nmatches
        : REGEX_VM_PIKE_MAX_MATCHES));
    ++list->nthreads;
    break;

  /* Control-flow */
  case REGEX_PROGRAM_OPCODE_SPLIT:
    regexVmPikeAddThread(list, program, pc->first,
      string, sp, matches, nmatches);
    regexVmPikeAddThread(list, program, pc->second,
      string, sp, matches, nmatches);
    break;
  case REGEX_PROGRAM_OPCODE_JUMP:
    regexVmPikeAddThread(list, program, pc->target,
      string, sp, matches, nmatches);
    break;

  /* Assertions */
  case REGEX_PROGRAM_OPCODE_ASSERT_BEGIN:
    if (sp == string)
      regexVmPikeAddThread(list, program, pc + 1,
        string, sp, matches, nmatches);
    break;
  case REGEX_PROGRAM_OPCODE_ASSERT_END:
    if (!*sp)
      regexVmPikeAddThread(list, program, pc + 1,
        string, sp, matches, nmatches);
    break;

  /* Saving */
  case REGEX_PROGRAM_OPCODE_SAVE:
    if (pc->save < nmatches && pc->save < REGEX_VM_PIKE_MAX_MATCHES) {
      const char *saved = matches[pc->save];
      matches[pc->save] = sp;
      regexVmPikeAddThread(list, program, pc + 1,
        string, sp, matches, nmatches);
      matches[pc->save] = saved;
    } else {
      regexVmPikeAddThread(list, program, pc + 1,
        string, sp, matches, nmatches);
    }
    break;
  }
}

// PUBLIC API /////////////////////////////////////////////////////////////////

int regexVmPikeRun(const RegexProgram *program, const char *string,
  const char **matches, int nmatches) {
  const RegexAllocators *allocators = regexAllocatorsGet();
  size_t size = sizeof (RegexVmPikeThread)
    * regexVmPikeEstimateThreads(program);
  RegexVmPikeThread *threads;
  int matched;

  if (!(threads = allocators->malloc(size)))
    return -1;

  matched = regexVmPikeRunWithThreads(program, string,
    matches, nmatches, threads);
  allocators->free(threads);
  return matched;
}

int regexVmPikeRunWithThreads(const RegexProgram *program, const char *string,
  const char **matches, int nmatches, RegexVmPikeThread *threads) {
  RegexVmPikeThreadList *current = &(RegexVmPikeThreadList){
    .nthreads = 0, .threads = threads};
  RegexVmPikeThreadList *next    = &(RegexVmPikeThreadList){
    .nthreads = 0, .threads = threads + program->ninstructions};
  int matched = 0;

  memset(threads, 0, sizeof (RegexVmPikeThread) * program->ninstructions * 2);

  regexVmPikeAddThread(current, program, program->instructions,
    string, string, matches, nmatches);

  for (const char *sp = string; ; ++sp) {
    for (int i = 0; i < current->nthreads; ++i) {
      RegexVmPikeThread *thread = current->threads + i;
      switch (thread->pc->opcode) {
      case REGEX_PROGRAM_OPCODE_MATCH:
        matched = 1;
        current->nthreads = 0;
        memcpy(matches, thread->matches,
          sizeof (matches[0]) * ((nmatches <= REGEX_VM_PIKE_MAX_MATCHES)
            ? nmatches
            : REGEX_VM_PIKE_MAX_MATCHES));
        continue;

      /* Characters */
      case REGEX_PROGRAM_OPCODE_CHARACTER:
        if (*sp == thread->pc->ch)
          break;
        continue;
      case REGEX_PROGRAM_OPCODE_ANY_CHARACTER:
        if (*sp)
          break;
        continue;
      case REGEX_PROGRAM_OPCODE_CHARACTER_CLASS:
        if (regexCharacterClassContains(thread->pc->klass, *sp))
          break;
        continue;
      case REGEX_PROGRAM_OPCODE_CHARACTER_CLASS_NEGATED:
        if (!regexCharacterClassContains(thread->pc->klass, *sp))
          break;
        continue;

      /* Control-flow */
      case REGEX_PROGRAM_OPCODE_SPLIT:
      case REGEX_PROGRAM_OPCODE_JUMP:
        // fall-through

      /* Assertions */
      case REGEX_PROGRAM_OPCODE_ASSERT_BEGIN:
      case REGEX_PROGRAM_OPCODE_ASSERT_END:
        // fall-through

      /* Saving */
      case REGEX_PROGRAM_OPCODE_SAVE:
        // handled in regexVmPikeAddThread()
        abort();
      }

      regexVmPikeAddThread(next, program, thread->pc + 1,
        string, sp + 1, thread->matches, nmatches);
    }

    // swap current and next thread list
    RegexVmPikeThreadList *swap = current;
    current = next;
    next = swap;
    next->nthreads = 0;

    // done if no more threads are running or end of string reached
    if (current->nthreads == 0 || !*sp)
      break;
  }

  return matched;
}

int regexVmPikeEstimateThreads(const RegexProgram *program) {
  return program->ninstructions * 2;
}

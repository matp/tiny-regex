#ifndef REGEX_PROGRAM_H
#define REGEX_PROGRAM_H

#include "regex_character_class.h"

typedef enum RegexProgramOpcode {
  REGEX_PROGRAM_OPCODE_MATCH = 0,
  /* Characters */
  REGEX_PROGRAM_OPCODE_CHARACTER,
  REGEX_PROGRAM_OPCODE_ANY_CHARACTER,
  REGEX_PROGRAM_OPCODE_CHARACTER_CLASS,
  REGEX_PROGRAM_OPCODE_CHARACTER_CLASS_NEGATED,
  /* Control-flow */
  REGEX_PROGRAM_OPCODE_SPLIT,
  REGEX_PROGRAM_OPCODE_JUMP,
  /* Assertions */
  REGEX_PROGRAM_OPCODE_ASSERT_BEGIN,
  REGEX_PROGRAM_OPCODE_ASSERT_END,
  /* Saving */
  REGEX_PROGRAM_OPCODE_SAVE
} RegexProgramOpcode;

typedef struct RegexProgramInstruction RegexProgramInstruction;

struct RegexProgramInstruction {
  RegexProgramOpcode opcode;
  union {
    /* REGEX_PROGRAM_OPCODE_CHARACTER */
    struct { int ch; };
    /* REGEX_PROGRAM_OPCODE_CHARACTER_CLASS,
     * REGEX_PROGRAM_OPCODE_CHARACTER_CLASS_NEGATED */
    struct { RegexCharacterClass klass; };
    /* REGEX_PROGRAM_OPCODE_SPLIT */
    struct { RegexProgramInstruction *first, *second; };
    /* REGEX_PROGRAM_OPCODE_JUMP */
    struct { RegexProgramInstruction *target; };
    /* REGEX_PROGRAM_OPCODE_SAVE */
    struct { int save; };
  };
};

typedef struct RegexProgram {
  int ninstructions;
  RegexProgramInstruction instructions[];
} RegexProgram;

/* Run program on string. */
int regexProgramRun(const RegexProgram *program, const char *string,
  const char **matches, int nmatches);

#endif // !REGEX_PROGRAM_H

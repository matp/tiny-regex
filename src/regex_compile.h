#ifndef REGEX_COMPILE_H
#define REGEX_COMPILE_H

#include "regex_node.h"
#include "regex_program.h"

/* Compile a pattern. */
RegexProgram *regexCompile(const char *pattern);

/* Compile a parsed pattern. */
RegexProgram *regexCompileNode(const RegexNode *root);

/* Compile a parsed pattern (using a previously allocated program with at least
 * regexCompileEstimateInstructions(root) instructions). */
RegexProgram *regexCompileNodeWithProgram(const RegexNode *root,
  RegexProgram *program);

/* Upper bound of number of instructions required to compile parsed pattern. */
int regexCompileEstimateInstructions(const RegexNode *root);

/* Free a compiled program. */
void regexCompileFree(RegexProgram *program);

#endif // !REGEX_COMPILE_H

#ifndef REGEX_PARSE_H
#define REGEX_PARSE_H

#include "regex_node.h"

/* Parse a pattern. */
RegexNode *regexParse(const char *pattern);

/* Parse a pattern (using a previously allocated buffer of at least
 * regexParseEstimateNodes(pattern) nodes). */
RegexNode *regexParseWithNodes(const char *pattern, RegexNode *nodes);

/* Upper bound of number of nodes required to parse pattern. */
int regexParseEstimateNodes(const char *pattern);

/* Free a parsed pattern. */
void regexParseFree(RegexNode *root);

#endif // !REGEX_PARSE_H

#include <assert.h>
#include <string.h>

#include "regex_allocators.h"
#include "regex_parse.h"

typedef struct RegexParseContext {
  const char *sp;
  RegexNode *stack, *output;
} RegexParseContext;

// SHUNTING YARD STACK HANDLING ///////////////////////////////////////////////

static inline RegexNode *regexParsePush(RegexParseContext *context,
  const RegexNode *node) {
  assert(context->stack <= context->output);
  *context->stack = *node;
  return context->stack++;
}

static inline RegexNode *regexParseDrop(RegexParseContext *context) {
  return --context->stack;
}

static inline RegexNode *regexParseConsume(RegexParseContext *context) {
  *--context->output = *--context->stack;
  return context->output;
}

static inline RegexNode *regexParseConcatenate(RegexParseContext *context,
  const RegexNode *bottom) {
  if (context->stack == bottom)
    regexParsePush(context, &(RegexNode){.type = REGEX_NODE_TYPE_EPSILON});
  else {
    while (context->stack - 1 > bottom) {
      RegexNode *right = regexParseConsume(context);
      RegexNode *left  = regexParseConsume(context);
      regexParsePush(context, &(RegexNode){
        .type  = REGEX_NODE_TYPE_CONCATENATION,
        .left  = left,
        .right = right});
    }
  }
  return context->stack - 1;
}

// PARSING ////////////////////////////////////////////////////////////////////

RegexNode *regexParseCharacterClass(RegexParseContext *context) {
  RegexNodeType type = (*context->sp == '^')
    ? (++context->sp, REGEX_NODE_TYPE_CHARACTER_CLASS_NEGATED)
    : REGEX_NODE_TYPE_CHARACTER_CLASS;
  const char *from = context->sp;

  for (;;) {
    int ch = *context->sp++;
    switch (ch) {
    case '\0':
      // premature end of character class
      return NULL;
    case ']':
      if (context->sp - 1 == from)
        goto CHARACTER;
      return regexParsePush(context, &(RegexNode){.type = type,
        .from = from, .to = context->sp - 1});
    case '\\':
      ch = *context->sp++;
      // fall-through
    default:
    CHARACTER:
      if (*context->sp == '-' && context->sp[1] != ']') {
        if (context->sp[1] < ch)
          // empty range in character class
          return NULL;
        context->sp += 2;
      }
      break;
    }
  }
}

RegexNode *regexParseInterval(RegexParseContext *context) {
  const char *from = context->sp;
  int nmin, nmax;

  for (nmin = 0; *context->sp >= '0' && *context->sp <= '9'; ++context->sp)
    nmin = (nmin * 10) + (*context->sp - '0');

  if (*context->sp == ',') {
    ++context->sp;
    if (*from != ',' && *context->sp == '}')
      nmax = -1;
    else {
      for (nmax = 0; *context->sp >= '0' && *context->sp <= '9'; ++context->sp)
        nmax = (nmax * 10) + (*context->sp - '0');
      if (*(context->sp - 1) == ',' || *context->sp != '}' || nmax < nmin) {
        context->sp = from;
        return NULL;
      }
    }
  } else if (*from != '}' && *context->sp == '}') {
    nmax = nmin;
  } else {
    context->sp = from;
    return NULL;
  }

  ++context->sp;
  return regexParsePush(context, &(RegexNode){
    .type       = REGEX_NODE_TYPE_QUANTIFIER,
    .nmin       = nmin,
    .nmax       = nmax,
    .greedy     = (*context->sp == '?') ? (++context->sp, 0) : 1,
    .quantified = regexParseConsume(context)});
}

RegexNode *regexParseContext(RegexParseContext *context, int depth) {
  RegexNode *bottom = context->stack;

  for (;;) {
    RegexNode *left, *right;
    int ch = *context->sp++;
    switch (ch) {
    /* Characters */
    case '\\':
      ch = *context->sp++;
      // fall-through
    default:
    CHARACTER:
      regexParsePush(context, &(RegexNode){.type = REGEX_NODE_TYPE_CHARACTER,
        .ch = ch});
      break;
    case '.':
      regexParsePush(context, &(RegexNode){
        .type = REGEX_NODE_TYPE_ANY_CHARACTER});
      break;
    case '[':
      if (!regexParseCharacterClass(context))
        return NULL;
      break;

    /* Composites */
    case '|':
      left = regexParseConcatenate(context, bottom);
      if (!(right = regexParseContext(context, depth)))
        return NULL;
      if (left->type == REGEX_NODE_TYPE_EPSILON && right->type == left->type) {
        regexParseDrop(context);
      } else if (left->type == REGEX_NODE_TYPE_EPSILON) {
        right = regexParseConsume(context);
        regexParseDrop(context);
        regexParsePush(context, &(RegexNode){
          .type       = REGEX_NODE_TYPE_QUANTIFIER,
          .nmin       = 0,
          .nmax       = 1,
          .greedy     = 1,
          .quantified = right});
      } else if (right->type == REGEX_NODE_TYPE_EPSILON) {
        regexParseDrop(context);
        left = regexParseConsume(context);
        regexParsePush(context, &(RegexNode){
          .type       = REGEX_NODE_TYPE_QUANTIFIER,
          .nmin       = 0,
          .nmax       = 1,
          .greedy     = 1,
          .quantified = left});
      } else {
        right = regexParseConsume(context);
        left  = regexParseConsume(context);
        regexParsePush(context, &(RegexNode){
          .type  = REGEX_NODE_TYPE_ALTERNATION,
          .left  = left,
          .right = right});
      }
      return bottom;

    /* Quantifiers */
#define QUANTIFIER(ch, min, max)                                             \
    case ch:                                                                 \
      if (context->stack == bottom)                                          \
        goto CHARACTER;                                                      \
      regexParsePush(context, &(RegexNode){                                  \
        .type       = REGEX_NODE_TYPE_QUANTIFIER,                            \
        .nmin       = min,                                                   \
        .nmax       = max,                                                   \
        .greedy     = (*context->sp == '?') ? (++context->sp, 0) : 1,        \
        .quantified = regexParseConsume(context)});                          \
      break
    QUANTIFIER('?', 0,  1);
    QUANTIFIER('*', 0, -1);
    QUANTIFIER('+', 1, -1);
#undef QUANTIFIER
    case '{':
      if ((context->stack == bottom) || !regexParseInterval(context))
        goto CHARACTER;
      break;

    /* Anchors */
    case '^':
      regexParsePush(context, &(RegexNode){
        .type = REGEX_NODE_TYPE_ANCHOR_BEGIN});
      break;
    case '$':
      regexParsePush(context, &(RegexNode){
        .type = REGEX_NODE_TYPE_ANCHOR_END});
      break;

    /* Captures */
    case '(':
      if (!regexParseContext(context, depth + 1))
        return NULL;
      regexParsePush(context, &(RegexNode){.type = REGEX_NODE_TYPE_CAPTURE,
        .captured = regexParseConsume(context)});
      break;
    case ')':
      if (depth > 0)
        return regexParseConcatenate(context, bottom);
      else
        // unmatched close parenthesis
        return NULL;

    /* End of string */
    case '\0':
      if (depth == 0)
        return regexParseConcatenate(context, bottom);
      else
        // unmatched open parenthesis
        return NULL;
    }
  }
}

// PUBLIC API /////////////////////////////////////////////////////////////////

RegexNode *regexParse(const char *pattern) {
  const RegexAllocators *allocators = regexAllocatorsGet();
  size_t size = sizeof (RegexNode) * regexParseEstimateNodes(pattern);
  RegexNode *nodes;

  if (!(nodes = allocators->malloc(size)))
    return NULL;

  if (!regexParseWithNodes(pattern, nodes)) {
    allocators->free(nodes);
    return NULL;
  }

  return nodes;
}

RegexNode *regexParseWithNodes(const char *pattern, RegexNode *nodes) {
  RegexParseContext *context = &(RegexParseContext){.sp = pattern,
    .stack = nodes, .output = nodes + regexParseEstimateNodes(pattern)};
  return regexParseContext(context, 0);
}

int regexParseEstimateNodes(const char *pattern) {
  return strlen(pattern) * 2;
}

void regexParseFree(RegexNode *root) {
  const RegexAllocators *allocators = regexAllocatorsGet();
  allocators->free(root);
}

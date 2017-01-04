#ifndef REGEX_NODE_H
#define REGEX_NODE_H

typedef enum RegexNodeType {
  REGEX_NODE_TYPE_EPSILON = 0,
  /* Characters */
  REGEX_NODE_TYPE_CHARACTER,
  REGEX_NODE_TYPE_ANY_CHARACTER,
  REGEX_NODE_TYPE_CHARACTER_CLASS,
  REGEX_NODE_TYPE_CHARACTER_CLASS_NEGATED,
  /* Composites */
  REGEX_NODE_TYPE_CONCATENATION,
  REGEX_NODE_TYPE_ALTERNATION,
  /* Quantifiers */
  REGEX_NODE_TYPE_QUANTIFIER,
  /* Anchors */
  REGEX_NODE_TYPE_ANCHOR_BEGIN,
  REGEX_NODE_TYPE_ANCHOR_END,
  /* Captures */
  REGEX_NODE_TYPE_CAPTURE
} RegexNodeType;

typedef struct RegexNode RegexNode;

struct RegexNode {
  RegexNodeType type;
  union {
    /* REGEX_NODE_TYPE_CHARACTER */
    struct { int ch; };
    /* REGEX_NODE_TYPE_CHARACTER_CLASS,
     * REGEX_NODE_TYPE_CHARACTER_CLASS_NEGATED */
    struct { const char *from, *to; };
    /* REGEX_NODE_TYPE_QUANTIFIER */
    struct { int nmin, nmax, greedy; RegexNode *quantified; };
    /* REGEX_NODE_TYPE_CONCATENATION,
     * REGEX_NODE_TYPE_ALTERNATION */
    struct { RegexNode *left, *right; };
    /* REGEX_NODE_TYPE_CAPTURE */
    struct { RegexNode *captured; };
  };
};

#endif // !REGEX_NODE_H

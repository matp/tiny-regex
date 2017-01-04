#include "regex_allocators.h"
#include "regex_compile.h"
#include "regex_parse.h"

typedef struct RegexCompileContext {
  RegexProgramInstruction *pc;
  int ncaptures;
} RegexCompileContext;

// COMPILATION ////////////////////////////////////////////////////////////////

int regexCompileCountInstructions(const RegexNode *node) {
  int ninstructions;

  switch (node->type) {
  case REGEX_NODE_TYPE_EPSILON:
    return 0;

  /* Characters */
  case REGEX_NODE_TYPE_CHARACTER:
  case REGEX_NODE_TYPE_ANY_CHARACTER:
  case REGEX_NODE_TYPE_CHARACTER_CLASS:
  case REGEX_NODE_TYPE_CHARACTER_CLASS_NEGATED:
    return 1;

  /* Composites */
  case REGEX_NODE_TYPE_CONCATENATION:
    return regexCompileCountInstructions(node->left)
      + regexCompileCountInstructions(node->right);
  case REGEX_NODE_TYPE_ALTERNATION:
    return 2 + regexCompileCountInstructions(node->left)
      + regexCompileCountInstructions(node->right);

  /* Quantifiers */
  case REGEX_NODE_TYPE_QUANTIFIER:
    ninstructions = regexCompileCountInstructions(node->quantified);
    if (node->nmax >= node->nmin)
      return node->nmin * ninstructions
        + (node->nmax - node->nmin) * (ninstructions + 1);
    else
      return 1 + (node->nmin ? node->nmin * ninstructions : ninstructions + 1);

  /* Anchors */
  case REGEX_NODE_TYPE_ANCHOR_BEGIN:
  case REGEX_NODE_TYPE_ANCHOR_END:
    return 1;

  /* Captures */
  case REGEX_NODE_TYPE_CAPTURE:
    return 2 + regexCompileCountInstructions(node->captured);
  }
}

int regexCompileNodeIsAnchored(const RegexNode *node) {
  switch (node->type) {
  case REGEX_NODE_TYPE_EPSILON:
    return 0;

  /* Characters */
  case REGEX_NODE_TYPE_CHARACTER:
  case REGEX_NODE_TYPE_ANY_CHARACTER:
  case REGEX_NODE_TYPE_CHARACTER_CLASS:
  case REGEX_NODE_TYPE_CHARACTER_CLASS_NEGATED:
    return 0;

  /* Composites */
  case REGEX_NODE_TYPE_CONCATENATION:
    return regexCompileNodeIsAnchored(node->left);
  case REGEX_NODE_TYPE_ALTERNATION:
    return regexCompileNodeIsAnchored(node->left)
      && regexCompileNodeIsAnchored(node->right);

  /* Quantifiers */
  case REGEX_NODE_TYPE_QUANTIFIER:
    return regexCompileNodeIsAnchored(node->quantified);

  /* Anchors */
  case REGEX_NODE_TYPE_ANCHOR_BEGIN:
    return 1;
  case REGEX_NODE_TYPE_ANCHOR_END:
    return 0;

  /* Captures */
  case REGEX_NODE_TYPE_CAPTURE:
    return regexCompileNodeIsAnchored(node->captured);
  }
}

static inline RegexProgramInstruction *regexCompileEmit(
  RegexCompileContext *context, const RegexProgramInstruction *instruction) {
  *context->pc = *instruction;
  return context->pc++;
}

RegexProgramInstruction *regexCompileCharacterClass(const RegexNode *node,
  RegexProgramInstruction *instruction) {
  const char *sp = node->from;

  for (;;) {
    int ch = *sp++;
    switch (ch) {
    case ']':
      if (sp - 1 == node->from)
        goto CHARACTER;
      return instruction;
    case '\\':
      ch = *sp++;
      // fall-through
    default:
    CHARACTER:
      if (*sp == '-' && sp[1] != ']') {
        for (; ch <= sp[1]; ++ch)
          regexCharacterClassAdd(instruction->klass, ch);
        sp += 2;
      } else {
        regexCharacterClassAdd(instruction->klass, ch);
      }
      break;
    }
  }
}

RegexProgramInstruction *regexCompileContext(RegexCompileContext *context,
  const RegexNode *node) {
  RegexProgramInstruction *bottom = context->pc, *split, *jump, *last;
  int ncaptures = context->ncaptures, capture;

  switch (node->type) {
  case REGEX_NODE_TYPE_EPSILON:
    break;

  /* Characters */
  case REGEX_NODE_TYPE_CHARACTER:
    regexCompileEmit(context, &(RegexProgramInstruction){
      .opcode = REGEX_PROGRAM_OPCODE_CHARACTER, .ch = node->ch});
    break;
  case REGEX_NODE_TYPE_ANY_CHARACTER:
    regexCompileEmit(context, &(RegexProgramInstruction){
      .opcode = REGEX_PROGRAM_OPCODE_ANY_CHARACTER});
    break;
  case REGEX_NODE_TYPE_CHARACTER_CLASS:
    regexCompileCharacterClass(node,
      regexCompileEmit(context, &(RegexProgramInstruction){
        .opcode = REGEX_PROGRAM_OPCODE_CHARACTER_CLASS}));
    break;
  case REGEX_NODE_TYPE_CHARACTER_CLASS_NEGATED:
    regexCompileCharacterClass(node,
      regexCompileEmit(context, &(RegexProgramInstruction){
        .opcode = REGEX_PROGRAM_OPCODE_CHARACTER_CLASS_NEGATED}));
    break;

  /* Composites */
  case REGEX_NODE_TYPE_CONCATENATION:
    regexCompileContext(context, node->left);
    regexCompileContext(context, node->right);
    break;
  case REGEX_NODE_TYPE_ALTERNATION:
    split = regexCompileEmit(context, &(RegexProgramInstruction){
      .opcode = REGEX_PROGRAM_OPCODE_SPLIT});
    split->first = regexCompileContext(context, node->left);
    jump = regexCompileEmit(context, &(RegexProgramInstruction){
      .opcode = REGEX_PROGRAM_OPCODE_JUMP});
    split->second = regexCompileContext(context, node->right);
    jump->target = context->pc;
    break;

  /* Quantifiers */
  case REGEX_NODE_TYPE_QUANTIFIER:
    for (int i = 0; i < node->nmin; ++i) {
      context->ncaptures = ncaptures;
      last = regexCompileContext(context, node->quantified);
    }
    if (node->nmax > node->nmin) {
      for (int i = 0; i < node->nmax - node->nmin; ++i) {
        context->ncaptures = ncaptures;
        split = regexCompileEmit(context, &(RegexProgramInstruction){
          .opcode = REGEX_PROGRAM_OPCODE_SPLIT});
        split->first  = regexCompileContext(context, node->quantified);
        split->second = context->pc;
        if (!node->greedy) {
          RegexProgramInstruction *swap = split->first;
          split->first  = split->second;
          split->second = swap;
        }
      }
    } else if (node->nmax == -1) {
      split = regexCompileEmit(context, &(RegexProgramInstruction){
        .opcode = REGEX_PROGRAM_OPCODE_SPLIT});
      if (node->nmin == 0) {
        split->first  = regexCompileContext(context, node->quantified);
        jump = regexCompileEmit(context, &(RegexProgramInstruction){
          .opcode = REGEX_PROGRAM_OPCODE_JUMP});
        split->second = context->pc;
        jump->target  = split;
      } else {
        split->first  = last;
        split->second = context->pc;
      }
      if (!node->greedy) {
        RegexProgramInstruction *swap = split->first;
        split->first  = split->second;
        split->second = swap;
      }
    }
    break;

  /* Anchors */
  case REGEX_NODE_TYPE_ANCHOR_BEGIN:
    regexCompileEmit(context, &(RegexProgramInstruction){
      .opcode = REGEX_PROGRAM_OPCODE_ASSERT_BEGIN});
    break;
  case REGEX_NODE_TYPE_ANCHOR_END:
    regexCompileEmit(context, &(RegexProgramInstruction){
      .opcode = REGEX_PROGRAM_OPCODE_ASSERT_END});
    break;

  /* Captures */
  case REGEX_NODE_TYPE_CAPTURE:
    capture = context->ncaptures++ * 2;
    regexCompileEmit(context, &(RegexProgramInstruction){
      .opcode = REGEX_PROGRAM_OPCODE_SAVE, .save = capture});
    regexCompileContext(context, node->captured);
    regexCompileEmit(context, &(RegexProgramInstruction){
      .opcode = REGEX_PROGRAM_OPCODE_SAVE, .save = capture + 1});
    break;
  }

  return bottom;
}

// PUBLIC API /////////////////////////////////////////////////////////////////

RegexProgram *regexCompile(const char *pattern) {
  RegexNode *root;
  RegexProgram *program;

  if (!(root = regexParse(pattern)))
    return NULL;

  program = regexCompileNode(root);
  regexParseFree(root);
  return program;
}

RegexProgram *regexCompileNode(const RegexNode *root) {
  const RegexAllocators *allocators = regexAllocatorsGet();
  size_t size = sizeof (RegexProgram) + sizeof (RegexProgramInstruction)
    * regexCompileEstimateInstructions(root);
  RegexProgram *program;

  if (!(program = allocators->malloc(size)))
    return NULL;

  if (!regexCompileNodeWithProgram(root, program)) {
    allocators->free(program);
    return NULL;
  }

  return program;
}

RegexProgram *regexCompileNodeWithProgram(const RegexNode *root,
  RegexProgram *program) {

  // add capture node for entire match
  root = &(RegexNode){.type = REGEX_NODE_TYPE_CAPTURE,
    .captured = (RegexNode *)root};

  // add .*? unless pattern starts with ^
  if (!regexCompileNodeIsAnchored(root))
    root = &(RegexNode){
      .type = REGEX_NODE_TYPE_CONCATENATION,
      .left = &(RegexNode){
        .type       = REGEX_NODE_TYPE_QUANTIFIER,
        .nmin       = 0,
        .nmax       = -1,
        .greedy     = 0,
        .quantified = &(RegexNode){.type = REGEX_NODE_TYPE_ANY_CHARACTER}},
      .right = (RegexNode *)root};

  // compile
  RegexCompileContext *context = &(RegexCompileContext){
    .pc = program->instructions, .ncaptures = 0};
  regexCompileContext(context, root);

  // emit final match instruction
  regexCompileEmit(context, &(RegexProgramInstruction){
    .opcode = REGEX_PROGRAM_OPCODE_MATCH});

  // set total number of instructions
  program->ninstructions = context->pc - program->instructions;

  return program;
}

int regexCompileEstimateInstructions(const RegexNode *root) {
  return regexCompileCountInstructions(root)
    /* .*? is added unless pattern starts with ^,
     * save instructions are added for beginning and end of match,
     * a final match instruction is added to the end of the program */
    + !regexCompileNodeIsAnchored(root) * 3 + 2 + 1;
}

/* Free a compiled program. */
void regexCompileFree(RegexProgram *program) {
  const RegexAllocators *allocators = regexAllocatorsGet();
  allocators->free(program);
}

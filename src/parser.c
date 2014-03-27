#include "san.h"
#include "parser.h"

typedef struct {
  san_vector_t const *tokens;
  san_token_t const *tokenPtr;
  san_vector_t nodeStack;

  san_vector_t *errors;
} parser_state_t;

typedef int (*parser_t)(parser_state_t *state);

#define SAN_MATCH       1
#define SAN_NO_MATCH    2
#define SAN_ERR_MATCH   3

#define parseError(__state, __code, ...) do { \
  san_error_t err; \
  memset(&err, 0, sizeof err); \
  err.code = __code; \
  err.line = (__state)->tokenPtr->line; \
  err.column = (__state)->tokenPtr->column; \
  sprintf(err.msg, __code##_MSG, __VA_ARGS__); \
  sanv_push((__state)->errors, &err); \
} while (0)

int parse_exp(parser_state_t const *state, parser_state_t *newState);
int parse_additive_exp(parser_state_t const *state, parser_state_t *newState);

/*
 * Parser helper functions
 */
static inline int head_is(parser_state_t const *state, int tokenType) {
  return state->tokenPtr->type == tokenType;
}

static inline parser_state_t consume_white_space(parser_state_t const *state) {
  parser_state_t newState = *state;
  while (head_is(&newState, SAN_TOKEN_WHITE_SPACE))
    newState.tokenPtr += 1;
  return newState;
}

static inline parser_state_t create_state() {
  parser_state_t state;
  sanv_create(&state.nodeStack, sizeof(san_node_t));
  return state;
}

/*
 * rawTokens
 *
 * Interpolates between two parser states. The output is a string of characters
 * starting at the first token in state s1 and ending at the last token of s2.
 */
void rawTokens(parser_state_t const *s1, parser_state_t const *s2, char *out, size_t max) {
  san_token_t const *curTok = s1->tokenPtr;
  int tokenIndex = 0, i;
  for (i = 0; i < max-1; ++i) {
    if (curTok == s2->tokenPtr) {
      if (tokenIndex == s2->tokenPtr->rawSize) {
        break;
      }
    }
    out[i] = curTok->raw[tokenIndex++];
    if (tokenIndex > strlen(curTok->raw)-1) {
      tokenIndex = 0;
      ++curTok;
      if (curTok->type == SAN_TOKEN_END) break;
    }
  }
  out[i+1] = '\0';
}

static inline parser_state_t advance_state(parser_state_t *state) {
  parser_state_t newState = consume_white_space(state);
  newState.tokenPtr = state->tokenPtr + 1;
  return newState;
}

int push_node(parser_state_t *state, int type) {
  san_node_t node;
  node.type = type;

  parser_state_t noWhitespace = consume_white_space(state);
  node.token = noWhitespace.tokenPtr;
  sanv_create(&node.children, sizeof(san_node_t));
  if (state->nodeStack.capacity == 0) {
    sanv_create(&state->nodeStack, sizeof(san_node_t));
  }
  sanv_push(&state->nodeStack, &node);
  return state->nodeStack.size - 1;
}

void pop_node(parser_state_t *state) {
  void *tmp = NULL;
  sanv_pop(&state->nodeStack, tmp);
}

char *fmt(int n){
  switch(n) {
  case 0: return "NULL"; break;
  case 1: return "Root"; break;
  case 2: return "Expression"; break;
  case 3: return "Identifier"; break;
  case 4: return "Number literal"; break;
  case 5: return "Additive expr"; break;
  case 6: return "Mult expr"; break;
  case SAN_PARSER_STATEMENT: return "Statement"; break;
  case SAN_PARSER_VARIABLE_DEFINITION: return "Variable definition"; break;
  case SAN_PARSER_LVALUE: return "L-value"; break;
  case SAN_PARSER_FUNCTION_PARAMETER_LIST: return "Function parameter list"; break;
  case SAN_PARSER_FUNCTION_PARAMETER: return "Function parameter"; break;
  case SAN_PARSER_PRIMARY_EXPRESSION: return "Primary expression"; break;
  }
  return "ERR";
}

void add_child(parser_state_t *state, int nodeIndex) {
  san_node_t tmp;
  san_node_t *node = sanv_nth(&state->nodeStack, nodeIndex);
  printf("\nNODE STACK\n");
  SAN_VECTOR_FOR_EACH(state->nodeStack, i, san_node_t, node)
    printf("%d: %s, '%s'\n", i, fmt(node->type), node->token->raw);
  SAN_VECTOR_END_FOR_EACH
  sanv_pop(&state->nodeStack, &tmp);
  sanv_push(&node->children, &tmp);
  printf("Popping [node type=%s, size=%d] onto [node type=%s, size=%d]\n", fmt(tmp.type), tmp.children.size, fmt(node->type), node->children.size);
  printf("\n");
}

static inline parser_state_t clone_state(parser_state_t const *state) {
  parser_state_t newState = *state;
  sanv_create(&newState.nodeStack, sizeof(san_node_t));
  SAN_VECTOR_FOR_EACH(state->nodeStack, i, san_node_t, node)
    sanv_push(&newState.nodeStack, node);
  SAN_VECTOR_END_FOR_EACH
  return newState;
}

/*
 * Parser functions
 */

int parse_terminal(parser_state_t const *state, parser_state_t *newState, int terminal) {
  if (state->tokenPtr->type == SAN_TOKEN_END) return SAN_NO_MATCH;
  *newState = consume_white_space(state);
  if (head_is(newState, terminal)) {
    *newState = advance_state(newState);
    return SAN_MATCH;
  }
  return SAN_NO_MATCH;
}

int parse_number_literal(parser_state_t const *state, parser_state_t *newState) {
  printf("Parsing number literal\n");
        fflush(stdout);
  *newState = clone_state(state);
  push_node(newState, SAN_PARSER_NUMBER_LITERAL);
  parser_state_t s1;
  if (parse_terminal(newState, &s1, SAN_TOKEN_NUMBER) != SAN_NO_MATCH) {
    *newState = s1;
    return SAN_MATCH;
  }
  return SAN_NO_MATCH;
}

int parse_primary_exp(parser_state_t const *state, parser_state_t *newState) {
  printf("Parsing primary expression\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_PRIMARY_EXPRESSION);
  parser_state_t s1;

  if (parse_number_literal(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);
    *newState = s1;
    return SAN_MATCH;
  } else if (parse_terminal(newState, &s1, SAN_TOKEN_IDENTIFIER) != SAN_NO_MATCH) {
    *newState = s1;
    return SAN_MATCH;
  }
  return SAN_NO_MATCH;
}

int parse_mult_exp(parser_state_t const *state, parser_state_t *newState) {
  printf("Parsing mult exp\n");
  fflush(stdout);
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_MULTIPLICATIVE_EXPRESSION);

  parser_state_t s1, s2, s3;
  if (parse_primary_exp(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);
    *newState = s1;
    if (parse_terminal(&s1, &s2, SAN_TOKEN_TIMES) != SAN_NO_MATCH) {
      if (parse_primary_exp(&s2, &s3) != SAN_NO_MATCH) {
        add_child(&s3, nodeIndex);
        *newState = s3;
        return SAN_MATCH;
      } else {
        // Expected expression
        return SAN_ERR_MATCH;
      }
    }
    return SAN_MATCH;
  }

  return SAN_NO_MATCH;
}

int parse_additive_exp(parser_state_t const *state, parser_state_t *newState) {
  printf("Parsing additive exp\n");
  fflush(stdout);
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_ADDITIVE_EXPRESSION);

  parser_state_t s1, s2;
  if (parse_mult_exp(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);
    *newState = s1;

    s2 = clone_state(&s1);
    while (parse_terminal(&s2, &s2, SAN_TOKEN_PLUS) != SAN_NO_MATCH) {
      if (parse_mult_exp(&s2, &s2) != SAN_NO_MATCH) {
        add_child(&s2, nodeIndex);
        *newState = s2;
      } else {
        char raw[1024];
        rawTokens(state, &s2, raw, 1024);
        parseError(state, SAN_ERROR_EXPECTED_TERM, raw);
        return SAN_ERR_MATCH;
      }
    }
    return SAN_MATCH;
  }

  return SAN_NO_MATCH;
}

int parse_exp(parser_state_t const *state, parser_state_t *newState) {
  printf("Parsing expression\n");
        fflush(stdout);
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_EXPRESSION);
  parser_state_t s1;
  if (parse_additive_exp(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);
    *newState = s1;
    return SAN_MATCH;
  }
  return SAN_NO_MATCH;
}

int parse_func_param(parser_state_t *state, parser_state_t *newState) {
  printf("Parsing function parameter\n");
  *newState = clone_state(state);
  push_node(newState, SAN_PARSER_FUNCTION_PARAMETER);

  if (parse_terminal(newState, newState, SAN_TOKEN_IDENTIFIER) != SAN_NO_MATCH) {
    return SAN_MATCH;
  }

  return SAN_NO_MATCH;
}

int parse_func_param_list(parser_state_t *state, parser_state_t *newState) {
  printf("Parsing function parameters\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_FUNCTION_PARAMETER_LIST);
  int result = SAN_NO_MATCH;
  parser_state_t s1;

  while (parse_func_param(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);
    *newState = s1;
    result = SAN_MATCH;
  }

  return result;
}

int parse_lvalue(parser_state_t *state, parser_state_t *newState) {
  printf("Parsing lvalue\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_LVALUE);
  int result = SAN_NO_MATCH;

  if (parse_terminal(newState, newState, SAN_TOKEN_IDENTIFIER) != SAN_NO_MATCH) {
    result = SAN_MATCH;
  }

  parser_state_t s1;
  if (parse_func_param_list(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);
    *newState = s1;
  }

  return result;
}

int parse_variable_defn(parser_state_t *state, parser_state_t *newState) {
  printf("Parsing variable definition\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_VARIABLE_DEFINITION);
  parser_state_t s1, s2, s3;
  if (parse_lvalue(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);

    if (parse_terminal(&s1, &s2, SAN_TOKEN_EQUALS) != SAN_NO_MATCH) {
      if (parse_exp(&s2, &s3) != SAN_NO_MATCH) {
        add_child(&s3, nodeIndex);
        *newState = s3;
        return SAN_MATCH;
      } else {
        char raw[1024];
        rawTokens(state, &s2, raw, 1024);
        parseError(state, SAN_ERROR_EXPECTED_EXPRESSION, raw);
        return SAN_ERR_MATCH;
      }
    } else {
      char raw[1024];
      rawTokens(newState, &s1, raw, 1024);
      parseError(newState, SAN_ERROR_EXPECTED_TOKEN, "=", raw);
      *newState = s1;
      return SAN_ERR_MATCH;
    }
  }
  return SAN_NO_MATCH;
}

int parse_stmt(parser_state_t *state, parser_state_t *newState) {
  printf("Parsing statement\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_STATEMENT);
  parser_state_t s1;
  if (parse_variable_defn(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);
    *newState = s1;
    return SAN_MATCH;
  }
  return SAN_NO_MATCH;
}

void indent(int n) {
  for(int i = 0; i < n; ++i) printf(" ");
}

void dump_ast(san_node_t *ast, int ind) {
  indent(ind);
  int type = ast != NULL ? ast->type : -1;
  printf("[node type: %s, ptr: '%s', nchildren: %d]\n", fmt(type), ast->token->raw, ast->children.size);
  fflush(stdout);
  SAN_VECTOR_FOR_EACH(ast->children, i, san_node_t, node)
    dump_ast(node, ind+2);
  SAN_VECTOR_END_FOR_EACH
}

int parse(san_vector_t const *tokens, san_node_t *ast, san_vector_t *errors) {
  parser_state_t state;

  ast->type = SAN_PARSER_ROOT;

  if (tokens == NULL || tokens->size == 0) {
    return SAN_FAIL;
  }

  state = create_state();
  state.tokens = tokens;
  state.errors = errors;
  state.tokenPtr = tokens->elems;

  int firstIndex = push_node(&state, SAN_PARSER_ROOT);
  parser_state_t s1;
  if (parse_stmt(&state, &s1) != SAN_NO_MATCH) {
    add_child(&s1, firstIndex);
    state = s1;
  } else {}
  *ast = *(san_node_t*)sanv_nth(&state.nodeStack, firstIndex);

  dump_ast(ast, 0);

  return SAN_OK;
}

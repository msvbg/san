#include "san.h"
#include "parser.h"

typedef struct {
  san_vector_t const *tokens;
  san_token_t const *tokenPtr;
  san_vector_t nodeStack;

  san_vector_t *errors;
  san_vector_t indentStack;
  int indentSensitive;
} parser_state_t;

typedef int (*parser_t)(parser_state_t *state);

#define SAN_MATCH       1
#define SAN_NO_MATCH    2
#define SAN_ERR_MATCH   3

static san_error_t _parseError(parser_state_t *state, int code) {
  san_error_t err;
  memset(&err, 0, sizeof err);
  err.code = code;
  err.line = (state)->tokenPtr->line;
  err.column = (state)->tokenPtr->column;
  return err;
}

#define parseError(__beginState, __endState, __code, ...) do { \
  char raw[1024]; \
  rawTokens((__beginState), (__endState), raw, 1024); \
  san_error_t err = _parseError(__beginState, __code); \
  sprintf(err.msg, __code##_MSG, raw, __VA_ARGS__); \
  sanv_push((__beginState)->errors, &err); \
} while (0)

#define parseError0(__beginState, __endState, __code) do { \
  char raw[1024]; \
  rawTokens((__beginState), (__endState), raw, 1024); \
  san_error_t err = _parseError(__beginState, __code); \
  sprintf(err.msg, __code##_MSG, raw); \
  sanv_push((__beginState)->errors, &err); \
} while (0)

int parse_exp(parser_state_t const *state, parser_state_t *newState);
int parse_additive_exp(parser_state_t const *state, parser_state_t *newState);

/*
 * Parser helper functions
 */
static inline int head_is(parser_state_t const *state, int tokenType) {
    return state->tokenPtr->type == tokenType;
}

static inline int head_len(parser_state_t const *state) {
  return strlen(state->tokenPtr->raw);
}

static inline parser_state_t eat_wspace(parser_state_t const *state) {
  parser_state_t newState = *state;
  while (head_is(&newState, SAN_TOKEN_WHITE_SPACE) ||
    (!state->indentSensitive && head_is(&newState, SAN_TOKEN_INDENTATION)))
    newState.tokenPtr += 1;
  return newState;
}

static inline parser_state_t create_state() {
  parser_state_t state;
  sanv_create(&state.nodeStack, sizeof(san_node_t));
  sanv_create(&state.indentStack, sizeof(int));
  return state;
}

static int at_end(parser_state_t const *state) {
  parser_state_t noWhitespace = eat_wspace(state);
  return noWhitespace.tokenPtr->type == SAN_TOKEN_END;
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
  parser_state_t newState = eat_wspace(state);
  newState.tokenPtr = state->tokenPtr + 1;
  return newState;
}

static int push_node(parser_state_t *state, int type) {
  san_node_t node;
  node.type = type;

  parser_state_t noWhitespace = eat_wspace(state);
  node.token = noWhitespace.tokenPtr;
  sanv_create(&node.children, sizeof(san_node_t));
  if (state->nodeStack.capacity == 0) {
    sanv_create(&state->nodeStack, sizeof(san_node_t));
  }
  sanv_push(&state->nodeStack, &node);
  return state->nodeStack.size - 1;
}

//static void begin_indent(parser_state_t *state, int depth);

char *fmt(int n) {
  switch(n) {
  case 0: return "NULL"; break;
  case SAN_PARSER_ROOT: return "Root"; break;
  case SAN_PARSER_EXPRESSION: return "Expression"; break;
  case SAN_PARSER_IDENTIFIER: return "Identifier"; break;
  case SAN_PARSER_NUMBER_LITERAL: return "Number literal"; break;
  case SAN_PARSER_ADDITIVE_EXPRESSION: return "Additive expr"; break;
  case SAN_PARSER_MULTIPLICATIVE_EXPRESSION: return "Mult expr"; break;
  case SAN_PARSER_VARIABLE_EXPRESSION: return "Variable definition"; break;
  case SAN_PARSER_VARIABLE_LVALUE: return "Variable L-value"; break;
  case SAN_PARSER_FUNCTION_LVALUE: return "Function L-value"; break;
  case SAN_PARSER_FUNCTION_PARAMETER_LIST: return "Function parameter list"; break;
  case SAN_PARSER_FUNCTION_PARAMETER: return "Function parameter"; break;
  case SAN_PARSER_BLOCK: return "Block"; break;
  case SAN_PARSER_PRIMARY_EXPRESSION: return "Primary expression"; break;
  case SAN_PARSER_IF_EXPRESSION: return "If expression"; break;
  case SAN_PARSER_FN_EXPRESSION: return "Function expression"; break;
  case SAN_PARSER_LIST: return "List"; break;
  case SAN_PARSER_PIPE_EXPRESSION: return "Pipe expression"; break;
  }
  return "ERR";
}

void add_child(parser_state_t *state, int nodeIndex) {
  san_node_t tmp;
  san_node_t *node = sanv_nth(&state->nodeStack, nodeIndex);
  //san_dbg("\nNODE STACK\n");
  //SAN_VECTOR_FOR_EACH(state->nodeStack, i, san_node_t, node)
    //san_dbg("%d: %s, '%s'\n", i, fmt(node->type), node->token->raw);
  //SAN_VECTOR_END_FOR_EACH
  sanv_pop(&state->nodeStack, &tmp);
  sanv_push(&node->children, &tmp);
  //san_dbg("Popping [node type=%s, size=%d] onto [node type=%s, size=%d]\n", fmt(tmp.type), tmp.children.size, fmt(node->type), node->children.size);
  //san_dbg("\n");
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
 * Indentation
 */
static inline int indent_depth(parser_state_t *state) {
  return state->indentStack.size > 0 ? sanv_back_int(&state->indentStack) : 0;
}

static int push_indent(parser_state_t *state) {
  int depth = 0;
  parser_state_t s1 = eat_wspace(state);
  if (head_is(&s1, SAN_TOKEN_INDENTATION)) {
    depth = strlen(s1.tokenPtr->raw);
    if (indent_depth(&s1) >= depth) {
      san_error_t err = _parseError(state, SAN_ERROR_BAD_INDENTATION);
      strcpy(err.msg, SAN_ERROR_BAD_INDENTATION_MSG);
      sanv_push(state->errors, &err);
      return 0;
    }
    sanv_push_int(&state->indentStack, depth);
  }
  return depth;
}

static inline void pop_indent(parser_state_t *state) {
  int dump;
  sanv_pop(&state->indentStack, &dump);
}

/*
 * Parser functions
 */

static int parse_terminal(parser_state_t const *state, parser_state_t *newState, int terminal) {
  *newState = eat_wspace(state);
  if (newState->tokenPtr->type == SAN_TOKEN_END) return SAN_NO_MATCH;
  if (head_is(newState, terminal)) {
    *newState = advance_state(newState);
    return SAN_MATCH;
  }
  return SAN_NO_MATCH;
}

static inline int parse_keyword(
  parser_state_t const *state,
  parser_state_t *newState,
  const char* keyword
) {
  *newState = eat_wspace(state);
  return strcmp(newState->tokenPtr->raw, keyword) == 0 &&
    parse_terminal(newState, newState, SAN_TOKEN_IDENTIFIER_OR_KEYWORD) == SAN_MATCH
    ? SAN_MATCH : SAN_NO_MATCH;
}

int parse_number_literal(parser_state_t const *state, parser_state_t *newState) {
  san_dbg("Parsing number literal\n");
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
  san_dbg("Parsing primary expression\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_PRIMARY_EXPRESSION);
  parser_state_t s1;

  if (parse_number_literal(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);
    *newState = s1;
    return SAN_MATCH;
  } else if (parse_terminal(newState, &s1, SAN_TOKEN_IDENTIFIER_OR_KEYWORD) != SAN_NO_MATCH) {
    *newState = s1;
    return SAN_MATCH;
  }
  return SAN_NO_MATCH;
}

int parse_mult_exp(parser_state_t const *state, parser_state_t *newState) {
  san_dbg("Parsing mult exp\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_MULTIPLICATIVE_EXPRESSION);

  parser_state_t s1, s2;
  if (parse_primary_exp(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);
    *newState = s1;

    s2 = clone_state(&s1);
    if (parse_terminal(&s1, &s2, SAN_TOKEN_TIMES) != SAN_NO_MATCH) {
      if (parse_primary_exp(&s2, &s2) != SAN_NO_MATCH) {
        add_child(&s2, nodeIndex);
        *newState = s2;
      } else {
        parseError0(newState, &s2, SAN_ERROR_EXPECTED_FACTOR);
        return SAN_ERR_MATCH;
      }
    }
    return SAN_MATCH;
  }

  return SAN_NO_MATCH;
}

int parse_additive_exp(parser_state_t const *state, parser_state_t *newState) {
  san_dbg("Parsing additive exp\n");
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
        parseError0(newState, &s2, SAN_ERROR_EXPECTED_TERM);
        return SAN_ERR_MATCH;
      }
    }
    return SAN_MATCH;
  }

  return SAN_NO_MATCH;
}

int parse_func_param(parser_state_t *state, parser_state_t *newState) {
  san_dbg("Parsing function parameter\n");
  *newState = clone_state(state);
  push_node(newState, SAN_PARSER_FUNCTION_PARAMETER);

  if (parse_terminal(newState, newState, SAN_TOKEN_IDENTIFIER_OR_KEYWORD) != SAN_NO_MATCH) {
    return SAN_MATCH;
  }

  return SAN_NO_MATCH;
}

int parse_func_param_list(parser_state_t *state, parser_state_t *newState) {
  san_dbg("Parsing function parameters\n");
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

int parse_var_lvalue(parser_state_t *state, parser_state_t *newState) {
  san_dbg("Parsing variable L-value\n");
  *newState = clone_state(state);
  push_node(newState, SAN_PARSER_VARIABLE_LVALUE);
  int result = SAN_NO_MATCH;

  if (parse_terminal(newState, newState, SAN_TOKEN_IDENTIFIER_OR_KEYWORD) != SAN_NO_MATCH) {
    result = SAN_MATCH;
  }

  return result;
}

static int parse_func_lvalue(parser_state_t *state, parser_state_t *newState) {
  san_dbg("Parsing function L-value\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_FUNCTION_LVALUE);
  parser_state_t s1;
  int result = SAN_NO_MATCH;
  int oldIndentSenst = newState->indentSensitive;

  newState->indentSensitive = 0;

  if (parse_terminal(newState, newState, SAN_TOKEN_IDENTIFIER_OR_KEYWORD) != SAN_NO_MATCH) {
    if (parse_func_param_list(newState, &s1) != SAN_NO_MATCH) {
      add_child(&s1, nodeIndex);
      *newState = s1;
      result = SAN_MATCH;
    }
  }

  newState->indentSensitive = oldIndentSenst;
  return result;
}

static int parse_block(parser_state_t const *state, parser_state_t *newState) {
  san_dbg("Parsing block body\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_BLOCK);
  parser_state_t s1;
  int oldIndentSenst = newState->indentSensitive;
  int result = SAN_NO_MATCH;

  newState->indentSensitive = 1;

  if (parse_exp(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);
    *newState = s1;
    result = SAN_MATCH;
    goto out;
  } else {
    int foundBlock = 0;
    int depth = push_indent(newState);
    for (*newState = eat_wspace(newState);
        head_len(newState) == depth &&
        parse_terminal(newState, newState, SAN_TOKEN_INDENTATION) != SAN_NO_MATCH;
        *newState = eat_wspace(newState)) {
      foundBlock = 1;
      if (parse_exp(newState, newState) != SAN_NO_MATCH) {
        add_child(newState, nodeIndex);
      }
    }
    pop_indent(newState);
    if (foundBlock) {
      result = SAN_MATCH;
      goto out;
    }
  }

out:
  newState->indentSensitive = oldIndentSenst;
  return result;
}

int parse_variable_exp(parser_state_t *state, parser_state_t *newState) {
  san_dbg("Parsing variable expression\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_VARIABLE_EXPRESSION);
  parser_state_t s1, s2, s3, s4;
  if (parse_keyword(newState, &s1, SAN_KEYWORD_LET) != SAN_NO_MATCH) {
    if (parse_func_lvalue(&s1, &s2) != SAN_NO_MATCH) {
      add_child(&s2, nodeIndex);

      if (parse_terminal(&s2, &s3, SAN_TOKEN_EQUALS) != SAN_NO_MATCH) {
        if (parse_block(&s3, &s4) != SAN_NO_MATCH) {
          add_child(&s4, nodeIndex);
          *newState = s4;
          return SAN_MATCH;
        } else {
          parseError0(newState, &s3, SAN_ERROR_EXPECTED_BLOCK);
          return SAN_ERR_MATCH;
        }
      } else {
        return SAN_NO_MATCH;
      }
    } else if (parse_var_lvalue(&s1, &s2) != SAN_NO_MATCH) {
      add_child(&s2, nodeIndex);

      if (parse_terminal(&s2, &s3, SAN_TOKEN_EQUALS) != SAN_NO_MATCH) {
        int oldIndentSenst = s3.indentSensitive;
        s3.indentSensitive = 0;
        if (parse_exp(&s3, &s4) != SAN_NO_MATCH) {
          add_child(&s4, nodeIndex);
          s4.indentSensitive = oldIndentSenst;
          *newState = s4;
          return SAN_MATCH;
        } else {
          parseError0(newState, &s3, SAN_ERROR_EXPECTED_EXPRESSION);
          return SAN_ERR_MATCH;
        }
      } else {
        return SAN_NO_MATCH;
      }
    } else {
      parseError0(state, &s1, SAN_ERROR_EXPECTED_LVALUE);
      return SAN_ERR_MATCH;
    }
  }
  return SAN_NO_MATCH;
}

/*
 * if_exp = 'if' exp 'then' exp
 *        | 'if' exp block
 *        ;
 */
int parse_if_exp(parser_state_t *state, parser_state_t *newState) {
  san_dbg("Parsing if expression\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_IF_EXPRESSION);
  parser_state_t s1, s2, s3, s4;
  if (parse_keyword(newState, &s1, SAN_KEYWORD_IF) != SAN_NO_MATCH) {
    if (parse_exp(&s1, &s2) != SAN_NO_MATCH) {
      add_child(&s2, nodeIndex);
      if (parse_keyword(&s2, &s3, SAN_KEYWORD_THEN) != SAN_NO_MATCH) {
        if (parse_exp(&s3, &s4) != SAN_NO_MATCH) {
          add_child(&s4, nodeIndex);
          *newState = s4;
          return SAN_MATCH;
        }
      } else if(parse_block(&s2, &s3) != SAN_NO_MATCH) {
        add_child(&s3, nodeIndex);
        *newState = s3;
        return SAN_MATCH;
      } else {
        parseError0(state, &s2, SAN_ERROR_EXPECTED_BLOCK);
        return SAN_ERR_MATCH;
      }
    } else {
      parseError0(state, &s1, SAN_ERROR_EXPECTED_EXPRESSION);
      return SAN_ERR_MATCH;
    }
  }
  return SAN_NO_MATCH;
}

int parse_paren_list(parser_state_t *state, parser_state_t *newState) {
  san_dbg("Parsing paren list\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_LIST);
  parser_state_t s1, s2, s3;

  if (parse_terminal(newState, &s1, SAN_TOKEN_LPAREN) != SAN_NO_MATCH) {
    if (parse_exp(&s1, &s2) != SAN_NO_MATCH) {
      add_child(&s2, nodeIndex);
      if (parse_terminal(&s2, &s3, SAN_TOKEN_RPAREN) != SAN_NO_MATCH) {
        *newState = s3;
        return SAN_MATCH;
      }
    }
  }

  return SAN_NO_MATCH;
}

int parse_list(parser_state_t *state, parser_state_t *newState) {
  san_dbg("Parsing list\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_LIST);
  int result = SAN_NO_MATCH;
  parser_state_t s1, s2, s3;

  if (parse_terminal(newState, &s1, SAN_TOKEN_LPAREN) != SAN_NO_MATCH) {
    if (parse_exp(&s1, &s2) != SAN_NO_MATCH) {
      add_child(&s2, nodeIndex);
      if (parse_terminal(&s2, &s3, SAN_TOKEN_RPAREN) != SAN_NO_MATCH) {
        *newState = s3;
        return SAN_MATCH;
      }
    }
  } else {
    while ((parse_additive_exp(newState, &s1) != SAN_NO_MATCH) ||
           (parse_paren_list(newState, &s1) != SAN_NO_MATCH)) {
      add_child(&s1, nodeIndex);
      *newState = s1;
      result = SAN_MATCH;
    }
  }

  return result;
}

int parse_fn_exp(parser_state_t *state, parser_state_t *newState) {
  san_dbg("Parsing function expression\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_FN_EXPRESSION);
  parser_state_t s1, s2;

  if (parse_var_lvalue(newState, &s1) != SAN_NO_MATCH) {
    add_child(&s1, nodeIndex);

    if (parse_list(&s1, &s2) != SAN_NO_MATCH) {
        add_child(&s2, nodeIndex);
        *newState = s2;
        return SAN_MATCH;
    }
  }

  return SAN_NO_MATCH;
}

int parse_pipe_exp(parser_state_t *state, parser_state_t *newState) {
  san_dbg("Parsing pipe expression\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_PIPE_EXPRESSION);
  parser_state_t s1, s2, s3;

  if ((parse_fn_exp(newState, &s1) != SAN_NO_MATCH) ||
      (parse_list(newState, &s1) != SAN_NO_MATCH) ||
      (parse_additive_exp(newState, &s1) != SAN_NO_MATCH) ||
      (parse_exp(newState, &s1) != SAN_NO_MATCH)) {
    add_child(&s1, nodeIndex);

    while (parse_terminal(&s1, &s2, SAN_TOKEN_PIPE) != SAN_NO_MATCH) {
      if ((parse_fn_exp(&s2, &s3) != SAN_NO_MATCH) ||
          (parse_additive_exp(&s2, &s3) != SAN_NO_MATCH)) {
        add_child(&s3, nodeIndex);
        *newState = s3;
        return SAN_MATCH;
      }
    }
  }

  return SAN_NO_MATCH;
}

int parse_exp(parser_state_t const *state, parser_state_t *newState) {
  san_dbg("Parsing expression\n");
  *newState = clone_state(state);
  int nodeIndex = push_node(newState, SAN_PARSER_EXPRESSION);
  parser_state_t s1;

  if (at_end(state)) return SAN_NO_MATCH;

  if ((parse_variable_exp(newState, &s1) != SAN_NO_MATCH) ||
      (parse_pipe_exp(newState, &s1) != SAN_NO_MATCH) ||
      (parse_if_exp(newState, &s1) != SAN_NO_MATCH) ||
      (parse_fn_exp(newState, &s1) != SAN_NO_MATCH) ||
      (parse_list(newState, &s1) != SAN_NO_MATCH) ||
      (parse_additive_exp(newState, &s1) != SAN_NO_MATCH)
      ) {
    add_child(&s1, nodeIndex);
    *newState = s1;
    return SAN_MATCH;
  }

  return SAN_NO_MATCH;
}

void indent(int n) {
  for(int i = 0; i < n; ++i) san_dbg("| ");
}

void dump_ast(san_node_t *ast, int ind) {
  indent(ind);
  int type = ast != NULL ? ast->type : -1;
  san_dbg("[node type: %s, ptr: '%s', nchildren: %d]\n", fmt(type), ast->token->raw, ast->children.size);
  fflush(stdout);
  SAN_VECTOR_FOR_EACH(ast->children, i, san_node_t, node)
    dump_ast(node, ind+1);
  SAN_VECTOR_END_FOR_EACH
}

int sanp_parse(san_vector_t const *tokens, san_node_t *ast, san_vector_t *errors) {
  parser_state_t state;

  ast->type = SAN_PARSER_ROOT;

  if (tokens == NULL || tokens->size == 0) {
    return SAN_FAIL;
  }

  state = create_state();
  state.tokens = tokens;
  state.errors = errors;
  state.tokenPtr = tokens->elems;
  state.indentSensitive = 1;

  int firstIndex = push_node(&state, SAN_PARSER_ROOT);
  parser_state_t s1;
  if (parse_exp(&state, &s1) != SAN_NO_MATCH) {
    add_child(&s1, firstIndex);
    state = s1;
  } else {}
  *ast = *(san_node_t*)sanv_nth(&state.nodeStack, firstIndex);

  dump_ast(ast, 0);

  return SAN_OK;
}

#include "san.h"
#include "parser.h"

typedef struct {
  san_vector_t const *tokens;
  san_token_t const *tokenPtr;
  san_vector_t nodeStack;
} parser_state_t;

typedef int (*parser_t)(parser_state_t *state);

#define SAN_MATCH       1
#define SAN_NO_MATCH    2

int parse_exp(parser_state_t *state);
int parse_additive_exp(parser_state_t *state);

/*
 * Parser helper functions
 */
static inline int head_is(parser_state_t const *state, int tokenType) {
  return state->tokenPtr->type == tokenType;
}

static inline int head_is_not(parser_state_t const *state, int tokenType) {
  return !head_is(state, tokenType);
}

static inline parser_state_t advance_state(parser_state_t *state) {
  parser_state_t newState = *state;
  newState.tokenPtr = state->tokenPtr + 1;
  return newState;
}

int pushNode(parser_state_t *state, int type) {
  san_node_t node;
  node.type = type;
  node.token = state->tokenPtr;
  sanv_create(&node.children, sizeof(san_node_t));
  sanv_push(&state->nodeStack, &node);
  return state->nodeStack.size - 1;
}

int kleene(parser_state_t *state, parser_t parser) {
  int match = SAN_NO_MATCH;
  while (head_is_not(state, SAN_TOKEN_END) &&
    parser(state) == SAN_MATCH) { match = SAN_MATCH; }
  return match;
}

int or(parser_state_t *state, parser_t first, parser_t second) {
  return first(state) == SAN_MATCH ? SAN_MATCH :
         second(state) == SAN_MATCH ? SAN_MATCH : SAN_NO_MATCH;
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

/*
 * Parser functions
 */
int parse_number_literal(parser_state_t *state) {
  printf("Parsing number literal\n");
        fflush(stdout);
  pushNode(state, SAN_PARSER_NUMBER_LITERAL);
  if (head_is(state, SAN_TOKEN_IDENTIFIER)) {
    return SAN_MATCH;
  } else if (head_is(state, SAN_TOKEN_NUMBER)) {
    return SAN_MATCH;
  }
  return SAN_NO_MATCH;
}

int match_terminal(parser_state_t *state, int terminal) {
  return head_is(state, terminal) ? SAN_MATCH : SAN_NO_MATCH;
}

int parse_mult_exp(parser_state_t *state) {
  printf("Parsing mult exp\n");
  fflush(stdout);
  int nodeIndex = pushNode(state, SAN_PARSER_MULTIPLICATIVE_EXPRESSION);

  if (parse_number_literal(state) == SAN_MATCH) {
    add_child(state, nodeIndex);
    parser_state_t s1 = advance_state(state);

    if (match_terminal(&s1, SAN_TOKEN_TIMES) == SAN_MATCH) {
      parser_state_t s2 = advance_state(&s1);
      
      if (parse_number_literal(&s2) == SAN_MATCH) {
        *state = s2;
        add_child(state, nodeIndex);
        return SAN_MATCH;
      } else {
        // Expected expression
      }
    } else {
      return SAN_MATCH;
    }
  } else {
    return SAN_NO_MATCH;
  }

  return head_is(state, SAN_TOKEN_TIMES) ? SAN_MATCH : SAN_NO_MATCH;
}

int parse_additive_exp(parser_state_t *state) {
  printf("Parsing additive exp\n");
  fflush(stdout);
  int nodeIndex = pushNode(state, SAN_PARSER_ADDITIVE_EXPRESSION);

  if (parse_mult_exp(state) == SAN_MATCH) {
    add_child(state, nodeIndex);
    parser_state_t s1 = advance_state(state);

    if (match_terminal(&s1, SAN_TOKEN_PLUS) == SAN_MATCH) {
      parser_state_t s2 = advance_state(&s1);
      
      if (parse_mult_exp(&s2) == SAN_MATCH) {
        *state = s2;
        add_child(state, nodeIndex);
        return SAN_MATCH;
      } else {
        // Expected expression
      }
    } else {
      return SAN_MATCH;
    }
  } else {
    return SAN_NO_MATCH;
  }

  /* This should never happen */
  return SAN_FAIL;
}

int parse_exp(parser_state_t *state) {
  printf("Parsing expression\n");
        //printf("%d\n", state->nodeStack.size);
        fflush(stdout);
  int nodeIndex = pushNode(state, SAN_PARSER_EXPRESSION);
  if (parse_additive_exp(state) == SAN_MATCH) {
    add_child(state, nodeIndex);
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
  //char * raw = ast != NULL && ast->token != NULL && ast->token->type != SAN_TOKEN_END ? ast->token->raw : "";
  printf("[node type: %s, ptr: '', nchildren: %d]\n", fmt(type), ast->children.size);
  fflush(stdout);
  SAN_VECTOR_FOR_EACH(ast->children, i, san_node_t, node)
    dump_ast(node, ind+2);
  SAN_VECTOR_END_FOR_EACH
}

int parseTokens(san_vector_t const *tokens, san_node_t *ast) {
  parser_state_t state;

  ast->type = SAN_PARSER_ROOT;

  if (tokens == NULL || tokens->size == 0) {
    return SAN_FAIL;
  }

  state.tokens = tokens;
  state.tokenPtr = tokens->elems;
  sanv_create(&state.nodeStack, sizeof(san_node_t));

  int firstIndex = pushNode(&state, SAN_PARSER_ROOT);
  parse_exp(&state);
  add_child(&state, firstIndex);
  *ast = *(san_node_t*)sanv_nth(&state.nodeStack, firstIndex);

  dump_ast(ast, 0);

  return SAN_OK;
}

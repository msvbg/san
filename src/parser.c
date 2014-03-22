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

int parseExpression(parser_state_t *state);
int parseTerm(parser_state_t *state);

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

void pushNode(parser_state_t *state, int type) {
  san_ast_node_t node;
  node.type = type;
  node.token = state->tokenPtr;
  sanv_create(&node.children, sizeof(san_ast_node_t*));
  sanv_push(&state->nodeStack, &node);
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

/*
 * Parser functions
 */
int parseFactor(parser_state_t *state) {
  printf("Parsing factor\n");
        fflush(stdout);
  if (head_is(state, SAN_TOKEN_IDENTIFIER)) {
    return SAN_MATCH;
  } else if (head_is(state, SAN_TOKEN_NUMBER)) {
    return SAN_MATCH;
  }
  return SAN_NO_MATCH;
}

int parsePlus(parser_state_t *state) {
  printf("Parsing plus: %d\n", head_is(state, SAN_TOKEN_PLUS));
        fflush(stdout);
  return head_is(state, SAN_TOKEN_PLUS) ? SAN_MATCH : SAN_NO_MATCH;
}

int parseTimes(parser_state_t *state) {
  printf("Parsing times\n");
        fflush(stdout);
  return head_is(state, SAN_TOKEN_TIMES) ? SAN_MATCH : SAN_NO_MATCH;
}

int parseTerm(parser_state_t *state) {
  printf("Parsing term\n");
        fflush(stdout);

  if (parseFactor(state) == SAN_MATCH) {
    pushNode(state, SAN_PARSER_FACTOR);
    parser_state_t s1 = advance_state(state);

    if (parseTimes(&s1) == SAN_MATCH) {
      pushNode(&s1, SAN_PARSER_MULTIPLICATION_EXPRESSION);
      parser_state_t s2 = advance_state(&s1);
      
      if (parseExpression(&s2) == SAN_MATCH) {
        pushNode(&s2, SAN_PARSER_EXPRESSION);

        *state = s2;
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

int parseExpression(parser_state_t *state) {
  printf("Parsing expression\n");
        //printf("%d\n", state->nodeStack.size);
        fflush(stdout);
        
  if (parseTerm(state) == SAN_MATCH) {
    pushNode(state, SAN_PARSER_TERM);
    parser_state_t s1 = advance_state(state);

    if (parsePlus(&s1) == SAN_MATCH) {
      pushNode(&s1, SAN_PARSER_ADDITION_EXPRESSION);
      parser_state_t s2 = advance_state(&s1);

      if (parseExpression(&s2) == SAN_MATCH) {
        pushNode(&s2, SAN_PARSER_EXPRESSION);
        printf("KLK\n");

        *state = s2;
        return SAN_MATCH;
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

int parseTokens(san_vector_t const *tokens, san_ast_t **ast) {
  parser_state_t state;

  if (tokens == NULL || tokens->size == 0) {
    *ast = NULL;
    return SAN_OK;
  }

  state.tokens = tokens;
  state.tokenPtr = tokens->elems;
  sanv_create(&state.nodeStack, sizeof(san_ast_node_t));

  parseExpression(&state);

  SAN_VECTOR_FOR_EACH(state.nodeStack, i, san_ast_node_t, node)
    printf("%d: %d\n", i, node->type);
  SAN_VECTOR_END_FOR_EACH

  return SAN_OK;
}

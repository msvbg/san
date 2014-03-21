#include "san.h"
#include "parser.h"

typedef struct {
  san_token_t const *tokenPtr;

  san_ast_node_t *nodeStack;
  int nodeStackSize, nodeStackCapacity;
} san_parser_state_t;

typedef int (*parser_t)(san_parser_state_t *state);

#define SAN_MATCH       1
#define SAN_NO_MATCH    2

int parseExpression(san_parser_state_t *state);
int parseTerm(san_parser_state_t *state);

/*
 * Parser helper functions
 */
int headIs(san_parser_state_t const *state, int tokenType) {
  return state->tokenPtr->type == tokenType;
}

int headIsNot(san_parser_state_t const *state, int tokenType) {
  return state->tokenPtr->type != tokenType;
}

san_parser_state_t cloneState(san_parser_state_t *state) {
  return *state;
}

san_parser_state_t advanceState(san_parser_state_t *state) {
  san_parser_state_t newState = *state;
  newState.tokenPtr = state->tokenPtr + 1;
  return newState;
}

void pushNode(san_parser_state_t *state, int type) {
  san_ast_node_t *topNode;

  /* Resize node stack if necessary */
  if (state->nodeStackSize + 1 > state->nodeStackCapacity) {
    san_ast_node_t *nodes = calloc(state->nodeStackCapacity * 2, sizeof(san_ast_node_t));
    memcpy(nodes, state->nodeStack, state->nodeStackSize);
    free(state->nodeStack);
    state->nodeStack = nodes;
  }

  topNode = state->nodeStack + state->nodeStackSize;
  topNode = malloc(sizeof(san_ast_node_t));
  topNode->type = type;
  topNode->token = state->tokenPtr;
  topNode->childrenSize = 0;
  topNode->childrenCapacity = 2;
  topNode->children = calloc(2, sizeof(san_ast_node_t*));
}

int kleene(san_parser_state_t *state, parser_t parser) {
  int match = SAN_NO_MATCH;
  while (headIsNot(state, SAN_TOKEN_END) &&
    parser(state) == SAN_MATCH) { match = SAN_MATCH; }
  return match;
}

int or(san_parser_state_t *state, parser_t first, parser_t second) {
  return first(state) == SAN_MATCH ? SAN_MATCH :
         second(state) == SAN_MATCH ? SAN_MATCH : SAN_NO_MATCH;
}

/*
 * Parser functions
 */
int parseFactor(san_parser_state_t *state) {
  while (headIsNot(state, SAN_TOKEN_END)) {
    if (headIs(state, SAN_TOKEN_IDENTIFIER)) {
      return SAN_OK;
    } else if (headIs(state, SAN_TOKEN_NUMBER)) {
      return SAN_OK;
    }
  }
  return SAN_OK;
}

int parsePlus(san_parser_state_t *state) {
  return headIs(state, SAN_TOKEN_PLUS);
}

int parseTimes(san_parser_state_t *state) {
  return headIs(state, SAN_TOKEN_TIMES);
}

int parseTerm(san_parser_state_t *state) {
  san_parser_state_t nextState = *state;

  if (parseFactor(&nextState) == SAN_MATCH) {
    nextState = advanceState(&nextState);
    pushNode(state, SAN_PARSER_FACTOR);

    if (parseTimes(&nextState) == SAN_MATCH) {
      nextState = advanceState(&nextState);
      pushNode(state, SAN_PARSER_MULTIPLICATION_EXPRESSION);
      
      if (parseExpression(&nextState) == SAN_MATCH) {
        pushNode(state, SAN_PARSER_EXPRESSION);

        *state = advanceState(&nextState);
        return SAN_MATCH;
      }
    } else {
      *state = advanceState(&nextState);
      return SAN_MATCH;
    }
  } else {
    return SAN_NO_MATCH;
  }

  /* This should never happen */
  return SAN_FAIL;
}

int parseExpression(san_parser_state_t *state) {
  san_parser_state_t nextState = cloneState(state);

  if (parseTerm(&nextState) == SAN_MATCH) {
    nextState = advanceState(&nextState);
    pushNode(state, SAN_PARSER_TERM);

    if (parsePlus(&nextState) == SAN_MATCH) {
      nextState = advanceState(&nextState);
      pushNode(state, SAN_PARSER_ADDITION_EXPRESSION);

      if (parseExpression(&nextState) == SAN_MATCH) {
        pushNode(state, SAN_PARSER_EXPRESSION);
        
        *state = advanceState(&nextState);
        return SAN_MATCH;
      }
    } else {
      *state = advanceState(&nextState);
      return SAN_MATCH;
    }
  } else {
    return SAN_NO_MATCH;
  }

  /* This should never happen */
  return SAN_FAIL;
}

int parseTokens(san_token_t const *tokens, san_ast_t **ast) {
  san_parser_state_t *state;

  if (tokens == NULL) {
    *ast = NULL;
    return SAN_OK;
  }

  state = malloc(sizeof(san_parser_state_t));
  if (state == NULL) return SAN_FAIL;

  state->tokenPtr = tokens;
  state->nodeStackCapacity = 10;
  state->nodeStackSize = 0;
  state->nodeStack = malloc(sizeof(san_ast_node_t) * state->nodeStackCapacity);

  parseExpression(state);

  /* free nodeStack */

  return SAN_OK;
}

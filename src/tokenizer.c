#include "san.h"

/*
 * Only intended to be used from within the tokenError macro.
 */
void __addTokenError(san_tokenizer_state_t *state, int code) {
  san_error_list_t *errList = state->errorList;
  if (errList->nErrors + 1 > errList->errorCapacity) {
    errList->errors = realloc(errList->errors, sizeof(san_error_t) * errList->errorCapacity*2);
    errList->errorCapacity *= 2;
  }

  san_error_t *err = errList->errors + errList->nErrors;
  err->code = code;
  err->line = state->line;
  err->column = state->column;
}

#define tokenError(state, code, ...) do { \
  __addTokenError(state, code); \
  sprintf(state->errorList->errors[state->errorList->nErrors].msg, code##_MSG, __VA_ARGS__); \
  ++state->errorList->nErrors; \
} while (0)

/*
 * Tokenizer state
 */
int createTokenizerState(san_tokenizer_state_t **state) {
  san_error_list_t *errList;
  *state = calloc(1, sizeof(san_tokenizer_state_t));
  if (state == NULL) return SAN_FAIL;

  (*state)->line = 1;
  (*state)->column = 1;

  errList = (*state)->errorList = malloc(sizeof(san_error_list_t));
  if (errList == NULL) return SAN_FAIL;

  errList->nErrors = 0;
  errList->errorCapacity = 5;
  errList->errors = calloc(errList->errorCapacity, sizeof(san_error_t));
  if (errList->errors == NULL) return SAN_FAIL;

  return SAN_OK;
}

void destroyTokenizerState(san_tokenizer_state_t *state) {
  free(state);
}

void advance(san_tokenizer_state_t *state) {
  if (*(state->inputPtr) == '\n') {
    ++state->line;
    state->column = 1;
  } else {
    ++state->column;
  }
  ++state->inputPtr;
}

/*
 * Character matching functions
 */
int isAlphabetic(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isAlphanumeric(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') \
    || (c >= '0' && c <= '9');
}

int isKeyword(const char *word) {
  return strcmp(word, "if") == 0;
}

int isDigit(char c) {
  return c >= '0' && c <= '9';
}

int isWhiteSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int classifyToken(const char *input) {
  const char firstChar = input[0];
  if (isAlphabetic(firstChar)) return SAN_TOKEN_IDENTIFIER_OR_KEYWORD;
  if (isWhiteSpace(firstChar)) return SAN_TOKEN_WHITE_SPACE;
  if (isDigit(firstChar)) return SAN_TOKEN_NUMBER;
  if (firstChar == '=') return SAN_TOKEN_EQUALS;
  return SAN_INVALID_TOKEN; 
}

/*
 * Token construction/destruction
 */
int resetToken(san_token_t *token) {
  token->type = SAN_NO_TOKEN;

  if (token->raw != NULL)
    free(token->raw);
  
  token->raw = malloc(sizeof(char) * 32);
  token->rawSize = 32;
  if (token->raw == NULL) return SAN_FAIL;
  return SAN_OK;
}

int createTokens(san_token_t **out, unsigned int n) {
  int i;
  san_token_t *tokens = calloc(n, sizeof(san_token_t));

  if (tokens == NULL) return SAN_FAIL;

  for (i = 0; i < n; ++i) {
    resetToken(tokens + i);
  }

  *out = tokens;

  return SAN_OK;
}

int createToken(san_token_t **out) {
  return createTokens(out, 1);
}

int destroyTokens(san_token_t *tokens, unsigned int n) {
  int i;
  for (i = 0; i < n; ++i) {
    free(tokens[i].raw);
  }
  free(tokens);
  return SAN_OK;
}

int destroyToken(san_token_t *token) {
  return destroyTokens(token, 1);
}

int acceptChar(char chr, san_token_t *token) {
  int len = strlen(token->raw);
  
  /* Size of old raw block is too small, so double it */
  if (len + 1 >= token->rawSize) {
    token->raw = realloc(token->raw, token->rawSize * 2); 
    token->rawSize = token->rawSize * 2;
    if (token->raw == NULL) return SAN_FAIL;
  } else {
    token->raw[len] = chr;
    token->raw[len+1] = '\0';
  }
  return SAN_OK;
}

int appendToken(san_token_t **tokens, unsigned int *nTokens, unsigned int *tokenCapacity) {
  int i;

  if (*nTokens + 1 > *tokenCapacity) {
    san_token_t *newTokens = calloc(*tokenCapacity * 2, sizeof(san_token_t));
    memcpy(newTokens, tokens, *tokenCapacity);
    *tokens = newTokens;

    for (i = *tokenCapacity; i < *tokenCapacity * 2; ++i) {
      if (resetToken((*tokens) + i) == SAN_FAIL) return SAN_FAIL;
    }
    *tokenCapacity *= 2;
  }
  *nTokens += 1;
  return SAN_OK;
}

int readIdentifierOrKeyword(san_tokenizer_state_t *state) {
  while (*state->inputPtr != '\0') {
    if (isAlphanumeric(*state->inputPtr)) {
      if (acceptChar(*state->inputPtr, state->outputPtr) == SAN_FAIL) return SAN_FAIL;
      advance(state);
    } else {
      break;
    }
  }

  if (isKeyword(state->outputPtr->raw)) state->outputPtr->type = SAN_TOKEN_KEYWORD;
  else state->outputPtr->type = SAN_TOKEN_IDENTIFIER;

  return SAN_OK;
}

int readWhiteSpace(san_tokenizer_state_t *state) {
  while (*state->inputPtr != '\0') {
    if (isWhiteSpace(*state->inputPtr)) {
      if (acceptChar(*state->inputPtr, state->outputPtr) == SAN_FAIL) return SAN_FAIL;
      advance(state);
    } else break;
  }
  return SAN_OK;
}

int readNumber(san_tokenizer_state_t *state) {
  while (*state->inputPtr != '\0')  {
    if (isDigit(*state->inputPtr)) {
      if (acceptChar(*state->inputPtr, state->outputPtr) == SAN_FAIL) return SAN_FAIL;
      advance(state);
    } else if(isAlphabetic(*state->inputPtr)) {
      tokenError(state, SAN_ERROR_ADJACENT_NUMBER_ALPHA,
        *state->inputPtr, state->outputPtr->raw);
      break;
    } else break;
  }
  return SAN_OK;
}

int readTokens(const char *input, san_token_t **output, san_error_list_t **errors) {
  unsigned int nTokens = 0, tokenCapacity = 1024;
  san_tokenizer_state_t *state;

  if (input == NULL || input[0] == '\0') return SAN_FAIL;
  
  createTokenizerState(&state);

  createTokens(output, tokenCapacity);
  if (*output == NULL) return SAN_FAIL;

  state->outputPtr = *output;
  state->inputPtr = input;

  while (*state->inputPtr != '\0') {
    if (state->outputPtr->type == SAN_NO_TOKEN) {
      state->outputPtr->type = classifyToken(state->inputPtr);
    }
    switch (state->outputPtr->type) {
      case SAN_TOKEN_IDENTIFIER_OR_KEYWORD:
        readIdentifierOrKeyword(state);
        break;
      case SAN_TOKEN_WHITE_SPACE:
        readWhiteSpace(state);
        break;
      case SAN_TOKEN_NUMBER:
        readNumber(state);
        break;
      case SAN_TOKEN_EQUALS:
        acceptChar(*state->inputPtr, state->outputPtr);
        advance(state);
        break;

      case SAN_INVALID_TOKEN:
        tokenError(state, SAN_ERROR_INVALID_CHARACTER, *state->inputPtr);
        resetToken(state->outputPtr);
        advance(state);
        continue;
    }

    if (appendToken(output, &nTokens, &tokenCapacity) == SAN_FAIL) {
      tokenError(state, SAN_ERROR_INTERNAL, NULL);
      break;
    }
    state->outputPtr = *output + nTokens;
  }
  
  state->outputPtr->type = SAN_TOKEN_END;
  if (state->errorList->nErrors > 0) *errors = state->errorList;
  else *errors = NULL;

  destroyTokenizerState(state);

  return SAN_OK;
}

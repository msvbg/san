#include "tokenizer.h"

typedef struct {
  const char *inputPtr;
  san_vector_t *output;
  int hasReadLineNonSpace;

  int line, column;
  san_vector_t *errorList;
} tokenizer_state_t;

static san_error_t _tokenError(tokenizer_state_t *state, int code) {
  san_error_t err;
  memset(&err, 0, sizeof err);
  err.code = code;
  err.line = state->line;
  err.column = state->column;
  return err;
}

#define tokenError(__state, __code, ...) do {                                  \
  san_error_t err = _tokenError(__state, __code);                              \
  sprintf(err.msg, __code##_MSG, __VA_ARGS__);                                 \
  sanv_push((__state)->errorList, &err);                                       \
} while(0)
#define tokenError0(__state, __code) do {                                      \
  san_error_t err = _tokenError(__state, __code);                              \
  sprintf(err.msg, __code##_MSG);                                              \
  sanv_push((__state)->errorList, &err);                                       \
} while(0)

/*
 * Character matching functions
 */
static inline int is_alphabetic(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline int is_alphanumeric(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') \
    || (c >= '0' && c <= '9');
}

static inline int is_digit(char c) {
  return c >= '0' && c <= '9';
}

static inline int is_white_space(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/*
 * Tokenizer state
 */
static int create_state(tokenizer_state_t **state, san_vector_t *errorList) {
  *state = calloc(1, sizeof(tokenizer_state_t));
  if (state == NULL) return SAN_FAIL;

  (*state)->line = 1;
  (*state)->column = 1;
  (*state)->errorList = errorList;
  (*state)->hasReadLineNonSpace = 0;

  return SAN_OK;
}

static void destroy_state(tokenizer_state_t *state) {
  free(state);
}

void advance(tokenizer_state_t *state) {
  if (*state->inputPtr != ' ')
    state->hasReadLineNonSpace = 1;

  if (*(state->inputPtr) == '\n') {
    ++state->line;
    state->column = 1;
    state->hasReadLineNonSpace = 0;
  } else {
    ++state->column;
  }

  ++state->inputPtr;
}

int classify_token(const char *input) {
  const char firstChar = input[0];
  if (is_alphabetic(firstChar)) return SAN_TOKEN_IDENTIFIER_OR_KEYWORD;
  if (is_white_space(firstChar)) return SAN_TOKEN_WHITE_SPACE;
  if (is_digit(firstChar)) return SAN_TOKEN_NUMBER;
  if (firstChar == '=') return SAN_TOKEN_EQUALS;
  if (firstChar == '*') return SAN_TOKEN_TIMES;
  if (firstChar == '+') return SAN_TOKEN_PLUS;
  return SAN_INVALID_TOKEN;
}

/*
 * Token construction/destruction
 */
int reset_token(san_token_t *token) {
  token->type = SAN_NO_TOKEN;

  if (token->raw != NULL)
    free(token->raw);

  token->raw = malloc(sizeof(char) * 32);
  token->rawSize = 32;
  if (token->raw == NULL) return SAN_FAIL;
  return SAN_OK;
}

/*
static int create_tokens(san_token_t **out, unsigned int n) {
  int i;
  san_token_t *tokens = calloc(n, sizeof(san_token_t));

  if (tokens == NULL) return SAN_FAIL;
  for (i = 0; i < n; ++i) reset_token(tokens + i);
  *out = tokens;
  return SAN_OK;
}
static inline int create_token(san_token_t **out) {
  return create_tokens(out, 1);
}
*/

int destroyTokens(san_token_t *tokens, int n) {
  int i;
  if (tokens != NULL) {
    for (i = 0; i < n; ++i) {
      san_token_t *this = &tokens[i];
      if (this != NULL) {
        free(this->raw);
        this = NULL;
      }
    }
    free(tokens);
    tokens = NULL;
  }
  return SAN_OK;
}

int sant_destructor(void *ptr) {
  san_token_t *token = (san_token_t*)ptr;
  if (token->raw != NULL && strcmp(token->raw, "") != 0) {
    free(token->raw);
  }
  ptr = NULL;
  return SAN_OK;
}

#define LAST_TOKEN(state) ((san_token_t*)sanv_back((state)->output))

int acceptChar(tokenizer_state_t *state) {
  san_token_t *token = LAST_TOKEN(state);
  int len = token->raw == NULL ? (unsigned)(-1) : strlen(token->raw);

  /* Size of old raw block is too small, so double it */
  if (len + 1 >= token->rawSize) {
    token->raw = realloc(token->raw, token->rawSize * 2);
    token->rawSize = token->rawSize * 2;
    if (token->raw == NULL) return SAN_FAIL;
  }

  token->raw[len] = *state->inputPtr;
  token->raw[len+1] = '\0';

  return SAN_OK;
}

int readIdentifierOrKeyword(tokenizer_state_t *state) {
  while (*state->inputPtr != '\0') {
    if (is_alphanumeric(*state->inputPtr)) {
      if (acceptChar(state) == SAN_FAIL) return SAN_FAIL;
      advance(state);
    } else {
      break;
    }
  }

  return SAN_OK;
}

int readIndentation(tokenizer_state_t *state) {
  for (int nSpaces = 1; *state->inputPtr == ' '; ++nSpaces) {
    if (acceptChar(state) == SAN_FAIL) return SAN_FAIL;
    advance(state);
  }
  if (*state->inputPtr == '\t')
    tokenError0(state, SAN_ERROR_TAB_AS_INDENTATION);
  state->hasReadLineNonSpace = 1;
  return SAN_OK;
}

int readWhiteSpace(tokenizer_state_t *state) {
  while (*state->inputPtr != '\0') {
    if (is_white_space(*state->inputPtr)) {
      if (acceptChar(state) == SAN_FAIL) return SAN_FAIL;
      if (*state->inputPtr == '\n') { advance(state); break; }
      advance(state);
    } else break;
  }
  return SAN_OK;
}

int readNumber(tokenizer_state_t *state) {
  while (*state->inputPtr != '\0')  {
    if (is_digit(*state->inputPtr)) {
      if (acceptChar(state) == SAN_FAIL) return SAN_FAIL;
      advance(state);
    } else if(is_alphabetic(*state->inputPtr)) {
      san_token_t *thisToken = (san_token_t*)sanv_back(state->output);
      tokenError(state, SAN_ERROR_ADJACENT_NUMBER_ALPHA,
        *state->inputPtr, thisToken->raw);
      break;
    } else break;
  }
  return SAN_OK;
}

int sant_tokenize(const char *input, san_vector_t *output, san_vector_t *errors) {
  tokenizer_state_t *state;

  if (input == NULL || input[0] == '\0') return SAN_FAIL;

  create_state(&state, errors);

  state->output = output;
  state->inputPtr = input;

  while (*state->inputPtr != '\0') {
    san_token_t *thisToken;
    san_token_t newToken;
    memset(&newToken, 0, sizeof(san_token_t));
    newToken.line = state->line;
    newToken.column = state->column;
    sanv_push(state->output, &newToken);

    thisToken = sanv_back(state->output);
    thisToken->raw = calloc(32, sizeof(char));
    thisToken->rawSize = 32 * sizeof(char);
    thisToken->type = classify_token(state->inputPtr);

    switch (thisToken->type) {
      case SAN_TOKEN_IDENTIFIER_OR_KEYWORD:
        readIdentifierOrKeyword(state);
        break;
      case SAN_TOKEN_WHITE_SPACE:
        if (state->hasReadLineNonSpace)
          readWhiteSpace(state);
        else {
          thisToken->type = SAN_TOKEN_INDENTATION;
          readIndentation(state);
        }
        break;
      case SAN_TOKEN_NUMBER:
        readNumber(state);
        break;

      case SAN_TOKEN_TIMES:
      case SAN_TOKEN_PLUS:
      case SAN_TOKEN_EQUALS:
        acceptChar(state);
        advance(state);
        break;

      default:
      case SAN_INVALID_TOKEN:
        tokenError(state, SAN_ERROR_INVALID_CHARACTER, *state->inputPtr);
        reset_token(thisToken);
        advance(state);
        continue;
    }
  }

  san_token_t endToken;
  endToken.type = SAN_TOKEN_END;
  endToken.raw = "";
  sanv_push(state->output, &endToken);

  destroy_state(state);

  return SAN_OK;
}

#include "san.h"

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

int isWhiteSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int classifyToken(const char *input) {
  const char firstChar = input[0];
  if (isAlphabetic(firstChar)) return SAN_TOKEN_IDENTIFIER_OR_KEYWORD;
  if (isWhiteSpace(firstChar)) return SAN_TOKEN_WHITE_SPACE;
  return SAN_INVALID_TOKEN; 
}

int resetToken(token_t *token) {
  token->type = SAN_NO_TOKEN;

  token->raw = malloc(sizeof(char) * 32);
  token->rawSize = 32;
  if (token->raw == NULL) return SAN_FAIL;
  return SAN_OK;
}

int createTokens(token_t **out, unsigned int n) {
  int i;
  token_t *tokens = malloc(n * sizeof(token_t));

  if (tokens == NULL) return SAN_FAIL;

  for (i = 0; i < n; ++i) {
    resetToken(tokens + i);
  }

  *out = tokens;

  return SAN_OK;
}

int createToken(token_t **out) {
  return createTokens(out, 1);
}

int destroyTokens(token_t *tokens, unsigned int n) {
  int i;
  for (i = 0; i < n; ++i) {
    free(tokens[i].raw);
  }
  free(tokens);
  return SAN_OK;
}

int acceptChar(char chr, token_t *token) {
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

int appendToken(token_t **tokens, unsigned int *nTokens, unsigned int *tokenCapacity) {
  int i;

  if (*nTokens + 1 > *tokenCapacity) {
    *tokens = realloc(*tokens, sizeof(token_t) * *tokenCapacity * 2);
    for (i = *tokenCapacity; i < *tokenCapacity * 2; ++i) {
      if (resetToken((*tokens) + i) == SAN_FAIL) return SAN_FAIL;
    }
    *tokenCapacity *= 2;
  }
  *nTokens += 1;
  return SAN_OK;
}

int readIdentifierOrKeyword(const char **input, token_t *output) {
  const char *inPtr;

  for (inPtr = *input; *inPtr != '\0'; ++inPtr) {
    if (isAlphanumeric(*inPtr)) {
      if (acceptChar(*inPtr, output) == SAN_FAIL) return SAN_FAIL;
    } else {
      break;
    }
  }
  *input = inPtr - 1;

  if (isKeyword(output->raw)) output->type = SAN_TOKEN_KEYWORD;
  else output->type = SAN_TOKEN_IDENTIFIER;

  return SAN_OK;
}

int readWhiteSpace(const char **input, token_t *output) {
  const char *inPtr;
  for (inPtr = *input; *inPtr != '\0'; ++inPtr) {
    if (isWhiteSpace(*inPtr)) {
      if (acceptChar(*inPtr, output) == SAN_FAIL) return SAN_FAIL;
    } else break;
  }
  *input = inPtr - 1;
  return SAN_OK;
}

int readTokens(const char *input, token_t **output) {
  const char *inputPtr;
  token_t *outputPtr;
  unsigned int nTokens = 0, tokenCapacity = 1024;

  if (input == NULL || input[0] == '\0') return SAN_FAIL;

  createTokens(output, tokenCapacity);
  if (*output == NULL) return SAN_FAIL;

  outputPtr = *output;

  for (inputPtr = input; *inputPtr != '\0'; ++inputPtr) {
    if (outputPtr->type == SAN_NO_TOKEN) {
      outputPtr->type = classifyToken(inputPtr);
    }
    switch (outputPtr->type) {
      case SAN_TOKEN_IDENTIFIER_OR_KEYWORD:
        readIdentifierOrKeyword(&inputPtr, outputPtr);
        break;
      case SAN_TOKEN_WHITE_SPACE:
        readWhiteSpace(&inputPtr, outputPtr);
        break;

      case SAN_INVALID_TOKEN:
        printf("INVALID");
        return SAN_FAIL;
      default:
        printf("DEFAULT");
        return SAN_FAIL;
    }

    if (appendToken(output, &nTokens, &tokenCapacity) == SAN_FAIL) return SAN_FAIL;
    outputPtr = *output + nTokens;
  }

  outputPtr->type = SAN_TOKEN_END;

  return SAN_OK;
}

#ifndef __SAN_H
#define __SAN_H

#define SAN_OK     0
#define SAN_FAIL  -1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  int code; 
  char msg[200];
  int line, column;
  const char *file;
} san_error_t;

typedef struct {
  san_error_t *errors;
  int nErrors, errorCapacity;
} san_error_list_t;

#define SAN_ERROR_INTERNAL                     1000
#define SAN_ERROR_INTERNAL_MSG \
  "An internal error occurred"

#define SAN_ERROR_ADJACENT_NUMBER_ALPHA        1001
#define SAN_ERROR_ADJACENT_NUMBER_ALPHA_MSG  \
  "An alphabetic character ('%c') cannot directly follow a number (%s) without a delimiter"

#define SAN_ERROR_INVALID_CHARACTER            1002
#define SAN_ERROR_INVALID_CHARACTER_MSG \
  "Encountered an invalid characteter ('%c')"


/* Token definitions */
typedef struct {
  int type;
  char *raw;
  size_t rawSize;
} san_token_t;

typedef struct {
  const char *inputPtr;
  san_token_t *outputPtr;

  int line, column;
  san_error_list_t *errorList;
} san_tokenizer_state_t;

#define SAN_NO_TOKEN                      0
#define SAN_INVALID_TOKEN                 1
#define SAN_TOKEN_END                     2
#define SAN_TOKEN_IDENTIFIER              3
#define SAN_TOKEN_IDENTIFIER_OR_KEYWORD   4
#define SAN_TOKEN_KEYWORD                 5
#define SAN_TOKEN_WHITE_SPACE             6
#define SAN_TOKEN_NUMBER                  7

int readTokens(const char *input, san_token_t **tokens, san_error_list_t **errors);

#endif

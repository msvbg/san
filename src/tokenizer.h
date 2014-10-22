#ifndef __SAN_TOKENIZER_H
#define __SAN_TOKENIZER_H

#include "vector.h"
#include "errors.h"

/* Token definitions */
typedef struct {
  int type;
  char *raw;
  size_t rawSize;

  int line, column;
} san_token_t;

#define SAN_NO_TOKEN                      0
#define SAN_INVALID_TOKEN                 1
#define SAN_TOKEN_END                     2
#define SAN_TOKEN_IDENTIFIER_OR_KEYWORD   3
#define SAN_TOKEN_WHITE_SPACE             4
#define SAN_TOKEN_NUMBER_LITERAL          5
#define SAN_TOKEN_EQUALS                  6
#define SAN_TOKEN_TIMES                   7
#define SAN_TOKEN_PLUS                    8
#define SAN_TOKEN_INDENTATION             9
#define SAN_TOKEN_COMMENT                10
#define SAN_TOKEN_PIPE                   11
#define SAN_TOKEN_LPAREN                 12
#define SAN_TOKEN_RPAREN                 13
#define SAN_TOKEN_STRING_LITERAL         14

int sant_tokenize(const char *input, san_vector_t *tokens, san_vector_t *errors);
int sant_destructor(void *ptr);

#endif

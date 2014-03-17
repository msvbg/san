#ifndef __SAN_H
#define __SAN_H

#define SAN_OK     0
#define SAN_FAIL  -1

#define SAN_TRUE 1
#define SAN_FALSE 0

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Token definitions */
typedef struct {
  int type;
  char *raw;
  size_t rawSize;
} token_t;

#define SAN_NO_TOKEN                      0
#define SAN_INVALID_TOKEN                 1
#define SAN_TOKEN_END                     2
#define SAN_TOKEN_IDENTIFIER              3
#define SAN_TOKEN_IDENTIFIER_OR_KEYWORD   4
#define SAN_TOKEN_KEYWORD                 5
#define SAN_TOKEN_WHITE_SPACE             6

int readTokens(
  const char *input,
  token_t **output);

#endif

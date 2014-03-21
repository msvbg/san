#ifndef __SAN_PARSER_H
#define __SAN_PARSER_H

#include "tokenizer.h"

/*
 * Parser
 */

#define SAN_PARSER_EXPRESSION                 1
#define SAN_PARSER_TERM                       2
#define SAN_PARSER_FACTOR                     3
#define SAN_PARSER_ADDITION_EXPRESSION        4
#define SAN_PARSER_MULTIPLICATION_EXPRESSION  5

typedef struct {
  int type;
  san_token_t const *token;
  struct san_ast_node_t **children;
  int childrenSize, childrenCapacity;
} san_ast_node_t;

typedef struct {
  san_ast_node_t *root;
} san_ast_t;

int parseTokens(san_token_t const* tokens, san_ast_t **ast);

#endif

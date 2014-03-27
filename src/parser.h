#ifndef __SAN_PARSER_H
#define __SAN_PARSER_H

#include "tokenizer.h"
#include "vector.h"

/*
 * Parser
 */

#define SAN_PARSER_ROOT                       1
#define SAN_PARSER_EXPRESSION                 2
#define SAN_PARSER_IDENTIFIER                 3
#define SAN_PARSER_NUMBER_LITERAL             4
#define SAN_PARSER_ADDITIVE_EXPRESSION        5
#define SAN_PARSER_MULTIPLICATIVE_EXPRESSION  6
#define SAN_PARSER_STATEMENT                  7
#define SAN_PARSER_VARIABLE_DEFINITION        8
#define SAN_PARSER_LVALUE                     9
#define SAN_PARSER_FUNCTION_PARAMETER_LIST    10
#define SAN_PARSER_FUNCTION_PARAMETER         11
#define SAN_PARSER_PRIMARY_EXPRESSION         12

typedef struct {
  int type;
  san_token_t const *token;
  san_vector_t children;
} san_node_t;

int parse(san_vector_t const* tokens, san_node_t *ast, san_vector_t *errors);

#endif

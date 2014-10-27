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
#define SAN_PARSER_VARIABLE_EXPRESSION        7
#define SAN_PARSER_VARIABLE_LVALUE            8
#define SAN_PARSER_FUNCTION_LVALUE            9
#define SAN_PARSER_FUNCTION_PARAMETER_LIST    10
#define SAN_PARSER_FUNCTION_PARAMETER         11
#define SAN_PARSER_BLOCK                      12
#define SAN_PARSER_PRIMARY_EXPRESSION         13
#define SAN_PARSER_IF_EXPRESSION              14
#define SAN_PARSER_FN_EXPRESSION              15
#define SAN_PARSER_LIST                       16
#define SAN_PARSER_PIPE_EXPRESSION            17
#define SAN_PARSER_STRING_LITERAL             18

#define SAN_KEYWORD_LET                       "let"
#define SAN_KEYWORD_IF                        "if"
#define SAN_KEYWORD_THEN                      "then"

typedef struct {
  int type;
  san_token_t const *token;
  san_vector_t children;
} san_node_t;

int sanp_parse(san_vector_t const* tokens, san_node_t *ast, san_vector_t *errors);
int sanp_destroy(san_node_t *ptr);

#endif

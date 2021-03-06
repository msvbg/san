#ifndef __SAN_BYTECODE
#define __SAN_BYTECODE

#include "parser.h"

#define SAN_BYTECODE_PUSH 1
#define SAN_BYTECODE_POP  2
#define SAN_BYTECODE_MUL  3
#define SAN_BYTECODE_ADD  4
#define SAN_BYTECODE_CALL 5

#define SAN_BYTECODE_TYPE_NUMBER_LITERAL                      1
#define SAN_BYTECODE_TYPE_IDENTIFIER                          2
#define SAN_BYTECODE_TYPE_STRING_LITERAL                      3

typedef struct {
  int type;
  int ref;
} san_arg_t;

typedef struct {
  int opcode;
  san_arg_t arg1, arg2;
} san_bytecode_t;

typedef struct {
  san_vector_t numbers;
  san_vector_t strings;
  san_vector_t symbols;
  san_vector_t bytecode;
} san_program_t;

int sanb_generate(const san_node_t *ast, san_program_t *program, san_vector_t *errors);
int sanb_destroy(san_program_t *program);

#endif

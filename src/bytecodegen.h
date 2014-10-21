#ifndef SAN_BYTECODEGEN
#define SAN_BYTECODEGEN

#include "parser.h"

typedef struct {
  int opcode;
} san_bytecode_t;

int sanb_generate(san_node_t *ast, san_vector_t *bytecode, san_vector_t *errors);

#endif

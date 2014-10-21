#include "bytecodegen.h"

typedef struct {
  san_node_t *node;
  san_vector_t *bytecode;
  san_vector_t *errors;
} bcgen_state_t;

static int block(bcgen_state_t *state) {

  return SAN_OK;
}

int sanb_generate(san_node_t *ast, san_vector_t *bytecode, san_vector_t *errors) {
  bcgen_state_t state = { ast, bytecode, errors };

  switch (ast->type) {
    case SAN_PARSER_ROOT: block(&state); break;
  }
  return SAN_OK;
}

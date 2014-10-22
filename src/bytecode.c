#include "bytecode.h"

typedef struct {
  const san_node_t *node;
  san_program_t *program;
  san_vector_t *errors;
} bcgen_state_t;

san_arg_t NO_ARG = { -1, -1 };

static int generate(bcgen_state_t *state);
/*
static bcgen_state_t clone_state(const bcgen_state_t *state) {
  bcgen_state_t clone = { state->node, state->bytecode, state->errors };
  return clone;
}
*/

static void emit0(bcgen_state_t *state, int opcode) {
  san_bytecode_t code = { opcode, NO_ARG, NO_ARG };
  sanv_push(&state->program->bytecode, &code);
}

static void emit1(bcgen_state_t *state, int opcode, san_arg_t *arg1) {
  san_bytecode_t code = { opcode, *arg1, NO_ARG };
  sanv_push(&state->program->bytecode, &code);
}
/*
static void emit2(bcgen_state_t *state, int opcode, void *arg1, void *arg2) {
  san_bytecode_t code = { opcode, arg1, arg2 };
  sanv_push(&state->program->bytecode, &code);
}
*/
static int store_number_literal(bcgen_state_t *state, int number, int *ref) {
  if (sanv_push(&state->program->numbers, &number) != SAN_OK) {
    return SAN_FAIL;
  }

  *ref = state->program->numbers.size - 1;

  return SAN_OK;
}

static void gen_children(bcgen_state_t *state) {
  const san_vector_t *children = &state->node->children;

  SAN_VECTOR_FOR_EACH(*children, i, san_node_t, child)
    bcgen_state_t childState = { child, state->program, state->errors };
    generate(&childState);
  SAN_VECTOR_END_FOR_EACH
}

static int generate(bcgen_state_t *state) {
  switch (state->node->type) {
    case SAN_PARSER_ROOT: gen_children(state); break;
    case SAN_PARSER_EXPRESSION:
      gen_children(state);
      break;
    case SAN_PARSER_ADDITIVE_EXPRESSION:
      gen_children(state);
      if (state->node->children.size == 1) {
        /* Pass through */
      } else if (state->node->children.size == 2) {
        emit0(state, SAN_BYTECODE_ADD);
      }
      break;
    case SAN_PARSER_MULTIPLICATIVE_EXPRESSION:
      gen_children(state);
      if (state->node->children.size == 1) {
        /* Pass through */
      } else if (state->node->children.size == 2) {
        emit0(state, SAN_BYTECODE_MUL);
      }
      break;
    case SAN_PARSER_PRIMARY_EXPRESSION:
      gen_children(state);
      break;
    case SAN_PARSER_NUMBER_LITERAL: {
      int number = strtol(state->node->token->raw, NULL, 10);
      san_arg_t arg = { SAN_BYTECODE_TYPE_NUMBER_LITERAL, 0 };
      store_number_literal(state, number, &arg.ref);
      emit1(state, SAN_BYTECODE_PUSH, &arg);
      break;
    }
    default:
      return SAN_FAIL;
  }

  return SAN_OK;
}

const char *fmt_opcode(int opcode) {
  switch (opcode) {
  case SAN_BYTECODE_PUSH: return "push";
  case SAN_BYTECODE_POP: return "pop";
  case SAN_BYTECODE_MUL: return "mul";
  case SAN_BYTECODE_ADD: return "add";
  }
  return "ERROR";
}

void dump_program(san_program_t *program) {
  san_dbg("Number literals:\n");
  SAN_VECTOR_FOR_EACH(program->numbers, i, int, number)
    printf("%d\n", *number);
  SAN_VECTOR_END_FOR_EACH

  san_dbg("\nOpcodes:\n");
  SAN_VECTOR_FOR_EACH(program->bytecode, i, san_bytecode_t, code)
    printf("%s (%d, %d)\n", fmt_opcode(code->opcode), code->arg1.ref, code->arg2.ref);
  SAN_VECTOR_END_FOR_EACH
}

int sanb_generate(const san_node_t *ast, san_program_t *program, san_vector_t *errors) {
  bcgen_state_t state = { ast, program, errors };

  sanv_create(&program->bytecode, sizeof(san_bytecode_t));
  sanv_create(&program->numbers, sizeof(int));
  generate(&state);
  //sanv_destroy(program->bytecode);

  dump_program(program);

  return SAN_OK;
}

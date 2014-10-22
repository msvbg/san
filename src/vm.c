#include "vm.h"

int sanm_run(const san_program_t *program) {
  san_dbg("\nRunning program:\n");

  san_vector_t stack;
  sanv_create(&stack, sizeof(int));
  SAN_VECTOR_FOR_EACH(program->bytecode, i, san_bytecode_t, code)
    switch (code->opcode) {

      case SAN_BYTECODE_PUSH: {
        switch (code->arg1.type) {
          case SAN_BYTECODE_TYPE_NUMBER_LITERAL: {
            int val = *(int*)sanv_nth(&program->numbers, code->arg1.ref);
            printf("PUSH %d\n", val);
            sanv_push_int(&stack, val);
            break;
          }
        }
        break;
      }

      case SAN_BYTECODE_MUL: {
        int arg1, arg2;
        sanv_pop(&stack, &arg1);
        sanv_pop(&stack, &arg2);
        printf("MUL %d, %d\n", arg1, arg2);
        sanv_push_int(&stack, arg1 * arg2);
        break;
      }

      case SAN_BYTECODE_ADD: {
        int arg1, arg2;
        sanv_pop(&stack, &arg1);
        sanv_pop(&stack, &arg2);
        printf("ADD %d, %d\n", arg1, arg2);
        sanv_push_int(&stack, arg1 + arg2);
        break;
      }

    }
  SAN_VECTOR_END_FOR_EACH

  int result;
  sanv_pop(&stack, &result);
  printf("RESULT: %d\n", result);

  return SAN_OK;
}

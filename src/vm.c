#include "vm.h"
#include "std.h"

#define SAN_VM_INT         1
#define SAN_VM_STRING      2
#define SAN_VM_SYMBOL      3

typedef struct {
  int type;
  union {
    int integer;
    const char *string;
  } value;
} vm_object;

static inline vm_object vm_int(const san_program_t *program, int ref) {
    int val = *(int*)sanv_nth(&program->numbers, ref);
    vm_object obj = { SAN_VM_INT, { .integer = val } };
    return obj;
}

static inline vm_object vm_string(const san_program_t *program, int ref) {
    const char *val = *(const char**)sanv_nth(&program->strings, ref);
    vm_object obj = { SAN_VM_STRING, { .string = val } };
    return obj;
}

static inline vm_object vm_symbol(const san_program_t *program, int ref) {
    const char *val = *(const char**)sanv_nth(&program->symbols, ref);
    vm_object obj = { SAN_VM_SYMBOL, { .string = val } };
    return obj;
}

int sanm_run(const san_program_t *program) {
  san_dbg("\nRunning program:\n");

  san_vector_t stack;
  sanv_create(&stack, sizeof(vm_object));
  
  SAN_VECTOR_FOR_EACH(program->bytecode, i, san_bytecode_t, code)
    switch (code->opcode) {

      case SAN_BYTECODE_PUSH: {
        switch (code->arg1.type) {
          case SAN_BYTECODE_TYPE_NUMBER_LITERAL: {
            vm_object obj = vm_int(program, code->arg1.ref);
            san_dbg("PUSH %d\n", obj.value.integer);
            sanv_push(&stack, (void*)&obj);
            break;
          }
          case SAN_BYTECODE_TYPE_IDENTIFIER: {
            vm_object obj = vm_symbol(program, code->arg1.ref);
            san_dbg("PUSH %s\n", obj.value.string);
            sanv_push(&stack, (void*)&obj);
            break;
          }
          case SAN_BYTECODE_TYPE_STRING_LITERAL: {
            vm_object obj = vm_string(program, code->arg1.ref);
            san_dbg("PUSH %s\n", obj.value.string);;
            sanv_push(&stack, (void*)&obj);
            break;
          }
        }
        break;
      }

      case SAN_BYTECODE_MUL: {
        vm_object arg1, arg2;
        sanv_pop(&stack, &arg1);
        sanv_pop(&stack, &arg2);
        san_dbg("MUL %d, %d\n", arg1.value.integer, arg2.value.integer);
        vm_object result = { SAN_VM_INT, { .integer = arg1.value.integer * arg2.value.integer } };
        sanv_push(&stack, &result);
        break;
      }

      case SAN_BYTECODE_ADD: {
        vm_object arg1, arg2;
        sanv_pop(&stack, &arg1);
        sanv_pop(&stack, &arg2);
        san_dbg("ADD %d, %d\n", arg1.value.integer, arg2.value.integer);
        vm_object result = { SAN_VM_INT, { .integer = arg1.value.integer + arg2.value.integer } };
        sanv_push(&stack, &result);
        break;
      }

      case SAN_BYTECODE_CALL: {
        vm_object fn, args;
        sanv_pop(&stack, &args);
        sanv_pop(&stack, &fn);

        const char *fname = fn.value.string;
        san_dbg("CALL %s\n", fname);

        if (strcmp(fname, "print") == 0) {
          if (args.type == SAN_VM_INT) {
            printf("%d\n", args.value.integer);
          } else if (args.type == SAN_VM_STRING) {
            printf("%s\n", args.value.string);
          }
        } else if(strcmp(fname, "square") == 0) {
          vm_object result = { SAN_VM_INT, { .integer = sanstd_squarei(args.value.integer) } };
          sanv_push(&stack, &result);
        } else if(strcmp(fname, "sqrt") == 0) {
          vm_object result = { SAN_VM_INT, { .integer = sanstd_sqrti(args.value.integer) } };
          sanv_push(&stack, &result);
        } else if(strcmp(fname, "factorial") == 0) {
          vm_object result = { SAN_VM_INT, { .integer = sanstd_factoriali(args.value.integer) } };
          sanv_push(&stack, &result);
        }

        break;
      }

    }
  SAN_VECTOR_END_FOR_EACH
/*
  int result;
  sanv_pop(&stack, &result);
  ("RESULT: %d\n", result);
  */

  sanv_destroy(&stack, sanv_nodestructor);

  return SAN_OK;
}

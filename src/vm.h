#ifndef __SAN_VM_H
#define __SAN_VM_H

#include "san.h"
#include "bytecode.h"

/*
typedef struct {
} san_runtime_t;
*/

int sanm_run(const san_program_t *program);

#endif

#ifndef __SAN_PVECTOR
#define __SAN_PVECTOR

#include "san.h"

typedef struct san_pvector_t {
  int type;
  int length;
  size_t elementSize;
  union {
    void *leaf;
    struct san_pvector_t *tree;
  } left;
  union {
    void *leaf;
    struct san_pvector_t *tree;
  } right;
} san_pvector_t;

int sanpv_create(san_pvector_t *out, size_t elementSize);
int sanpv_push(san_pvector_t const *vector, san_pvector_t *out, void const *value);
void* sanpv_back(san_pvector_t const *vector);
int sanpv_destroy(san_pvector_t *vector, int (*destructor)(void *));

#endif

#ifndef __SAN_VECTOR_H
#define __SAN_VECTOR_H

#include "san.h"

typedef struct {
  void *elems;
  size_t elementSize;
  unsigned int size, capacity;
} san_vector_t;

int sanv_create(san_vector_t *vector, size_t elementSize);
int sanv_destroy(san_vector_t *vector, int (*destructor)(void *));
void *sanv_nth(san_vector_t const *vector, int n);
void *sanv_back(san_vector_t const *vector);
int sanv_back_int(san_vector_t const *vector);
int sanv_push(san_vector_t *vector, void *value);
int sanv_push_int(san_vector_t *vector, int value);
int sanv_pop(san_vector_t *vector, void *value);
int sanv_pop_all(san_vector_t *vector);
int sanv_nodestructor(void *ptr);

#define SAN_VECTOR_FOR_EACH(__vector, __index, __type, __elem) \
  for (int __index = 0; __index < __vector.size; ++__index) { \
    __type* __elem = (__type*)sanv_nth(&__vector, __index);
#define SAN_VECTOR_FOR_EACH2(__vector2, __index2, __type2, __elem2) \
  for (int __index2 = 0; __index2 < __vector2.size; ++__index2) { \
    __type2* __elem2 = (__type2*)sanv_nth(&__vector2, __index2);

#define SAN_VECTOR_END_FOR_EACH }

#endif

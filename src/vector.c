#include "san.h"
#include "vector.h"

int sanv_create(san_vector_t *vector, size_t elementSize) {
  memset(vector, 0, sizeof(san_vector_t));
  vector->capacity = 8;
  vector->elementSize = elementSize;
  vector->size = 0;
  vector->elems = SAN_CALLOC(vector->capacity, elementSize);
  return SAN_OK;
}

int sanv_destroy(san_vector_t *vector, int (*destructor)(void *)) {
  int i;
  for (i = 0; i < vector->size; ++i) {
    destructor((char *)(vector->elems) + i * vector->elementSize);
  }
  SAN_FREE(vector->elems);
  return SAN_OK;
}

inline void *sanv_nth(san_vector_t const *vector, int n) {
  return (void*)((char*)(vector->elems) + (int)(vector->elementSize * n));
}

inline void *sanv_back(san_vector_t const *vector) {
  return vector->size > 0 ? sanv_nth(vector, vector->size - 1) : NULL;
}

inline int sanv_back_int(san_vector_t const *vector) {
  return *(int*)sanv_back(vector);
}

int sanv_push(san_vector_t *vector, const void *value) {
  void *back;

  if (vector->size + 1 > vector->capacity) {
    void *newElems = SAN_CALLOC(vector->capacity * 2, vector->elementSize);
    void *old = vector->elems;
    vector->capacity *= 2;
    memcpy(newElems, vector->elems, vector->size * vector->elementSize);
    vector->elems = newElems;
    SAN_FREE(old);
  }

  back = sanv_nth(vector, vector->size);
  memcpy(back, value, vector->elementSize);

  vector->size += 1;

  return SAN_OK;
}

inline int sanv_push_int(san_vector_t *vector, int value) {
  return sanv_push(vector, &value);
}

int sanv_pop(san_vector_t *vector, void *value) {
  if (vector->size <= 0) return SAN_FAIL;

  if (value != NULL)
    memcpy(value, sanv_back(vector), vector->elementSize);
  vector->size -= 1;
  return SAN_OK;
}

int sanv_pop_all(san_vector_t *vector) {
  vector->size = 0;
  return SAN_OK;
}

int sanv_nodestructor(void *ptr) { return SAN_OK; }

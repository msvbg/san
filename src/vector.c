#include "san.h"
#include "vector.h"
                                                                
int sanv_create(san_vector_t **vector, size_t elementSize) {  
  *vector = calloc(1, sizeof(san_vector_t)); 
  if (*vector == NULL) return SAN_FAIL; 
  (*vector)->capacity = 2;  
  (*vector)->elementSize = elementSize;
  (*vector)->size = 0; 
  (*vector)->elems = calloc((*vector)->capacity, elementSize); 
  return SAN_OK; 
} 

int sanv_destroy(san_vector_t *vector, int (*destructor)(void *)) {  
  int i; 
  for (i = 0; i < vector->size; ++i) { 
    destructor((char *)(vector->elems) + i * vector->elementSize); 
  } 
  free(vector); 
  vector = NULL; 
  return SAN_OK; 
}

inline void *sanv_nth(san_vector_t *vector, int n) {
  return (void*)((char*)(vector->elems) + (int)(vector->elementSize * n));
}

inline void *sanv_back(san_vector_t *vector) {
  return vector->size > 0 ? sanv_nth(vector, vector->size - 1) : NULL;
}

int sanv_push(san_vector_t *vector, void *value) {
  void *back;

  if (vector->size + 1 > vector->capacity) {
    void *newElems = calloc(vector->capacity * 2, vector->elementSize);
    void *old = vector->elems;
    vector->capacity *= 2;
    memcpy(newElems, vector->elems, vector->size * vector->elementSize);
    vector->elems = newElems;
    free(old);
  }

  back = sanv_nth(vector, vector->size);
  memcpy(back, value, vector->elementSize);

  vector->size += 1;

  return SAN_OK;
}


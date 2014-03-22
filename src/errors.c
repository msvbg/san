#include "san.h"
#include "errors.h"

int sane_create(san_error_t **error) {
  *error = malloc(sizeof(san_error_t));
  if (error == NULL) return SAN_FAIL;
  return SAN_OK;
}

int sane_destructor(void *ptr) {
  san_error_t *err = (san_error_t*)ptr;
  free(err);
  ptr = NULL;
  return SAN_OK;
}
#include "san.h"
#include "errors.h"

void __san_noop(int k, ...) {}

int sane_create(san_error_t **error) {
  *error = malloc(sizeof(san_error_t));
  if (error == NULL) return SAN_FAIL;
  return SAN_OK;
}

int sane_destructor(void *ptr) {
  return SAN_OK;
}

#include "pvector.h"

#define SAN_PVECTOR_ROOT     1
#define SAN_PVECTOR_LEAF     2
#define SAN_PVECTOR_INTERIOR 3

int sanpv_create(san_pvector_t *out, size_t elementSize) {
  out->type = SAN_PVECTOR_ROOT;
  out->length = 0;
  out->elementSize = elementSize;
  out->left.leaf = out->right.leaf = NULL;
  return SAN_OK;
}

int sanpv_push(san_pvector_t const *vector, san_pvector_t *out, void const *value) {
  sanpv_create(out, vector->elementSize);
  //san_pvector_t const *ptr = vector;

  if (vector->length == 0) {
    if (!(out->left.tree = malloc(sizeof(san_pvector_t)))) return SAN_FAIL;
    sanpv_create(out->left.tree, vector->elementSize);
    out->left.tree->type = SAN_PVECTOR_LEAF;
    if (!(out->left.tree->left.leaf = malloc(vector->elementSize))) return SAN_FAIL;
    memcpy(out->left.tree->left.leaf, value, vector->elementSize);
    out->length += 1;
    return SAN_OK;
  }

  /*
  while (ptr->left != NULL) {
      out->left = ptr->left;
      ptr = ptr->right;
      out = ptr->right;
  }*/

  /*
  if (vector->left. == NULL) {
    sanpv_create(out->left, out->elementSize);
    ++out->length;

    if (malloc(out->left->data, out->elementSize)) {
      memcpy(out->left->data, value, out->elementSize);
    } else {
      return SAN_FAIL;
    }
  }*/
  return SAN_OK;
}

void* sanpv_back(san_pvector_t const *vector) {
  if (vector->left.tree == NULL) return NULL;
  while (vector->type != SAN_PVECTOR_LEAF) {
    if (vector->right.tree != NULL) {
      vector = vector->right.tree;
    } else {
      vector = vector->left.tree;
    };
  }
  return vector->right.leaf != NULL ? vector->right.leaf : vector->left.leaf;
}

int sanpv_destroy(san_pvector_t *vector, int (*destructor)(void *)) {
  return SAN_OK;
}

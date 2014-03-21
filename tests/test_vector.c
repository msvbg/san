#include <check.h>
#include "../src/vector.h"

static int int_destructor(void *ptr) {
  return SAN_OK;
}

START_TEST (test_empty_vector) {
  san_vector_t *t;
  sanv_create(&t, sizeof(int));
  ck_assert_int_eq(t->size, 0);
  sanv_destroy(t, int_destructor);
} END_TEST

START_TEST (test_vector_push_back) {
  san_vector_t *t;
  int *val;
  sanv_create(&t, sizeof(int));

  val = malloc(sizeof(int));
  *val = 5;

  ck_assert_int_eq(t->size, 0);
  sanv_push(t, val);
  ck_assert_int_eq(t->size, 1);
  ck_assert_int_eq(*(int*)sanv_back(t), 5);
  
  *val = 9;
  sanv_push(t, val);
  ck_assert_int_eq(t->size, 2);
  ck_assert_int_eq(*(int*)sanv_back(t), 9);

  *val = 11;
  sanv_push(t, val);
  ck_assert_int_eq(t->size, 3);
  ck_assert_int_eq(*(int*)sanv_nth(t, 1), 9);
  ck_assert_int_eq(*(int*)sanv_back(t), 11);

  sanv_destroy(t, int_destructor);
  free(val);
} END_TEST

Suite* vector_suite(void) {
  Suite *s = suite_create("Vector");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_empty_vector);
  tcase_add_test(tc_core, test_vector_push_back);
  suite_add_tcase(s, tc_core);

  return s;
}

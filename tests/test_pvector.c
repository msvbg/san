#include <check.h>
#include "../src/san.h"
#include "../src/pvector.h"

int int_destructor(void *p) {
    return SAN_OK;
}

START_TEST (test_empty_vector) {
  san_pvector_t t;
  sanpv_create(&t, sizeof(int));
  ck_assert_int_eq(t.length, 0);
  sanpv_destroy(&t, int_destructor);
} END_TEST

START_TEST (test_push_back) {
  san_pvector_t t1, t2;
  sanpv_create(&t1, sizeof(int));

  int *val = malloc(sizeof(int));
  *val = 5;

  ck_assert_int_eq(t1.length, 0);
  sanpv_push(&t1, &t2, val);
  ck_assert_int_eq(t1.length, 0);
  ck_assert_int_eq(t2.length, 1);
  ck_assert_int_eq((int)sanpv_back(&t1), 0);
  ck_assert_int_eq(*(int*)sanpv_back(&t2), 5);

  free(val);
  sanpv_destroy(&t1, int_destructor);
} END_TEST

Suite* pvector_suite(void) {
  Suite *s = suite_create("Persistent vector");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_empty_vector);
  tcase_add_test(tc_core, test_push_back);
  suite_add_tcase(s, tc_core);

  return s;
}

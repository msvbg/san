#include <check.h>
#include "../src/parser.c"

START_TEST (test_empty_input) {
  san_ast_t *ast;
  parseTokens(NULL, &ast);
  ck_assert(ast == NULL);
} END_TEST

Suite* parser_suite(void) {
  Suite *s = suite_create("Parser");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_empty_input);
  suite_add_tcase(s, tc_core);

  return s;
}


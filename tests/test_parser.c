#include <check.h>
#include "../src/parser.h"

START_TEST (test_empty_input) {
  san_ast_t *ast;
  parseTokens(NULL, &ast);
  ck_assert(ast == NULL);
} END_TEST

START_TEST (test_addition) {
  san_vector_t *tokens;
  san_vector_t *errorList;
  //san_ast_t *ast;
  
  sanv_create(&errorList, sizeof(san_error_t));
  sanv_create(&tokens, sizeof(san_token_t));

  //sant_tokenize("4+5", tokens, errorList);
  //parseTokens(tokens, &ast);
  
} END_TEST

Suite* parser_suite(void) {
  Suite *s = suite_create("Parser");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_empty_input);
  tcase_add_test(tc_core, test_addition);
  suite_add_tcase(s, tc_core);

  return s;
}


#include <check.h>
#include "../src/tokenizer.h"

START_TEST (test_tokenize) {
  san_vector_t *tokens;
  san_vector_t *errList;

  sanv_create(&tokens, sizeof(san_token_t));
  sanv_create(&errList, sizeof(san_error_t));
  sant_tokenize("foo bar", tokens, errList);

  ck_assert_int_eq(tokens->size, 3);
  ck_assert_int_eq(((san_token_t*)sanv_nth(tokens, 0))->type, SAN_TOKEN_IDENTIFIER);
  ck_assert_int_eq(((san_token_t*)sanv_nth(tokens, 1))->type, SAN_TOKEN_WHITE_SPACE);
  ck_assert_int_eq(((san_token_t*)sanv_nth(tokens, 2))->type, SAN_TOKEN_IDENTIFIER);
  ck_assert_int_eq(errList->size, 0);
  
/*
  sanv_destroy(tokens, &sant_destructor);
  sanv_destroy(errList, &sane_destructor);
*/
  /* SEGFAULT :D
  readTokens("foo=9", &tokens, &errList);
  ck_assert_int_eq(tokens[1].type, SAN_TOKEN_EQUALS);
  ck_assert(errList == NULL);

  destroyTokens(tokens, 4);
  destroyErrorList(errList);

  readTokens("*+", &tokens, &errList);
  ck_assert_int_eq(tokens[0].type, SAN_TOKEN_TIMES);
  ck_assert_int_eq(tokens[1].type, SAN_TOKEN_PLUS);
  ck_assert(errList == NULL);

  destroyTokens(tokens, 3);
  destroyErrorList(errList);
  */

} END_TEST

Suite* tokenizer_suite(void) {
  Suite *s = suite_create("Tokenizer");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_tokenize);
  suite_add_tcase(s, tc_core);

  return s;
}

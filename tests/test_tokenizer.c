#include <check.h>
#include "../src/tokenizer.h"

#define nth(n) ((san_token_t*)sanv_nth(&tokens, n))
#define asrti(a, b) ck_assert_int_eq(a, b)

#define beginTokenize(str) { \
  san_vector_t tokens, errList; \
  sanv_create(&tokens, sizeof(san_token_t)); \
  sanv_create(&errList, sizeof(san_error_t)); \
  sant_tokenize(str, &tokens, &errList);

#define endTokenize \
  sanv_destroy(&tokens, &sant_destructor); \
  sanv_destroy(&errList, &sane_destructor); \
}

START_TEST (test_tokenize) {

  beginTokenize("foo bar")
    asrti(tokens.size, 4);
    asrti(nth(0)->type, SAN_TOKEN_IDENTIFIER);
    asrti(nth(1)->type, SAN_TOKEN_WHITE_SPACE);
    asrti(nth(2)->type, SAN_TOKEN_IDENTIFIER);
    asrti(nth(3)->type, SAN_TOKEN_END);
    asrti(errList.size, 0);
  endTokenize

  beginTokenize("123 + 5")
    asrti(tokens.size, 6);
    asrti(nth(0)->type, SAN_TOKEN_NUMBER);
    asrti(nth(1)->type, SAN_TOKEN_WHITE_SPACE);
    asrti(nth(2)->type, SAN_TOKEN_PLUS);
    asrti(nth(3)->type, SAN_TOKEN_WHITE_SPACE);
    asrti(nth(4)->type, SAN_TOKEN_NUMBER);
    asrti(nth(5)->type, SAN_TOKEN_END);
    asrti(errList.size, 0);
  endTokenize

  beginTokenize("   quuz = foo")
    asrti(tokens.size, 7);
    asrti(nth(0)->type, SAN_TOKEN_INDENTATION);
    asrti(nth(1)->type, SAN_TOKEN_IDENTIFIER);
    asrti(nth(2)->type, SAN_TOKEN_WHITE_SPACE);
    asrti(nth(3)->type, SAN_TOKEN_EQUALS);
    asrti(nth(4)->type, SAN_TOKEN_WHITE_SPACE);
    asrti(nth(5)->type, SAN_TOKEN_IDENTIFIER);
    asrti(nth(6)->type, SAN_TOKEN_END);
    asrti(errList.size, 0);
  endTokenize

} END_TEST

Suite* tokenizer_suite(void) {
  Suite *s = suite_create("Tokenizer");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_tokenize);
  suite_add_tcase(s, tc_core);

  return s;
}

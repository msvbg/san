#include "../src/tokenizer.c"
#include <check.h>

START_TEST (test_is_alphabetic) {
  ck_assert_int_eq(isAlphabetic('q'), 1);
  ck_assert_int_eq(isAlphabetic('Z'), 1);
  ck_assert_int_eq(isAlphabetic('2'), 0);
} END_TEST

START_TEST (test_is_alphanumeric) {
  ck_assert_int_eq(isAlphanumeric('2'), 1);
  ck_assert_int_eq(isAlphanumeric('k'), 1);
  ck_assert_int_eq(isAlphanumeric('P'), 1);
} END_TEST

START_TEST (test_is_keyword) {
  ck_assert_int_eq(isKeyword("if"), 1);
  ck_assert_int_eq(isKeyword("foobar"), 0);
} END_TEST

START_TEST (test_classify_token) {
  ck_assert_int_eq(classifyToken("Abc"), SAN_TOKEN_IDENTIFIER_OR_KEYWORD);
  ck_assert_int_eq(classifyToken(" \t"), SAN_TOKEN_WHITE_SPACE);
} END_TEST

START_TEST (test_accept_char) {
  int i;
  token_t *token;
  createToken(&token);
  
  acceptChar('k', token);
  ck_assert_str_eq(token->raw, "k");
  acceptChar('w', token);
  ck_assert_str_eq(token->raw, "kw");

  for (i = 0; i < 100; ++i)
    acceptChar('z', token);
  ck_assert_int_eq(token->raw[99], 'z');

  destroyTokens(token, 1);
} END_TEST

START_TEST (test_append_token) {
  token_t *tokens;
  unsigned int nTokens = 0, tokenCapacity = 2;
  createTokens(&tokens, tokenCapacity);
  
  appendToken(&tokens, &nTokens, &tokenCapacity);
  ck_assert_int_eq(nTokens, 1);
  ck_assert_int_eq(tokenCapacity, 2);

  appendToken(&tokens, &nTokens, &tokenCapacity);
  ck_assert_int_eq(nTokens, 2);
  ck_assert_int_eq(tokenCapacity, 2);

  appendToken(&tokens, &nTokens, &tokenCapacity);
  ck_assert_int_eq(nTokens, 3);
  ck_assert(tokenCapacity > 2);

  destroyTokens(tokens, tokenCapacity);
  
} END_TEST

START_TEST (test_read_identifier_or_keyword) {
  const char *ptr = "foo";
  token_t *out;

  createToken(&out);
  ck_assert_int_eq(readIdentifierOrKeyword(&ptr, out), SAN_OK);
  ck_assert_int_eq(out->type, SAN_TOKEN_IDENTIFIER);
  destroyTokens(out, 1);

  ptr = "if";
  createToken(&out);
  ck_assert_int_eq(readIdentifierOrKeyword(&ptr, out), SAN_OK);
  ck_assert_int_eq(out->type, SAN_TOKEN_KEYWORD);
  destroyTokens(out, 1);

} END_TEST

START_TEST (test_read_white_space) {
  const char *input = "  \t\n";
  const char *inPtr = input;
  token_t *token;
  createToken(&token);
  
  readWhiteSpace(&inPtr, token);
  ck_assert_str_eq(token->raw, input);

  destroyTokens(token, 1);
} END_TEST

START_TEST (test_create_token) {
  token_t *token;
  ck_assert_int_eq(createToken(&token), SAN_OK);
  ck_assert_int_eq(token->type, SAN_NO_TOKEN);
  ck_assert(token->rawSize > 0);
  ck_assert_int_eq(token->raw[0], 0);
  destroyTokens(token, 1);
} END_TEST

START_TEST (test_create_tokens) {
  int i;
  token_t *tokens;
  ck_assert_int_eq(createTokens(&tokens, 1024), SAN_OK);

  for (i = 0; i < 1024; ++i) {
    ck_assert_int_eq(tokens[i].type, SAN_NO_TOKEN);
    ck_assert_int_eq(tokens[i].raw[0], 0);
  }

  destroyTokens(tokens, 1024);
} END_TEST

Suite* tokenizer_suite(void) {
  Suite *s = suite_create("Tokenizer");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_is_alphabetic);
  tcase_add_test(tc_core, test_is_alphanumeric);
  tcase_add_test(tc_core, test_is_keyword);
  tcase_add_test(tc_core, test_classify_token);
  tcase_add_test(tc_core, test_read_identifier_or_keyword);
  tcase_add_test(tc_core, test_read_white_space);
  tcase_add_test(tc_core, test_accept_char);
  tcase_add_test(tc_core, test_append_token);
  tcase_add_test(tc_core, test_create_token);
  tcase_add_test(tc_core, test_create_tokens);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(void) {
  int num_failed;
  Suite *s = tokenizer_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  num_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

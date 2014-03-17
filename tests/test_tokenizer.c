#include <check.h>
#include "../src/tokenizer.c"

START_TEST (test_create_tokenizer_state) {
  san_tokenizer_state_t *state;
  createTokenizerState(&state);
  ck_assert_int_eq(state->line, 1);
  ck_assert_int_eq(state->column, 1);
  ck_assert_int_eq(state->errorList->nErrors, 0);
  ck_assert(state->errorList->errorCapacity > 0);
} END_TEST

START_TEST (test_advance) {
  san_tokenizer_state_t *state;
  createTokenizerState(&state);
  state->inputPtr = "a\nb";

  ck_assert_int_eq(state->line, 1);
  ck_assert_int_eq(state->column, 1);

  advance(state);

  ck_assert_int_eq(state->line, 1);
  ck_assert_int_eq(state->column, 2);

  advance(state);

  ck_assert_int_eq(state->line, 2);
  ck_assert_int_eq(state->column, 1);

  advance(state);

  ck_assert_int_eq(state->line, 2);
  ck_assert_int_eq(state->column, 2);
} END_TEST

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

START_TEST (test_is_digit) {
  ck_assert_int_eq(isDigit('5'), 1);
} END_TEST

START_TEST (test_classify_token) {
  ck_assert_int_eq(classifyToken("Abc"), SAN_TOKEN_IDENTIFIER_OR_KEYWORD);
  ck_assert_int_eq(classifyToken(" \t"), SAN_TOKEN_WHITE_SPACE);
  ck_assert_int_eq(classifyToken("459"), SAN_TOKEN_NUMBER);
  ck_assert_int_eq(classifyToken("="), SAN_TOKEN_EQUALS);
} END_TEST

START_TEST (test_accept_char) {
  int i;
  san_token_t *token;
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
  san_token_t *tokens;
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
  const char *in = "foo";
  san_tokenizer_state_t *state;
  san_token_t *out;

  createToken(&out);
  createTokenizerState(&state);
  state->inputPtr = in;
  state->outputPtr = out;

  ck_assert_int_eq(readIdentifierOrKeyword(state), SAN_OK);
  ck_assert_int_eq(out->type, SAN_TOKEN_IDENTIFIER);
  destroyToken(out);

  in = "if";
  createToken(&out);
  state->inputPtr = in;
  state->outputPtr = out;

  ck_assert_int_eq(readIdentifierOrKeyword(state), SAN_OK);
  ck_assert_int_eq(out->type, SAN_TOKEN_KEYWORD);
  destroyToken(out);

} END_TEST

START_TEST (test_read_white_space) {
  const char *input = "  \t\n";
  san_tokenizer_state_t *state;
  san_token_t *token;

  createTokenizerState(&state);
  createToken(&token);

  state->inputPtr = input;
  state->outputPtr = token;
  
  readWhiteSpace(state);
  ck_assert_str_eq(token->raw, input);

  destroyTokenizerState(state);
  destroyToken(token);

} END_TEST

START_TEST (test_create_token) {
  san_token_t *token;
  ck_assert_int_eq(createToken(&token), SAN_OK);
  ck_assert_int_eq(token->type, SAN_NO_TOKEN);
  ck_assert(token->rawSize > 0);
  ck_assert_int_eq(token->raw[0], 0);
  destroyTokens(token, 1);
} END_TEST

START_TEST (test_create_tokens) {
  int i;
  san_token_t *tokens;
  ck_assert_int_eq(createTokens(&tokens, 1024), SAN_OK);

  for (i = 0; i < 1024; ++i) {
    ck_assert_int_eq(tokens[i].type, SAN_NO_TOKEN);
    ck_assert_int_eq(tokens[i].raw[0], 0);
  }

  destroyTokens(tokens, 1024);
} END_TEST

START_TEST (test_read_tokens) {
  san_token_t *tokens;
  san_error_list_t *errList;

  readTokens("foo bar", &tokens, &errList);
  ck_assert_int_eq(tokens[0].type, SAN_TOKEN_IDENTIFIER);
  ck_assert_int_eq(tokens[1].type, SAN_TOKEN_WHITE_SPACE);
  ck_assert_int_eq(tokens[2].type, SAN_TOKEN_IDENTIFIER);
  ck_assert_int_eq(tokens[3].type, SAN_TOKEN_END);
  ck_assert(errList == NULL);
  
  /* TODO: destroy tokens */
  /* TODO: destroy error list */

  readTokens("foo=9", &tokens, &errList);
  ck_assert_int_eq(tokens[1].type, SAN_TOKEN_EQUALS);
  ck_assert(errList == NULL);


} END_TEST

Suite* tokenizer_suite(void) {
  Suite *s = suite_create("Tokenizer");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_create_tokenizer_state);
  tcase_add_test(tc_core, test_advance);
  tcase_add_test(tc_core, test_is_alphabetic);
  tcase_add_test(tc_core, test_is_alphanumeric);
  tcase_add_test(tc_core, test_is_keyword);
  tcase_add_test(tc_core, test_is_digit);
  tcase_add_test(tc_core, test_classify_token);
  tcase_add_test(tc_core, test_read_identifier_or_keyword);
  tcase_add_test(tc_core, test_read_white_space);
  tcase_add_test(tc_core, test_accept_char);
  tcase_add_test(tc_core, test_append_token);
  tcase_add_test(tc_core, test_create_token);
  tcase_add_test(tc_core, test_create_tokens);
  tcase_add_test(tc_core, test_read_tokens);
  suite_add_tcase(s, tc_core);

  return s;
}

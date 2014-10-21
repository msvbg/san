#include <check.h>
#include "../src/bytecodegen.h"

#define BYTECODE_OF(x, is) \
  san_vector_t tokens, bytecode, errors; \
  sanv_create(&tokens, sizeof(san_token_t)); \
  sanv_create(&errors, sizeof(san_error_t)); \
  sanv_create(&bytecode, sizeof(san_bytecode_t)); \
  sant_tokenize((x), &tokens, &errors); \
  san_node_t ast; \
  sanp_parse(&tokens, &ast, &errors); \
  sanb_generate(&ast, &bytecode, &errors); \
  SAN_VECTOR_FOR_EACH(bytecode, i, san_bytecode_t, )

START_TEST (test_empty_input) {


} END_TEST

Suite* bytecodegen_suite(void) {
  Suite *s = suite_create("Bytecode Generator");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_empty_input);
  suite_add_tcase(s, tc_core);

  return s;
}

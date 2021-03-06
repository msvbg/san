#include <check.h>
#include "../src/parser.h"

typedef struct {
  int what, parent, satisfied;
  const char *msg;
} expectation_t;

typedef struct {
  san_node_t node;
  san_node_t const *parent;
} node_with_parent_t;

int noop_destructor(void *ptr) {return SAN_OK;}

#define BEGIN_WALK_TREE(_expr) { \
  san_vector_t tokens, errorList; \
  san_node_t ast; \
  const char *expr = _expr; \
  sanv_create(&errorList, sizeof(san_error_t)); \
  sanv_create(&tokens, sizeof(san_token_t)); \
  sant_tokenize(expr, &tokens, &errorList); \
  sanp_parse(&tokens, &ast, &errorList); \
  san_vector_t expectations, flat; \
  sanv_create(&expectations, sizeof(expectation_t)); \
  sanv_create(&flat, sizeof(node_with_parent_t)); \

#define with_parent

#define expect_exists(what, parent) do { \
  expectation_t exp = { what, parent, 0, #what " " #parent }; \
  sanv_push(&expectations, &exp); \
} while (0);

#define expect_no_errors do { \
  if (errorList.size > 0) { \
    SAN_VECTOR_FOR_EACH(errorList, i, san_error_t, error) \
      printf("[%d:%d] Error %d: %s\n", error->line, error->column, i+1, error->msg); \
    SAN_VECTOR_END_FOR_EACH \
  } \
  ck_assert_int_eq(errorList.size, 0); \
} while(0);

void __flatten(san_vector_t *vec, san_node_t const* root) {
  if (root->children.size > 0) {
    SAN_VECTOR_FOR_EACH(root->children, i, san_node_t, child)
      node_with_parent_t tmp = { *child, root };
      sanv_push(vec, &tmp);
      __flatten(vec, child);
    SAN_VECTOR_END_FOR_EACH
  }
}

#define END_WALK_TREE \
  __flatten(&flat, &ast); \
  SAN_VECTOR_FOR_EACH(flat, i, node_with_parent_t, nwp) \
    SAN_VECTOR_FOR_EACH(expectations, j, expectation_t, exp) \
      if (!exp->satisfied && \
        nwp->node.type == exp->what && \
        nwp->parent->type == exp->parent) { \
        exp->satisfied = 1; \
      } \
    SAN_VECTOR_END_FOR_EACH \
  SAN_VECTOR_END_FOR_EACH \
  SAN_VECTOR_FOR_EACH(expectations, i, expectation_t, exp) \
    if (!exp->satisfied) { \
      printf("Unsatisfied expectation: %s in '%s'\n", exp->msg, expr); \
      ck_assert_int_eq(1, 0); \
    } \
  SAN_VECTOR_END_FOR_EACH \
  sanv_destroy(&tokens, &sant_destructor); \
  sanv_destroy(&errorList, &sane_destructor); \
  sanv_destroy(&expectations, &noop_destructor); \
  sanv_destroy(&flat, &noop_destructor); \
}

START_TEST (test_empty_input) {
  san_node_t ast;
  san_vector_t errors;
  sanv_create(&errors, sizeof(san_error_t));
  ck_assert_int_eq(sanp_parse(NULL, &ast, &errors), SAN_FAIL);
} END_TEST

START_TEST (test_function_definition) {

  BEGIN_WALK_TREE("let somefunc param1 param2 = n + 1")
    expect_exists(
      SAN_PARSER_FUNCTION_LVALUE
      , with_parent SAN_PARSER_VARIABLE_EXPRESSION)
    expect_exists(
      SAN_PARSER_MULTIPLICATIVE_EXPRESSION
      , with_parent SAN_PARSER_ADDITIVE_EXPRESSION)
    expect_exists(
      SAN_PARSER_FUNCTION_PARAMETER
      , with_parent SAN_PARSER_FUNCTION_PARAMETER_LIST)
    expect_exists(
      SAN_PARSER_VARIABLE_EXPRESSION
      , with_parent SAN_PARSER_EXPRESSION
    )
    expect_no_errors
  END_WALK_TREE

} END_TEST

START_TEST (test_function_indentation) {

  BEGIN_WALK_TREE("let\n somefunc\n param1 param2 = let x y =\n\n y")
    expect_exists(
      SAN_PARSER_BLOCK
      , with_parent SAN_PARSER_VARIABLE_EXPRESSION)
    expect_no_errors
  END_WALK_TREE

  BEGIN_WALK_TREE(
    "let\n"
    " somefunc\n"
    "   param1 param2 =\n"
    "    let x y =\n"
    "     y\n"
    "    let z = 5")
    expect_exists(
      SAN_PARSER_BLOCK
      , with_parent SAN_PARSER_VARIABLE_EXPRESSION)
    expect_no_errors
  END_WALK_TREE

} END_TEST

START_TEST (test_if_expression) {

  BEGIN_WALK_TREE("if x then y")
    expect_exists(
      SAN_PARSER_EXPRESSION
      , with_parent SAN_PARSER_IF_EXPRESSION)
    expect_no_errors
  END_WALK_TREE

  BEGIN_WALK_TREE("if x\n"
                  "  y")
    expect_exists(
      SAN_PARSER_EXPRESSION
      , with_parent SAN_PARSER_IF_EXPRESSION)
    expect_no_errors
  END_WALK_TREE

} END_TEST

Suite* parser_suite(void) {
  Suite *s = suite_create("Parser");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_empty_input);
  tcase_add_test(tc_core, test_function_definition);
  tcase_add_test(tc_core, test_function_indentation);
  tcase_add_test(tc_core, test_if_expression);
  suite_add_tcase(s, tc_core);

  return s;
}

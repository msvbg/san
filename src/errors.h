#ifndef __SAN_ERRORS_H
#define __SAN_ERRORS_H

/*
 * Errors
 */
typedef struct {
  int code;
  char msg[200];
  int line, column;
  const char *file;
} san_error_t;

#define SAN_ERROR_INTERNAL                     1000
#define SAN_ERROR_INTERNAL_MSG \
  "An internal error occurred"

#define SAN_ERROR_ADJACENT_NUMBER_ALPHA        1001
#define SAN_ERROR_ADJACENT_NUMBER_ALPHA_MSG  \
  "An alphabetic character ('%c') cannot directly follow a number (%s) without a delimiter"

#define SAN_ERROR_INVALID_CHARACTER            1002
#define SAN_ERROR_INVALID_CHARACTER_MSG \
  "Encountered an invalid characteter ('%c')"

#define SAN_ERROR_EXPECTED_EXPRESSION          1003
#define SAN_ERROR_EXPECTED_EXPRESSION_MSG \
  "Expected an expression after '%s'"

#define SAN_ERROR_EXPECTED_TERM                1004
#define SAN_ERROR_EXPECTED_TERM_MSG \
  "Expected a term in additive expression after '%s'"

#define SAN_ERROR_EXPECTED_TOKEN               1005
#define SAN_ERROR_EXPECTED_TOKEN_MSG \
  "Expected token '%s' after expression '%s'"

#define SAN_ERROR_TAB_AS_INDENTATION           1006
#define SAN_ERROR_TAB_AS_INDENTATION_MSG \
  "Tabs are not permitted to be used as indentation"

#define SAN_ERROR_EXPECTED_LVALUE              1007
#define SAN_ERROR_EXPECTED_LVALUE_MSG \
  "Expected an L-value after 'let' keyword in '%s'"

#define SAN_ERROR_EXPECTED_FUNCTION_BODY       1008
#define SAN_ERROR_EXPECTED_FUNCTION_BODY_MSG \
  "Expected a function body after '=' in '%s'"

#define SAN_ERROR_BAD_INDENTATION              1009
#define SAN_ERROR_BAD_INDENTATION_MSG \
  "Indentation error"

#define SAN_ERROR_EXPECTED_BLOCK               1010
#define SAN_ERROR_EXPECTED_BLOCK_MSG \
  "In '%s', expected a block after '%s'"

int sane_create(san_error_t **error);
int sane_destructor(void *ptr);

#endif

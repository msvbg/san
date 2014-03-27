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
  "Expected a term in additive expression at '%s'"

#define SAN_ERROR_EXPECTED_TOKEN               1005
#define SAN_ERROR_EXPECTED_TOKEN_MSG \
  "Expected token '%s' after expression '%s'"

int sane_create(san_error_t **error);
int sane_destructor(void *ptr);

#endif

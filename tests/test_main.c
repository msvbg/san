#include <check.h>
#include "test_tokenizer.c"
#include "test_parser.c"

void runSuite(Suite* (*suiteFn)(void), int *numFailed) {
  Suite *s = suiteFn();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  *numFailed = srunner_ntests_failed(sr);
  srunner_free(sr);
}

int main(void) {
  int numFailed, numTotalFailed = 0, i;
  Suite* (*suites[])(void) = {
    &tokenizer_suite,
    &parser_suite,
    0
  };

  for (i = 0; suites[i] != 0; ++i) {
    runSuite(suites[i], &numFailed);
    numTotalFailed += numFailed;
  }

  return numTotalFailed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
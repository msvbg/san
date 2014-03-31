#include "san.h"
#include "vector.h"
#include "errors.h"
#include "tokenizer.h"
#include "parser.h"

void print_error(san_error_t const *error) {
  const char *file = "CLI";
  if (error->file != NULL)
    file = error->file;

  printf(
    "\x1B[1;37m[%s:%d:%d] "
    "\x1B[1;31mERROR S%d:"
    "\x1B[1;37m %s\n\x1B[0m",
    file, error->line, error->column,
    error->code, error->msg);
}

int main(int argc, const char **argv) {
  static const int MAX_LINE_LEN = 1024;
  int isReadingMultiline = 0;
  san_vector_t input;
  sanv_create(&input, sizeof(char) * MAX_LINE_LEN);

  while (1) {
    char line[MAX_LINE_LEN];

    if (!isReadingMultiline) {
      printf("san> ");
      fgets(line, MAX_LINE_LEN, stdin);
      sanv_pop_all(&input);
      sanv_push(&input, line);

      if (strcmp(line, "quit\n") == 0) {
        break;
      }
    } else {
      printf("...> ");
      fgets(line, MAX_LINE_LEN, stdin);
      if (strcmp(line, "\n") != 0) {
        sanv_push(&input, line);
        continue;
      }
    }

    san_vector_t tokens;
    sanv_create(&tokens, sizeof(san_token_t));

    san_vector_t errList;
    sanv_create(&errList, sizeof(san_error_t));

    char *inputString = malloc(sizeof(char) * MAX_LINE_LEN * input.size);
    char *inputPtr = inputString;
    for (int i = 0; i < input.size; ++i) {
      char *thisLine = (char*)sanv_nth(&input, i);
      int lineLen = strlen(thisLine);
      strncpy(inputPtr, thisLine, lineLen);
      inputPtr += lineLen;
    }

    if (sant_tokenize(inputString, &tokens, &errList) == SAN_OK) {
      if (errList.size != 0) {
        SAN_VECTOR_FOR_EACH(errList, i, san_error_t, error)
          print_error(error);
        SAN_VECTOR_END_FOR_EACH
      }
    }

    san_node_t root;
    sanp_parse(&tokens, &root, &errList);

    isReadingMultiline = 0;
    san_error_t *last = sanv_back(&errList);
    if (errList.size == 1 && last->code == SAN_ERROR_EXPECTED_BLOCK && !isReadingMultiline) {
      isReadingMultiline = 1;
    } else if (errList.size != 0) {
      SAN_VECTOR_FOR_EACH(errList, i, san_error_t, error)
        print_error(error);
      SAN_VECTOR_END_FOR_EACH
      printf("ERRORS: %d\n", errList.size);
    }

    sanv_destroy(&tokens, &sant_destructor);
    sanv_destroy(&errList, &sane_destructor);
    free(inputString);
  }

  sanv_destroy(&input, sanv_nodestructor);

  return 0;
}

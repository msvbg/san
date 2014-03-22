#include "san.h"
#include "vector.h"
#include "errors.h"
#include "tokenizer.h"

void printError(san_error_t const *error) {
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
  
  while (1) {
    printf("> "); 

    char line[1024];
    fgets(line, 1024, stdin);

    /* Replace newline at end with \0 */
    if (line[strlen(line) -1] == '\n') {
      line[strlen(line) - 1] = '\0'; 
    }

    if (strcmp(line, "quit") == 0) {
      break;
    }

    san_vector_t tokens;
    sanv_create(&tokens, sizeof(san_token_t));

    san_vector_t errList;
    sanv_create(&errList, sizeof(san_error_t));

    if (sant_tokenize(line, &tokens, &errList) == SAN_OK) {
      if (errList.size != 0) {
        SAN_VECTOR_FOR_EACH(errList, i, san_error_t, error)
          printError(error);
        SAN_VECTOR_END_FOR_EACH
      }
      
      SAN_VECTOR_FOR_EACH(tokens, i, san_token_t, token)
        printf("%d: \"%s\"\n", token->type, token->raw);
      SAN_VECTOR_END_FOR_EACH
    }

    //sanv_destroy(tokens, &sant_destructor);
    //sanv_destroy(errList, &sane_destructor);
  }

  return 0;
}

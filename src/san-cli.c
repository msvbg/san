#include "san.h"

void printError(san_error_t const *error) {
  const char *file = "CLI";
  if (error->file != NULL)
    file = error->file;

  printf(
    "\e[1;37m[%s:%d:%d] "
    "\e[1;31mERROR S%d:"
    "\e[1;37m %s\n\e[0m",
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

    san_token_t *tokens = NULL;
    san_error_list_t *errList = NULL;
    char *linePtr = line;
    if (readTokens(linePtr, &tokens, &errList) == SAN_OK) {
      if (errList != NULL) {
        for (int i = 0; i < errList->nErrors; ++i) {
          printError(errList->errors + i);
        }
      }
      
      for (int i = 0; tokens[i].type != SAN_TOKEN_END; ++i) {
        printf("%d: \"%s\"\n", tokens[i].type, tokens[i].raw);
      }
      free(tokens);
    }
  }

  return 0;
}

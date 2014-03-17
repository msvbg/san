#include "san.h"

int main(int argc, const char **argv) {
  
  while (SAN_TRUE) {
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

    token_t *tokens = NULL;
    char *linePtr = line;
    if (readTokens(linePtr, &tokens) == SAN_OK) {
      for (int i = 0; tokens[i].type != SAN_TOKEN_END; ++i) {
        printf("%d: \"%s\"\n", tokens[i].type, tokens[i].raw);
      }
      free(tokens);
    }
  }

  return 0;
}

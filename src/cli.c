#include "san.h"
#include "vector.h"
#include "errors.h"
#include "tokenizer.h"
#include "parser.h"

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

int eval(san_node_t *node) {
  int total;

  if (node->type == SAN_PARSER_ADDITIVE_EXPRESSION) {
    total = 0;
    SAN_VECTOR_FOR_EACH(node->children, i, san_node_t, n)
      total += eval(n);
    SAN_VECTOR_END_FOR_EACH
    return total;
  } else if (node->type == SAN_PARSER_MULTIPLICATIVE_EXPRESSION) {
    total = 1;
    SAN_VECTOR_FOR_EACH(node->children, i, san_node_t, n)
      total *= eval(n);
    SAN_VECTOR_END_FOR_EACH
    return total;
  } else if(node->type == SAN_PARSER_NUMBER_LITERAL) {
    return atoi(node->token->raw);
  } else if(node->children.size == 1) {
    return eval(sanv_nth(&node->children, 0));
  }
  return -1;
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

    strcpy(line,
      "   f x=5\n"
      "  f x=5\n"
      " g x y z=5*z"
    );

    if (sant_tokenize(line, &tokens, &errList) == SAN_OK) {
      if (errList.size != 0) {
        SAN_VECTOR_FOR_EACH(errList, i, san_error_t, error)
          printError(error);
        SAN_VECTOR_END_FOR_EACH
      }
    }

    SAN_VECTOR_FOR_EACH(tokens, i, san_token_t, tok)
      printf("raw: '%s', type: %d, indent: %d\n", tok->raw, tok->type, tok->column);
    SAN_VECTOR_END_FOR_EACH

    /*san_node_t root;
    parse(&tokens, &root, &errList);
    if (errList.size != 0) {
      SAN_VECTOR_FOR_EACH(errList, i, san_error_t, error)
        printError(error);
      SAN_VECTOR_END_FOR_EACH
    }
    printf("ERRORS: %d\n", errList.size);
    printf("RESULT: %d\n", eval(&root));*/
    //sanv_destroy(tokens, &sant_destructor);
    //sanv_destroy(errList, &sane_destructor);
  }

  return 0;
}

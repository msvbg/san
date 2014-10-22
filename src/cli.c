#include "san.h"
#include "vector.h"
#include "errors.h"
#include "tokenizer.h"
#include "parser.h"
#include "bytecode.h"
#include "vm.h"

void print_help() {
  printf("san version %d.%d.%d\n\n",
    SAN_VERSION_MAJOR,
    SAN_VERSION_MINOR,
    SAN_VERSION_PATCH);
  printf("Usage: san [ --repl | source.san ]\n");
}

void print_error(const char *file, const char *source, san_error_t const *error) {
  if (error->file != NULL)
    file = error->file;

  printf(
    "\x1B[1;37m[%s:%d:%d] "
    "\x1B[1;31mERROR S%d:"
    "\x1B[1;37m %s\n\x1B[0m",
    file, error->line, error->column,
    error->code, error->msg);

  /* Print line with error */
  int lineNo = 0;
  const char *newLine = "\n", *eof = "\0";
  for (const char *sourcePtr = source; *sourcePtr != '\0'; ++sourcePtr) {
    if (sourcePtr == source || *(sourcePtr-1) == '\n') {
      ++lineNo;
      const char *lineEnd = strstr(sourcePtr, newLine);
      if (lineEnd == NULL) {
        lineEnd = strstr(sourcePtr, eof);
      }
      int len = (lineEnd - sourcePtr);
      if (lineNo == error->line || lineNo == error->line - 1 || lineNo == error->line + 1) {
        char *sourceLine = (char*)malloc(sizeof(char) * len + 1);
        strncpy(sourceLine, sourcePtr, len);
        sourceLine[len] = 0;
        printf("\x1B[1;34m%04d\x1B[0m  %s\n", lineNo, sourceLine);
        free(sourceLine);
      }
      if (lineNo == error->line) {
        char *caretLine = (char*)malloc(sizeof(char) * len*2 + 12);
        memset(caretLine, '~', len*2 + 12);
        caretLine[len*2+12] = 0;
        strncpy(&caretLine[error->column], "\x1B[1;37m^\x1B[0m", 12);
        printf("     %s\n", caretLine);
        free(caretLine);
      }
    }
  }
}

void start_repl() {
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
    }

    san_node_t root;
    sanp_parse(&tokens, &root, &errList);

    isReadingMultiline = 0;
    san_error_t *last = sanv_back(&errList);
    if (errList.size == 1 && last->code == SAN_ERROR_EXPECTED_BLOCK && !isReadingMultiline) {
      isReadingMultiline = 1;
    } else if (errList.size != 0) {
      SAN_VECTOR_FOR_EACH(errList, i, san_error_t, error)
        print_error("CLI", inputString, error);
      SAN_VECTOR_END_FOR_EACH
      printf("ERRORS: %d\n", errList.size);
    }

    san_program_t program;
    sanb_generate(&root, &program, &errList);

    sanv_destroy(&tokens, &sant_destructor);
    sanv_destroy(&errList, &sane_destructor);
    free(inputString);
  }

  sanv_destroy(&input, sanv_nodestructor);
}

void run_file(const char *file) {
  FILE *fp;
  long fsize;
  char *input;
  san_vector_t tokens, errList;
  san_node_t root;

  fp = fopen(file, "r");
  if (fp == NULL) {
    printf("Unable to read file %s.\n", file);
    exit(1);
  }

  fseek(fp, 0, SEEK_END);
  fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  input = malloc(fsize + 1);
  fread(input, fsize, 1, fp);
  fclose(fp);

  input[fsize] = 0;

  sanv_create(&tokens, sizeof(san_token_t));
  sanv_create(&errList, sizeof(san_error_t));

  if (sant_tokenize(input, &tokens, &errList) == SAN_OK) {
  }

  sanp_parse(&tokens, &root, &errList);

  if (errList.size != 0) {
    SAN_VECTOR_FOR_EACH(errList, i, san_error_t, error)
      print_error(file, input, error);
    SAN_VECTOR_END_FOR_EACH
    printf("ERRORS: %d\n", errList.size);
  }

  san_program_t program;
  sanb_generate(&root, &program, &errList);

  sanm_run(&program);

  sanv_destroy(&tokens, sant_destructor);
  sanv_destroy(&errList, sane_destructor);
}

int main(int argc, const char **argv) {
  if (argc == 1) {
    print_help();
  } else if (argc == 2) {
    if (strcmp(argv[1], "--repl") == 0) {
      start_repl();
    } else {
      run_file(argv[1]);
    }
  }

  return 0;
}

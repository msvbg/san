STD=-std=c99 -pedantic
WARN=-Wall -Werror
DBG=-g -ferror-limit=5
SAN_CFLAGS=$(STD) $(WARN) $(DBG)
TEST_LDFLAGS=-L/usr/local/Cellar/check/0.9.10/lib -lcheck

SAN_CC=cc $(SAN_CFLAGS)

src_files=errors.c tokenizer.c parser.c vector.c bytecodegen.c
test_src_files=test_tokenizer.c test_parser.c test_vector.c	\
	test_bytecodegen.c test_main.c

main_object=obj/cli.o
objects=$(patsubst %.c,obj/%.o,$(src_files))
test_objects=$(patsubst %.c, obj/tests/%.o, $(test_src_files))

obj/%.o: src/%.c
	@mkdir -p obj
	@$(SAN_CC) -c -o $@ $<

obj/tests/%.o: tests/%.c
	@mkdir -p obj/tests
	@$(SAN_CC) -c -o $@ $<

.DEFAULT: default
default: san #san_test
	@echo Build succeeded!

san: $(main_object) $(objects)
	@mkdir -p build
	@echo Building $@ \> build/$@
	@$(SAN_CC) -o build/$@ $^

san_test: $(objects) $(test_objects)
	@mkdir -p build
	@echo Building $@ \> build/$@
	@$(SAN_CC) -o build/$@ $^ -L/usr/local/Cellar/check/0.9.10/lib -lcheck

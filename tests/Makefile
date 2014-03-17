STD=-std=c99 -pedantic
WARN=-Wall -Werror
TEST_LDFLAGS=-L/usr/local/Cellar/check/0.9.10/lib -lcheck
CC=cc $(STD) $(WARN)

objects=test_main.o

default: $(objects)
	$(CC) $(TEST_LDFLAGS) -o test $(objects)
	rm -f *.o


STD=-std=c99 -pedantic
WARN=-Wall -Werror
DEBUG=-g
SAN_CFLAGS=$(STD) $(WARN) $(DEBUG)

SAN_CC=cc $(SAN_CFLAGS)

objects=san-cli.o tokenizer.o

san: $(objects)
	mkdir -p ../build
	$(SAN_CC) -o ../build/san $(objects)
	rm -f *.o

san-cli.o tokenizer.o: san.h

.PHONY: install
install:
	echo "Not yet implemented"

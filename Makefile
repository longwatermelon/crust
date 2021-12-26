SRC=$(wildcard src/*.c)
OBJS=$(notdir $(SRC:.c=.o))

CC=gcc
CFLAGS=-std=gnu17 -ggdb -Wall -Werror

all: hamcc

hamcc: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o hamcc


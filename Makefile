SRC=$(wildcard src/*.c)
OBJS=$(notdir $(SRC:.c=.o))

CC=gcc
CFLAGS=-std=gnu17 -ggdb -Wall -Werror
LDFLAGS=-lm

all: hamcc

hamcc: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

clean:
	rm *.o hamcc


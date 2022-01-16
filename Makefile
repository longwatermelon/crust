SRC=$(wildcard src/*.c)
OBJS=$(notdir $(SRC:.c=.o))

CC=gcc
CFLAGS=-std=gnu17 -ggdb -Wall -Werror
LDFLAGS=-lm

all: crust

crust: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

clean:
	-rm *.o crust


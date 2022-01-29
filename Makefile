SRC=$(wildcard src/*.c)
OBJS=$(notdir $(SRC:.c=.o))

CC=gcc
CFLAGS=-std=gnu17 -ggdb -Wall -Werror -pedantic
LDFLAGS=-lm

LIBSRC=$(wildcard lib/*.crust)
LIBOBJS=$(LIBSRC:.crust=.o)
AR=ar
ARFLAGS=rc

all: crust

crust: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

stdlib: $(LIBOBJS)
	$(AR) $(ARFLAGS) lib/libstdcrust.a $^

lib/%.o: lib/%.crust
	./crust --obj $<

clean:
	-rm *.o crust


CC=gcc
CFLAGS=-O2 -W -Wall -Wextra -std=c99 

all: rshash.o

rshash.o: rshash.c rshash.h
	$(CC) $(CFLAGS) -c -o rshash.o rshash.c

clean:
	rm -rf rshash.o

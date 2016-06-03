CC      = gcc
LDFLAGS = -ljansson -lcbor
OBJDIR  = ./

override CFLAGS  = -std=gnu99 -Wall -Wextra -Werror -O2

all: senml.o

senml.o: senml.c
	$(CC) $(CFLAGS) -c senml.c -o $(OBJDIR)senml.o

clean:
	rm senml.o

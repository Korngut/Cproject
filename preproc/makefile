CC = gcc
CFLAGS = -Wall -Wextra -pedantic -ansi -g

OBJ = preproc.o

all: preproc.o

preproc.o: preproc.c
	$(CC) $(CFLAGS) -c preproc.c -o preproc.o

clean:
	rm -f *.o *.am

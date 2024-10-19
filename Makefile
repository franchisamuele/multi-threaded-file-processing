.PHONY: test1 test2 test3 clean

CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm -lrt -lpthread

SOURCES = functions.c unboundedqueue.c
OBJECTS = $(SOURCES:.c=.o)

all: client server

client: client.o $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

server: server.o $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test1: client server
	./client . 1 & ./server

test2: client server
	./client . 5 & ./server

test3: client server
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client . 5 & 
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./server

clean:
	rm -f *.o client server

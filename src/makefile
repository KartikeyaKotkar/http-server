CC=gcc
CFLAGS=-Wall

all: http-server

http-server: src/main.c src/server.c
	$(CC) $(CFLAGS) src/main.c src/server.c -o http-server

clean:
	rm -f http-server

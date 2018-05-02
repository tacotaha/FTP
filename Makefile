CC=gcc
CFLAGS=-Werror -Wall -std=c99 -pedantic
EXEC=server client

all: client server

server: connect.o stream.o server.o
	$(CC) $(CFLAGS) connect.o stream.o server.o -o server -lpthread

client: connect.o stream.o client.o
	$(CC) $(CFLAGS) connect.o stream.o client.o -o client -lpthread

server.o: Server.c
	$(CC) $(CFLAGS) -c Server.c -o server.o

client.o: Client.c
	$(CC) $(CFLAGS) -c Client.c -o client.o

connect.o: Stream/Connect.c
	$(CC) $(CFLAGS) -c Stream/Connect.c -o connect.o

stream.o: Stream/Stream.c
	$(CC) $(CFLAGS) -c Stream/Stream.c -o stream.o

clean:
	rm -f $(EXEC) *.o *~

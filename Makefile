CC=gcc
CFLAGS=-Wall -Werror -std=c99 -pedantic
EXEC=server client

all: client server

server: connect.o stream.o server.o ftp.o
	$(CC) $(CFLAGS) connect.o stream.o server.o ftp.o -o server -lpthread

client: connect.o stream.o client.o ftp.o
	$(CC) $(CFLAGS) connect.o stream.o client.o ftp.o -o client -lpthread

server.o: Server.c
	$(CC) $(CFLAGS) -c Server.c -o server.o

client.o: Client.c
	$(CC) $(CFLAGS) -c Client.c -o client.o

connect.o: Stream/Connect.c
	$(CC) $(CFLAGS) -c Stream/Connect.c -o connect.o

stream.o: Stream/Stream.c
	$(CC) $(CFLAGS) -c Stream/Stream.c -o stream.o

ftp.o: FTP/FTP.c
	$(CC) $(CFLAGS) -c FTP/FTP.c -o ftp.o

clean:
	rm -f $(EXEC) *.o *~

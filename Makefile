CC=gcc
CFLAGS=-Wall -Werror -std=c99 -pedantic
EXEC=server client
LFLAGS=-lpthread -lm
all: client server

test_server: test_server.c connect.o stream.o ftp.o
	$(CC) $(CFLAGS) test_server.c connect.o stream.o ftp.o -o test_server $(LFLAGS)

test_client: test_client.c connect.o stream.o ftp.o
	$(CC) $(CFLAGS) test_client.c connect.o stream.o ftp.o -o test_client $(LFLAGS)

server: connect.o stream.o server.o ftp.o
	$(CC) $(CFLAGS) connect.o stream.o server.o ftp.o -o server $(LFLAGS)

client: connect.o stream.o client.o ftp.o
	$(CC) $(CFLAGS) connect.o stream.o client.o ftp.o -o client $(LFLAGS)

server.o: Server.c
	$(CC) $(CFLAGS) -c Server.c -o server.o

client.o: Client.c
	$(CC) $(CFLAGS) -c Client.c -o client.o

connect.o: Stream/Connect.c
	$(CC) $(CFLAGS) -c Stream/Connect.c -o connect.o

stream.o: Stream/Stream.c
	$(CC) $(CFLAGS) -c Stream/Stream.c -o stream.o

ftp.o: FTP/FTP.c
	$(CC) $(CFLAGS) -c FTP/FTP.c -o ftp.o -lm

clean:
	rm -f $(EXEC) *.o *~

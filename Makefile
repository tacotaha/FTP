CC=gcc
CFLAGS=-Wall -Werror -std=c99 -pedantic
EXEC=server client
LFLAGS=-lm
all: client server

test_server: test_server.c connect.o ftp.o
	$(CC) $(CFLAGS) test_server.c connect.o ftp.o -o test_server $(LFLAGS)

test_client: test_client.c connect.o ftp.o
	$(CC) $(CFLAGS) test_client.c connect.o ftp.o -o test_client $(LFLAGS)

server: connect.o server.o ftp.o
	$(CC) $(CFLAGS) connect.o server.o ftp.o -o server $(LFLAGS)

client: connect.o client.o ftp.o
	$(CC) $(CFLAGS) connect.o client.o ftp.o -o client $(LFLAGS)

server.o: Server.c
	$(CC) $(CFLAGS) -c Server.c -o server.o

client.o: Client.c
	$(CC) $(CFLAGS) -c Client.c -o client.o

connect.o: Connect/Connect.c
	$(CC) $(CFLAGS) -c Connect/Connect.c -o connect.o

ftp.o: FTP/FTP.c
	$(CC) $(CFLAGS) -c FTP/FTP.c -o ftp.o -lm

clean:
	rm -f $(EXEC) *.o *~

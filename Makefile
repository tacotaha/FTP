CC=gcc
CFLAGS=-Wall -std=c99 -pedantic
EXEC=ftp_server ftp_client

all: client server

server: connect.o stream.o ftp_server.o
	$(CC) $(CFLAGS) connect.o stream.o ftp_server.o -o ftp_server -lpthread

client: connect.o stream.o ftp_client.o
	$(CC) $(CFLAGS) connect.o stream.o ftp_client.o -o ftp_client -lpthread

ftp_server.o: Server/FTP_Server.c
	$(CC) $(CFLAGS) -c Server/FTP_Server.c -o ftp_server.o

ftp_client.o: Client/FTP_Client.c
	$(CC) $(CFLAGS) -c Client/FTP_Client.c -o ftp_client.o

connect.o: Stream/Connect.c
	$(CC) $(CFLAGS) -c Stream/Connect.c -o connect.o

stream.o: Stream/Stream.c
	$(CC) $(CFLAGS) -c Stream/Stream.c -o stream.o

clean:
	rm -f $(EXEC) *.o *~

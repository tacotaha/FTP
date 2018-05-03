#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "../FTP/FTP.h"


void* handle_client(void* args){
  int bytes_read = 0, client_socket = *(int*) args;
  char message[BUF];
  Command c;
  //send(client_socket, welcome, strlen(welcome), 0);
  while( (bytes_read = recv(client_socket,message,sizeof(message),0)) > 0 ){
    message[bytes_read] = 0x0;
    printf("Client sent: %s\n", message);
    get_command(&c, client_socket, 1);
    memset(message, 0, MSG_LEN);
  }
  
  if(bytes_read == 0){
    printf("Client %d disconnected\n", client_socket);
    fflush(stdout);
  } else if(bytes_read == -1)
    perror("recv()");
  
  pthread_exit(NULL);
}

void print_hex(const char* buf, size_t len){
  for(size_t i = 0; i < len - 1; ++i)
    printf("%x-", buf[i]);
  printf("%x\n", buf[len - 1]);
}

void print_banner(void){
  FILE* fp = fopen("ascii_chat.txt","r");
  char c;
  if(fp)
    while((c = fgetc(fp)) != EOF)
      printf("%c",c);
  fclose(fp);
}

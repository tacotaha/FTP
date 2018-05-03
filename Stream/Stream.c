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
  int bytes_read = 0, client_socket = *(int*) args, bytes_sent = 0;
  char message[BUF];
  Command c1;

  if((bytes_sent = send_response("220", welcome_message, client_socket)) <= 0){
    perror("Welcome Message()\n");
    close(client_socket);
    exit(-1);
  }

  /* Ensure the user's login status */
  while(handle_login(client_socket));
  
  while( (bytes_read = recv(client_socket,message,sizeof(message),0)) > 0 ){
    message[bytes_read] = 0x0;
    printf("Client sent: %s\n", message);
    get_command(&c1, client_socket, 1);
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

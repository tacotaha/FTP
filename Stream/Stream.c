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

#define MSG_LEN 1024

void* handle_client(void* args){
  int bytes_read = 0, client_socket = *(int*) args;
  char message[MSG_LEN];
  
  while( (bytes_read = recv(client_socket,message,sizeof(message),0)) > 0 ){
    message[bytes_read] = 0x0;
    printf("Client sent: %s\n", message);
    memset(message, 0, MSG_LEN);
  }
  
  if(bytes_read == 0){
    printf("Client %d disconnected\n", client_socket);
    fflush(stdout);
  } else if(bytes_read == -1)
    perror("recv()");
  
  pthread_exit(NULL);
}

void print_banner(void){
  FILE* fp = fopen("ascii_chat.txt","r");
  char c;
  if(fp)
    while((c = fgetc(fp)) != EOF)
      printf("%c",c);
  fclose(fp);
}

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
#include <limits.h>
#include <assert.h>

#define DEBUG 1
#include "../FTP/FTP.h"


void* handle_client(void* args){
  Server_Arg* server_arg = (Server_Arg*) args;
  int bytes_rcvd = 0, client_socket = server_arg->sockfd,
    bytes_sent = 0, data_sock = INT_MIN, port = server_arg->port;
  char user_name[BUF], *ip = server_arg->ip;
  Command c;
  
  if((bytes_sent = send_response("220", welcome_message, client_socket)) <= 0){
    perror("Welcome Message()\n");
    close(client_socket);
    exit(-1);
  }
  
  /* Verify initial login status */
  while(handle_login(client_socket));
  
  while((bytes_rcvd = get_command(&c, client_socket, DEBUG)) > 0){
    switch(cmd_str_to_enum(c.cmd)){
    case ABOR:
      break;
    case LIST:
      /* Make sure PASV was called at some point before this*/
      assert(data_sock != INT_MIN);
      handle_list(c.arg, client_socket, data_sock);
      break;
    case DELE:
      break;
    case RMD:
      break;
    case MKD:
      break;
    case PWD:
      break;
    case PASS:
      for(size_t i = 0; i < NUM_USERS + 1; ++i)
	if(i == NUM_USERS)
	  send_response("430", "Invalid Password", client_socket);
	else if(strcmp(PASSWORDS[i], c.arg) == 0 && user_name == USERS[i]){
	  send_response("230", "User logged in, proceed.", client_socket);
	  break;
	}
      break;
    case PORT:
      break;
    case QUIT:
      break;
    case RETR:
      break;
    case STOR:
      break;
    case SYST:
      break;
    case TYPE:
      break;
    case USER:
      for(size_t i = 0; i < NUM_USERS + 1; ++i)
	if(i == NUM_USERS)
	  send_response("430", "Invalid User name", client_socket);
	else if(strcmp(USERS[i], c.arg) == 0){
          strcpy(user_name, USERS[i]);
	  send_response("331", "User name okay, need password", client_socket);
	  break;
	}
      break;
    case PASV:
      data_sock = handle_pasv(port, ip, client_socket);
      break;
    default:
      break;
    }
  }
  
  
  if(bytes_rcvd == 0){
    printf("Client %d disconnected\n", client_socket);
    fflush(stdout);
  } else if(bytes_rcvd == -1)
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

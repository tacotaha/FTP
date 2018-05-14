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
  char user_name[BUF], *ip = server_arg->ip, cwd[BUF], response[BUF2];
  Command c;

  memset(cwd, 0, sizeof(cwd));
  getcwd(cwd, sizeof(cwd));
  
  if((bytes_sent = send_response("220", welcome_message, client_socket)) <= 0){
    perror("Welcome Message()\n");
    close(client_socket);
    exit(-1);
  }
  
  /* Verify initial login status */
  while(handle_login(client_socket));
  
  while((bytes_rcvd = get_command(&c, client_socket, DEBUG)) > 0){
    memset(response, 0, sizeof(response));
    switch(cmd_str_to_enum(c.cmd)){
    case LIST:
      assert(data_sock != INT_MIN);
      handle_list(c.arg, client_socket, data_sock);
      break;
    case DELE:
      handle_delete(c.arg, client_socket);
      break;
    case RMD:
      handle_rm(c.arg, client_socket);
      break;
    case MKD:
      handle_mkdir(c.arg, client_socket);
      break;
    case CWD:
      assert(handle_cwd(c.arg, client_socket) > 0);
      break;
    case PWD:
      assert(handle_pwd(cwd, client_socket) > 0);
      break;
    case PASS:
      assert(handle_pass(c.arg, user_name, client_socket) > 0);
      break;
    case RETR:
      assert(data_sock != INT_MIN);
      handle_retr(c.arg, client_socket, data_sock);
      break;
    case STOR:
      assert(data_sock != INT_MIN);
      handle_stor(c.arg, client_socket, data_sock);
      break;
    case USER:
      assert(handle_user(c.arg, user_name, client_socket) > 0);
      break;
    case PASV:
      data_sock = handle_pasv(port, ip, client_socket);
      break;
    default:
      break;
    }
    memset(&c, 0, sizeof(Command));
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

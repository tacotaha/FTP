#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>

#include "Connect/Connect.h"
#include "FTP/FTP.h"

#define DEBUG 1

int main(int argc, char* argv[]){
  int server_socket, client_socket,
    client_count = 0, port = PORT_NO,
    bytes_rcvd = 0, bytes_sent = 0,
    data_sock = INT_MIN;
  char out_buffer[BUF], in_buffer[BUF],
    user_name[BUF], cwd[BUF], response[BUF2], *ip = IP;
  struct sockaddr_in server;
  Command c;

  for(int i = 1; i < argc; i += 2)
    if(!strcmp(argv[i],"-i") && argc >= i + 1)
      ip = argv[i + 1];
    else if(!strcmp(argv[i],"-p") && argc >= i + 1)
      port = atoi(argv[i + 1]);
    else{
      printf("Usage: %s [-i] ip_addr [-p] port [-c]\n", argv[0]);
      printf("Default: ip = 127.0.0.0.1, port = 4444\n");
      exit(0);
    }
  
  memset(&server_socket,0x0,sizeof(server_socket));
  memset(in_buffer,0x0,sizeof(in_buffer));
  memset(out_buffer,0x0, sizeof(out_buffer));
  memset(cwd, 0, sizeof(cwd));
  getcwd(cwd, sizeof(cwd));
  
  server_socket = create_socket();
  server = create_socket_address(port, ip);
  bind_connection(server_socket,(struct sockaddr*)&server);
  listen_for_connection(server_socket, BACKLOG);
  socklen_t sckln = sizeof(struct sockaddr_in);

  /*=======================================Main Loop========================================*/
  while( (client_socket = accept(server_socket,(struct sockaddr *)&server,&sckln))){
    
    ++client_count;
    
    if(fork() == 0){
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
    }
  }
  
  close(server_socket);
  
  memset(out_buffer,0x0, sizeof(out_buffer));
  memset(in_buffer,0x0,sizeof(in_buffer));
  fflush(stdout);
  
  printf("[-] Closing the connection\n");
  close(server_socket);
  return 0;
}



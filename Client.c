#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>

#include "FTP/FTP.h"
#include "Stream/Connect.h"
#include "Stream/Stream.h"

#define DEBUG 1

int main(int argc, char* argv[]){
  int client_socket, port = PORT_NO, response = 0, data_sockfd = INT_MIN;
  struct sockaddr_in server_addr;
  char buffer[BUF], user_name[BUF], *ip = IP,
    password[BUF], *input, *param, *cmd;
  Command c, c2;
  
  memset(&client_socket,0,sizeof(client_socket));
  memset(&c, 0, sizeof(c));
  memset(buffer, 0, sizeof(buffer));
  client_socket = create_socket();
  server_addr = create_socket_address(port, ip);
  connect_to_server(client_socket,(struct sockaddr*)&server_addr);
  
  response = get_response(buffer, sizeof(buffer), client_socket, DEBUG);
  assert(response == 220);
  
  while(response != 230){
    do{
      printf("Username: ");
      fgets(user_name, sizeof(user_name), stdin);
      char* user = strtok(user_name, "\n");
      build_command(&c, "USER", user);
      if(send_command(&c, client_socket) < 0){
	perror("USER_NAME: send_command()");
	exit(1);
      }
      memset(buffer, 0, sizeof(buffer));
    }while(get_response(buffer, sizeof(buffer), client_socket, DEBUG) != 331);
    memset(&c, 0, sizeof(c));
    printf("Password: ");
    fgets(password, sizeof(password), stdin);
    char* pass = strtok(password, "\n");
    build_command(&c, "PASS", pass);
    if(send_command(&c, client_socket) < 0){
      perror("Password: send_command()");
      exit(1);
    }
    memset(buffer, 0, sizeof(buffer));
    response = get_response(buffer, sizeof(buffer), client_socket, DEBUG);
    memset(buffer, 0, sizeof(buffer));
  }

  assert(response == 230);

  do{
    memset(&c, 0, sizeof(c));
    memset(&c2, 0, sizeof(c2));
    printf("FTP > ");
    fgets(buffer, sizeof(buffer), stdin);

    input = strtok(buffer, "\n");
    
    if(input[0] == '!'){
      system(input + 1);
      continue;
    }
    
    cmd = strtok(input, " ");
    param = strtok(NULL, " ");
    
    switch(user_cmd_str_to_enum(cmd)){
    case LS:
      if(data_sockfd == INT_MIN){
	memset(&c, 0, sizeof(c));
	build_command(&c, "PASV", "");
	send_command(&c, client_socket);
	data_sockfd = data_port_connect(client_socket, ip);
      }
      if(param == NULL)
	handle_ls(".", client_socket, data_sockfd);
      else
	handle_ls(param, client_socket, data_sockfd);
      break;
    case CD:
      break;
    case DELETE:
      break;
    case GET:
      break;
    case HELP:
      break;
    case MKDIR:
      break;
    case PUT:
      break;
    case PWD_:
      break;
    case QUIT_:
      break;
    case RMDIR:
      break;
    default:
      break;
    }
  }while(strcmp(input, "exit") != 0);
  
  close(client_socket);
  return 0;
}

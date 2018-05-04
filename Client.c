#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "FTP/FTP.h"
#include "Stream/Connect.h"
#include "Stream/Stream.h"

#define DEBUG 1

int main(int argc, char* argv[]){
  int client_socket, port = PORT_NO, bytes_sent, response = 0;
  struct sockaddr_in server_addr;
  char buffer[BUF], user_name[BUF], *ip = IP, password[BUF];
  Command c, c2;
  
  memset(&client_socket,0,sizeof(client_socket));
  memset(&c, 0, sizeof(c));
  memset(buffer, 0, sizeof(buffer));
  client_socket = create_socket();
  server_addr = create_socket_address(port, ip);
  connect_to_server(client_socket,(struct sockaddr*)&server_addr);
  
  /* Should be initial connect message*/
  response = get_response(buffer, sizeof(buffer), client_socket, DEBUG);
  assert(response == 220);
  
  while(response != 230){
    /* Get Username */
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
    
    /* Get Password */
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

  /* User should be logged in beyond this point, begin main loop */
  assert(response == 230);
  
  do{
    memset(&c, 0, sizeof(c));
    memset(&c2, 0, sizeof(c2));
    printf("FTP > ");
    read_command(&c, stdin);
    bytes_sent = send_command(&c, client_socket);
    get_command(&c2, client_socket, 1);
  }while(strcmp(buffer, "exit") != 0 && bytes_sent > 0);
  
  close(client_socket);
  return 0;
}

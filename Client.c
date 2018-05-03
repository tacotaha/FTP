#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FTP/FTP.h"
#include "Stream/Connect.h"
#include "Stream/Stream.h"

int main(int argc, char* argv[]){
  int client_socket, port = PORT, bytes_sent;
  struct sockaddr_in server_addr;
  char buffer[BUF], *ip = IP;
  Command c;
  
  memset(&client_socket,0,sizeof(client_socket));  
  client_socket = create_socket();
  server_addr = create_socket_address(port, ip);
  connect_to_server(client_socket,(struct sockaddr*)&server_addr);

  do{
    printf("FTP> ");
    read_command(&c, stdin);
    bytes_sent = send_command(&c, client_socket);
  }while(strcmp(buffer, "exit") != 0 && bytes_sent > 0);
  
  close(client_socket);
  return 0;
}

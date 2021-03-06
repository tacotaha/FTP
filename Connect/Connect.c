#include "Connect.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_socket(void) {
  int option = 1;
  int sock = socket(AF_INET, SOCK_STREAM, 0);

  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  if (sock < 0) {
    perror("create_socket");
    exit(1);
  }

  return sock;
}

struct sockaddr_in create_socket_address(int port, const char* ip_addr) {
  struct sockaddr_in sock;
  sock.sin_family = AF_INET;
  sock.sin_port = htons(port);
  sock.sin_addr.s_addr = inet_addr(ip_addr);
  return sock;
}

int bind_connection(int socket, struct sockaddr* sa) {
  int status = bind(socket, sa, sizeof(*sa));
  if (status < 0) printf("[-] Failed to bind address to socket.\n");
  return status;
}

int listen_for_connection(int listener_socket, int backlog) {
  int status = listen(listener_socket, backlog);
  if (status != 0) printf("[+] Failed to listen for connection\n");
  return status;
}

int accept_connection_from_client(int server_socket, struct sockaddr* client,
                                  socklen_t* addr_size) {
  int client_socket =
      accept(server_socket, (struct sockaddr*)&client, addr_size);
  if (client_socket < 0) {
    perror("accept_connection_from_client");
    exit(1);
  }
  return client_socket;
}

int connect_to_server(int client_socket, struct sockaddr* server) {
  int status = connect(client_socket, server, sizeof(*server));
  if (status != 0) printf("[-] Address unsuccessfully bound to socket.\n");
  return status;
}

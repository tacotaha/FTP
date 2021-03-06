#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Connect/Connect.h"
#include "FTP/FTP.h"
#define DEBUG 1

int main(int argc, char* argv[]) {
  int client_socket, port = PORT_NO, response = 0, data_sockfd = INT_MIN;
  struct sockaddr_in server_addr;
  char buffer[BUF], user_name[BUF], *ip = IP, password[BUF], *input, *param,
                                    *cmd;

  for (int i = 1; i < argc; i += 2)
    if (!strcmp(argv[i], "-i") && argc >= i + 1)
      ip = argv[i + 1];
    else if (!strcmp(argv[i], "-p") && argc >= i + 1)
      port = atoi(argv[i + 1]);
    else {
      printf("Usage: %s [-i] ip_addr [-p] port\n", argv[0]);
      printf("Default: ip = 127.0.0.0.1, port = 4444\n");
      exit(0);
    }

  memset(&client_socket, 0, sizeof(client_socket));
  memset(buffer, 0, sizeof(buffer));
  client_socket = create_socket();
  server_addr = create_socket_address(port, ip);
  connect_to_server(client_socket, (struct sockaddr*)&server_addr);

  response = get_response(buffer, client_socket, DEBUG);
  assert(response == 220);

  while (response != 230) {
    do {
      printf("Username: ");
      fgets(user_name, sizeof(user_name), stdin);
      char* user = strtok(user_name, "\n");
      assert(send_command("USER", user, client_socket) > 0);
      memset(buffer, 0, sizeof(buffer));
    } while (get_response(buffer, client_socket, DEBUG) != 331);
    printf("Password: ");
    fgets(password, sizeof(password), stdin);
    char* pass = strtok(password, "\n");
    assert(send_command("PASS", pass, client_socket) > 0);
    memset(buffer, 0, sizeof(buffer));
    response = get_response(buffer, client_socket, DEBUG);
  }

  do {
    printf("FTP > ");
    fgets(buffer, sizeof(buffer), stdin);

    input = strtok(buffer, "\n");

    if (input[0] == '!') {
      system(input + 1);
      continue;
    }

    cmd = strtok(input, " ");
    param = strtok(NULL, " ");

    switch (user_cmd_str_to_enum(cmd)) {
      case LS:
        send_passive(client_socket, &data_sockfd, ip);
        if (param == NULL)
          handle_ls(".", client_socket, data_sockfd);
        else
          handle_ls(param, client_socket, data_sockfd);
        break;
      case CD:
        assert(send_command("CWD", (param == NULL ? getenv("HOME") : param),
                            client_socket) > 0);
        get_response(buffer, client_socket, DEBUG);
        break;
      case DELETE:
        if (param == NULL) break;
        assert(send_command("DELE", param, client_socket) > 0);
        get_response(buffer, client_socket, DEBUG);
        break;
      case GET:
        if (param == NULL) break;
        send_passive(client_socket, &data_sockfd, ip);
        handle_get(param, client_socket, data_sockfd);
        break;
      case HELP:
        handle_help();
        break;
      case MKDIR:
        if (param == NULL) break;
        assert(send_command("MKD", param, client_socket) > 0);
        get_response(buffer, client_socket, DEBUG);
        break;
      case PUT:
        if (param == NULL) break;
        send_passive(client_socket, &data_sockfd, ip);
        handle_put(param, client_socket, data_sockfd);
        break;
      case PWD_:
        assert(send_command("PWD", "", client_socket) > 0);
        get_response(buffer, client_socket, DEBUG);
        break;
      case QUIT_:
        printf("Exiting...Goodbye Now!\n");
        exit(0);
        break;
      case RMDIR:
        if (param == NULL) break;
        assert(send_command("RMD", param, client_socket) > 0);
        get_response(buffer, client_socket, DEBUG);
        break;
      default:
        printf("Invalid Command. See help for more information.\n");
        break;
    }
    memset(buffer, 0, sizeof(buffer));
  } while (1);

  close(client_socket);
  return 0;
}

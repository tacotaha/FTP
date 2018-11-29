#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <time.h>

#include "FTP.h"

const char* USERS[NUM_USERS] = {"Euclid", "Newton", "Gauss", "Euler", "test"};
const char* PASSWORDS[NUM_USERS] = {"geometry", "calculus", "electrostatics",
                                    "konigsberg", "test"};
const char* welcome_message =
    "-=(<*>)=-.:. (( Welcome to The FTP server)) .:.-=(<*>)=-";
const char* COMMAND_STRING[NUM_COMMANDS] = {
    "LIST", /* List file and directories */
    "DELE", /* Delete a file */
    "RMD",  /* Remove a directory */
    "MKD",  /* Create a directory */
    "CWD",  /* Change working directory */
    "PWD",  /* Print working directory */
    "PASS", /* Send password (NOTE: THE FTP USES PLAINTEXT PASSWORDS)*/
    "QUIT", /* Log off of server */
    "RETR", /* Retrieve a file */
    "STOR", /* Send or put a file */
    "USER", /* Send Username */
    "PASV"  /* Passive mode*/
};
const char* USER_CMD_STRING[NUM_USER_CMDS] = {
    "ls", "cd", "delete", "get", "help", "mkdir", "put", "pwd", "quit", "rm"};

int get_response(char* buffer, int sockfd, int print) {
  int response = 0;
  if (recv(sockfd, buffer, PACKET_LEN, 0) > 0) {
    buffer[PACKET_LEN] = 0;
    if (print) printf("%s\n", buffer);
    for (size_t i = 0; i < 3; ++i)
      response += (buffer[i] - 0x30) * pow(10, 2 - i);
  }
  return response;
}

int send_command(const char* cmnd, const char* arg, int sockfd) {
  Command c;
  build_command(&c, cmnd, arg);
  char cmd[BUF];
  sprintf(cmd, "%s %s\r\n", c.cmd, c.arg);
  return send(sockfd, cmd, PACKET_LEN, 0);
}

void build_command(Command* command, const char* cmd, const char* arg) {
  size_t i = 0;
  memset(command, 0, sizeof(Command));

  for (i = 0; i < strlen(cmd) && i < CMD_LEN; ++i) command->cmd[i] = cmd[i];
  command->cmd[i] = 0x0;

  for (i = 0; i < strlen(arg) && i < ARG_LEN; ++i) command->arg[i] = arg[i];
  command->arg[i] = 0x0;
}

int send_response(const char* status, const char* msg, int sockfd) {
  const size_t cmd_length = strlen(status) + strlen(msg) + 1;
  char cmd[cmd_length];
  sprintf(cmd, "%s %s\r\n", status, msg);
  int bytes_sent = send(sockfd, cmd, PACKET_LEN, 0);
  return bytes_sent;
}

int read_command(Command* command, FILE* fp) {
  char buffer[MSG_LEN + 1], *token;
  fgets(buffer, MSG_LEN + 1, fp);

  token = strtok(buffer, " ");
  if (strlen(token) > CMD_LEN) token[CMD_LEN] = 0;
  strcpy(command->cmd, token);

  if ((token = strtok(NULL, " ")) != NULL) {
    token = strtok(token, "\n");
    if (strlen(token) > ARG_LEN) token[ARG_LEN] = 0;
    strcpy(command->arg, token);
  }

  return strlen(command->cmd) + strlen(command->arg);
}

int get_command(Command* command, int sockfd, int print) {
  char buffer[MSG_LEN], *token;
  int bytes_rcvd = recv(sockfd, buffer, PACKET_LEN, 0);
  if (bytes_rcvd > 0) {
    buffer[bytes_rcvd] = 0;
    if (print) printf("%s\n", buffer);
    token = strtok(buffer, " ");
    if (strlen(token) > CMD_LEN) token[CMD_LEN] = 0;
    strcpy(command->cmd, token);
    if ((token = strtok(NULL, " ")) != NULL &&
        (token = strtok(token, "\r")) != NULL) {
      if (strlen(token) > ARG_LEN) token[ARG_LEN] = 0;
      strcpy(command->arg, token);
    }
  }

  return bytes_rcvd;
}

COMMAND_ENUM cmd_str_to_enum(const char* cmd_str) {
  for (size_t i = 0; i < NUM_COMMANDS; ++i)
    if (strcmp(cmd_str, COMMAND_STRING[i]) == 0) return i;
  return -1;
}

const char* cmd_enum_to_str(COMMAND_ENUM cmd_enum) {
  if (cmd_enum > 0 && cmd_enum < NUM_COMMANDS) return COMMAND_STRING[cmd_enum];
  return NULL;
}

int handle_login(int sockfd) {
  Command c;

  get_command(&c, sockfd, 1);
  while ((strcmp(c.cmd, "USER") != 0)) {
    send_response("332", "Need account for login", sockfd);
    get_command(&c, sockfd, 1);
  }

  for (size_t i = 0; i < NUM_USERS + 1; ++i)
    if (i == NUM_USERS) {
      send_response("430", "Invalid User name", sockfd);
      return 1;
    } else if (strcmp(USERS[i], c.arg) == 0) {
      send_response("331", "User name okay, need password", sockfd);
      break;
    }

  get_command(&c, sockfd, 1);
  while ((strcmp(c.cmd, "PASS") != 0)) {
    send_response("331", "User name okay, need password", sockfd);
    get_command(&c, sockfd, 1);
  }

  for (size_t i = 0; i < NUM_USERS + 1; ++i)
    if (i == NUM_USERS) {
      send_response("430", "Invalid Password", sockfd);
      return 1;
    } else if (strcmp(PASSWORDS[i], c.arg) == 0) {
      send_response("230", "User logged in, proceed.", sockfd);
      break;
    }

  return 0;
}

const char* user_cmd_enum_to_str(USER_CMD_ENUM cmd_enum) {
  if (cmd_enum > 0 && cmd_enum < NUM_USER_CMDS)
    return USER_CMD_STRING[cmd_enum];
  return NULL;
}

USER_CMD_ENUM user_cmd_str_to_enum(char* cmd_str) {
  if (cmd_str != NULL) {
    for (size_t i = 0; i < NUM_USER_CMDS; ++i)
      if (strcmp(cmd_str, USER_CMD_STRING[i]) == 0) return i;
  }
  return -1;
}

int handle_list(const char* arg, int sockfd, int data_sockfd) {
  int status = -1, byte_count = 0;
  DIR* dr;
  struct dirent* de;
  char buffer[BUF];

  memset(buffer, 0, sizeof(buffer));

  if (arg == NULL || (dr = opendir(arg)) == NULL) {
    send_response("451", "Requested action aborted. Local error in processing.",
                  sockfd);
    return -1;
  }

  while ((de = readdir(dr)) != NULL) byte_count += (strlen(de->d_name) + 1);

  closedir(dr);

  sprintf(buffer, "ASCII data connection for /bin/ls (%d bytes)", byte_count);
  assert(send_response("150", buffer, sockfd) >= 0);

  dr = opendir(arg);

  while ((de = readdir(dr)) != NULL) {
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s\n", de->d_name);
    if ((status = send(data_sockfd, buffer, strlen(buffer), 0)) < 0) {
      send_response("451",
                    "Requested action aborted. Local error in processing.",
                    sockfd);
      return -1;
    }
  }

  closedir(dr);

  if (status > 0)
    send_response("226", "ASCII Transfer Complete", sockfd);
  else
    send_response("550", "Requested action not taken.", sockfd);

  return status;
}

int handle_ls(const char* arg, int client_socket, int data_socket) {
  int byte_count = 0, bytes_rcvd = 0, status = 0;
  char buffer[BUF], *ptr;

  memset(buffer, 0, sizeof(buffer));
  assert(send_command("LIST", arg, client_socket) > 0);
  status = get_response(buffer, client_socket, 1);

  if (status == 451) return -1;

  printf("\n");

  ptr = strtok(buffer, "(");
  ptr = strtok(NULL, "(");
  ptr = strtok(ptr, " ");

  assert(ptr != NULL);

  for (size_t i = 0; i < strlen(ptr); ++i)
    byte_count += (ptr[i] - 0x30) * pow(10, strlen(ptr) - i - 1);

  memset(buffer, 0, sizeof(buffer));

  while (bytes_rcvd < byte_count &&
         (status = recv(data_socket, buffer, sizeof(buffer), 0)) > 0) {
    buffer[sizeof(buffer)] = 0;
    printf("%s", buffer);
    memset(buffer, 0, sizeof(buffer));
    bytes_rcvd += status;
  }

  return get_response(buffer, client_socket, 1);
}

int handle_pasv(int cmd_port, const char* ip, int sockfd) {
  int data_socket = 0, client_socket, data_port = cmd_port + 1, status = 0;
  char buffer[BUF];
  struct sockaddr_in server_addr;
  socklen_t sckln = sizeof(struct sockaddr_in);

  memset(buffer, 0, BUF);

  data_socket = create_socket();
  server_addr = create_socket_address(data_port, ip);
  while (bind_connection(data_socket, (struct sockaddr*)&server_addr) < 0)
    server_addr = create_socket_address(++data_port, ip);

  sprintf(buffer, "Entering Passive Mode (%d)", data_port);
  status = send_response("227", buffer, sockfd);

  if (status < 0) {
    perror("send_data_port()\n");
    exit(1);
  }

  listen_for_connection(data_socket, BACKLOG);
  client_socket = accept(data_socket, (struct sockaddr*)&server_addr, &sckln);

  return client_socket;
}

int data_port_connect(int sockfd, char* ip) {
  int data_port = 0, data_socket = 0;
  char buffer[PACKET_LEN], *ptr;
  struct sockaddr_in server_addr;

  memset(buffer, 0, sizeof(buffer));

  if (recv(sockfd, buffer, PACKET_LEN, 0) < 0) {
    perror("data_port_connect()\n");
    exit(1);
  }

  buffer[PACKET_LEN] = 0;

  ptr = strtok(buffer, "(");
  ptr = strtok(NULL, "(");
  ptr = strtok(ptr, ")");

  assert(ptr != NULL);

  for (size_t i = 0; i < strlen(ptr); ++i)
    data_port += (ptr[i] - 0x30) * pow(10, strlen(ptr) - i - 1);

  data_socket = create_socket();
  server_addr = create_socket_address(data_port, ip);
  if ((connect_to_server(data_socket, (struct sockaddr*)&server_addr)) < 0) {
    perror("data_port_connect()\n");
    exit(1);
  }

  return data_socket;
}

int handle_rm(const char* arg, int sockfd) {
  struct stat statbuf;
  char buffer[BUF];

  memset(buffer, 0, sizeof(buffer));
  sprintf(buffer, "rm -r %s", arg);

  if (stat(arg, &statbuf) == 0 && system(buffer) == 0)
    if (S_ISDIR(statbuf.st_mode))
      return send_response("250", "The directory was successfully removed",
                           sockfd);

  return send_response("550", "Can't remove directory: No such directory",
                       sockfd);
}

int handle_delete(const char* arg, int sockfd) {
  struct stat statbuf;
  char buffer[BUF];

  memset(buffer, 0, sizeof(buffer));
  sprintf(buffer, "rm %s", arg);

  if (stat(arg, &statbuf) == 0 && system(buffer) == 0)
    if (S_ISREG(statbuf.st_mode)) {
      memset(buffer, 0, sizeof(buffer));
      sprintf(buffer, "Deleted %s", arg);
      return send_response("250", buffer, sockfd);
    }

  memset(buffer, 0, sizeof(buffer));
  sprintf(buffer, "Could not delete %s: No such file", arg);
  return send_response("550", buffer, sockfd);
}

int handle_mkdir(const char* arg, int sockfd) {
  struct stat statbuf;
  char buffer[BUF];

  memset(buffer, 0, sizeof(buffer));

  if (stat(arg, &statbuf) == -1) {
    mkdir(arg, 0700);
    return send_response("257", "The directory was successfully created",
                         sockfd);
  }

  return send_response("550", "Can't create directory: File exists", sockfd);
}

int handle_retr(const char* arg, int client_socket, int data_socket) {
  struct stat statbuf;
  char buffer[PACKET_LEN];
  int fsize = 0, bytes_sent = 0, transferred = 0, bytes_left = 0, fd = 0;
  off_t offset = 0;

  memset(buffer, 0, sizeof(buffer));

  if (stat(arg, &statbuf) == 0) {
    if (S_ISDIR(statbuf.st_mode))
      return send_response("550", "I can only retrieve regular files",
                           client_socket);

    fd = open(arg, O_RDONLY);
    assert(fd != -1);

    assert(fstat(fd, &statbuf) >= 0);
    fsize = bytes_left = statbuf.st_size;

    sprintf(buffer, "Starting file transfer (%d bytes)", fsize);
    send_response("200", buffer, client_socket);

    while (bytes_left > 0 &&
           (bytes_sent = sendfile(data_socket, fd, &offset, PACKET_LEN)) > 0) {
      transferred += bytes_sent;
      bytes_left -= bytes_sent;
      printf("[%lf%c] Sent %d bytes...\n", (double)100 * transferred / fsize,
             '%', transferred);
    }

    close(fd);
    return send_response("200", "File transfer complete.", client_socket);
  }

  sprintf(buffer, "Can't open %s. No such file.", arg);
  return send_response("550", buffer, client_socket);
}

int handle_get(const char* arg, int client_socket, int data_socket) {
  char buffer[PACKET_LEN], *ptr;
  int file_size = 0, bytes_rcvd = 0, bytes_left = 0, response = 0;
  clock_t begin, end;
  double download_time = 0;
  FILE* fp;

  memset(buffer, 0, sizeof(buffer));
  assert(send_command("RETR", arg, client_socket) > 0);
  response = get_response(buffer, client_socket, 1);

  if (response == 200) {
    ptr = strtok(buffer, "(");
    ptr = strtok(NULL, "(");
    ptr = strtok(ptr, ")");
    assert(ptr != NULL);

    file_size = bytes_left = atoi(ptr);
    fp = fopen(arg, "w");
    assert(fp != NULL);

    begin = clock();
    while (bytes_left > 0 &&
           (bytes_rcvd = recv(data_socket, buffer, PACKET_LEN, 0)) > 0) {
      fwrite(buffer, 1, bytes_rcvd, fp);
      bytes_left -= bytes_rcvd;
    }
    end = clock();

    download_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Download time: %lf seconds --> %lf Kbytes/s\n", download_time,
           file_size / 1024 / download_time);

    fclose(fp);
    return get_response(buffer, client_socket, 1);
  }

  return response;
}

int send_passive(int client_socket, int* data_socket, char* ip) {
  if (*data_socket == INT_MIN) {
    *data_socket = 0;
    assert(send_command("PASV", "", client_socket) > 0);
    *data_socket = data_port_connect(client_socket, ip);
  }
  return *data_socket;
}

int handle_put(char* arg, int client_socket, int data_socket) {
  struct stat statbuf;
  char buffer[PACKET_LEN];
  int fsize = 0, bytes_sent = 0, transferred = 0, bytes_left = 0, fd = 0;
  off_t offset = 0;
  clock_t begin, end;
  double upload_time = 0;

  memset(buffer, 0, sizeof(buffer));

  if (stat(arg, &statbuf) == 0) {
    if (S_ISDIR(statbuf.st_mode)) {
      printf("%s: Not a plain file\n", arg);
      return -1;
    }

    fd = open(arg, O_RDONLY);
    assert(fd != -1);

    assert(fstat(fd, &statbuf) >= 0);
    fsize = bytes_left = statbuf.st_size;

    sprintf(buffer, "%s:(%d:bytes)", arg, fsize);
    assert(send_command("STOR", buffer, client_socket) > 0);

    memset(buffer, 0, sizeof(buffer));
    get_response(buffer, client_socket, 1);

    begin = clock();
    while (bytes_left > 0 &&
           (bytes_sent = sendfile(data_socket, fd, &offset, PACKET_LEN)) > 0) {
      transferred += bytes_sent;
      bytes_left -= bytes_sent;
    }
    end = clock();

    upload_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Upload time: %lf seconds --> %lf Kbytes/s\n", upload_time,
           fsize / 1024 / upload_time);
    return get_response(buffer, client_socket, 1);
  }

  printf("Can't open %s: No such file.\n", arg);
  return -1;
}

int handle_stor(char* arg, int client_socket, int data_socket) {
  char buffer[PACKET_LEN], *ptr;
  int file_size = 0, bytes_rcvd = 0, bytes_left = 0, response = 0;
  clock_t begin, end;
  double download_time;
  FILE* fp;

  memset(buffer, 0, sizeof(buffer));
  strcpy(buffer, arg);

  ptr = strtok(buffer, " ");
  ptr = strtok(ptr, ":");
  assert(ptr != NULL);

  fp = fopen(ptr, "w");
  assert(fp != NULL);

  ptr = strtok(arg, "(");
  ptr = strtok(NULL, "(");
  ptr = strtok(ptr, ":");
  assert(ptr != NULL);

  file_size = bytes_left = atoi(ptr);
  send_response("200", "Ready for file transfer.", client_socket);

  begin = clock();
  while (bytes_left > 0 &&
         (bytes_rcvd = recv(data_socket, buffer, PACKET_LEN, 0)) > 0) {
    fwrite(buffer, 1, bytes_rcvd, fp);
    bytes_left -= bytes_rcvd;
    printf("[%lf%c] Received %d bytes...\n",
           100 * (1 - (double)bytes_left / file_size), '%', bytes_rcvd);
  }
  end = clock();

  download_time = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Download time: %lf seconds --> %lf Kbytes/s\n", download_time,
         file_size / 1024 / download_time);
  send_response("226", "File successfully transferred.", client_socket);
  fclose(fp);

  return response;
}

int handle_cwd(const char* arg, int client_socket) {
  char response[BUF], cwd[PACKET_LEN];

  if (chdir(arg) < 0) {
    sprintf(response,
            "Can't change directory to: \"%s\": No such file or directory",
            arg);
    return send_response("550", response, client_socket);
  } else {
    memset(cwd, 0, sizeof(cwd));
    getcwd(cwd, sizeof(cwd));

    sprintf(response, "OK. Current directory is: \"%s\"", cwd);
    return send_response("250", response, client_socket);
  }

  return -1;
}

int handle_pwd(const char* arg, int client_socket) {
  char response[BUF];
  memset(response, 0, sizeof(response));
  sprintf(response, "\"%s\" is your current location.", arg);
  return send_response("257", response, client_socket);
}

int handle_pass(const char* arg, const char* user_name, int client_socket) {
  for (size_t i = 0; i < NUM_USERS + 1; ++i)
    if (i == NUM_USERS)
      return send_response("430", "Invalid Password", client_socket);
    else if (strcmp(PASSWORDS[i], arg) == 0 && user_name == USERS[i]) {
      return send_response("230", "User logged in, proceed.", client_socket);
      break;
    }
  return -1;
}

int handle_user(const char* arg, char* user_name, int client_socket) {
  for (size_t i = 0; i < NUM_USERS + 1; ++i)
    if (i == NUM_USERS)
      return send_response("430", "Invalid User name", client_socket);
    else if (strcmp(USERS[i], arg) == 0) {
      strcpy(user_name, USERS[i]);
      return send_response("331", "User name okay, need password",
                           client_socket);
    }
  return -1;
}

void handle_help(void) {
  for (size_t i = 0; i < NUM_USER_CMDS; ++i) printf("%s\n", USER_CMD_STRING[i]);
}

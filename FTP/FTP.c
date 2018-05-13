#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <dirent.h>
#include <limits.h>
#include <assert.h>
#include <sys/stat.h>

#include "../Stream/Stream.h"
#include "../Stream/Connect.h"
#include "FTP.h"

const char* USERS[NUM_USERS] = {"Euclid", "Newton", "Gauss", "Euler", "Hilbert"};
const char* PASSWORDS[NUM_USERS] = {"geometry", "calculus", "electrostatics", "konigsberg", "spaces"};
const char* welcome_message = "-=(<*>)=-.:. (( Welcome to The FTP server)) .:.-=(<*>)=-";
const char* COMMAND_STRING[NUM_COMMANDS] = {
  "ABOR", /* Abort previous FTP command */
  "LIST", /* List file and directories */
  "DELE", /* Delete a file */
  "RMD",  /* Remove a directory */
  "MKD",  /* Create a directory */
  "CWD",  /* Change working directory */
  "PWD",  /* Print working directory */
  "PASS", /* Send password (NOTE: THE FTP USES PLAINTEXT PASSWORDS)*/
  "PORT", /* Request open port on specific IP addr/Port No */
  "QUIT", /* Log off of server */
  "RETR", /* Retrieve a file */
  "STOR", /* Send or put a file */
  "SYST", /* Identify system type */
  "TYPE", /* Specify type (A for ASCII, I for binary) */
  "USER", /* Send Username */
  "PASV"  /* Passive mode*/
};

const char* USER_CMD_STRING[NUM_USER_CMDS] = {
  "ls",
  "cd",
  "delete",
  "get",
  "help",
  "mkdir",
  "put",
  "pwd",
  "quit",
  "rm"
};

int get_response(char* buffer, int sockfd, int print){
  int  response = 0;
  if(recv(sockfd, buffer, PACKET_LEN, 0) > 0){
    buffer[PACKET_LEN] = 0;
    if(print)
      printf("%s\n", buffer);
    for(size_t i = 0; i < 3; ++i)
      response += (buffer[i] - 0x30) * pow(10, 2 - i);
  }
  return response;
}

int send_command(const Command* command, int sockfd){
  const size_t cmd_length = CMD_LEN + strlen(command->arg);
  char cmd[cmd_length];
  sprintf(cmd, "%s %s\r\n", command->cmd, command->arg);
  int bytes_sent = send(sockfd, cmd, PACKET_LEN, 0);
  return bytes_sent;
}

void build_command(Command* command, char* cmd, char* arg){
  size_t i = 0;
  
  for(i = 0; i < strlen(cmd) && i < CMD_LEN; ++i)
    command->cmd[i] = cmd[i];
  command->cmd[i] = 0x0;

  for(i = 0; i < strlen(arg) && i < ARG_LEN; ++i)
    command->arg[i] = arg[i];
  command->arg[i] = 0x0;
}

int send_response(const char* status, const char* msg, int sockfd){
  const size_t cmd_length = strlen(status) + strlen(msg) + 1;
  char cmd[cmd_length];
  sprintf(cmd, "%s %s\r\n", status, msg);
  int bytes_sent = send(sockfd, cmd, PACKET_LEN, 0);
  return bytes_sent;
}

int read_command(Command* command, FILE* fp){
  char buffer[MSG_LEN + 1], *token;
  fgets(buffer, MSG_LEN + 1, fp);

  token = strtok(buffer, " ");
  if(strlen(token) > CMD_LEN)
    token[CMD_LEN] = 0;
  strcpy(command->cmd, token);
  
  if((token = strtok(NULL, " "))!= NULL){
    token = strtok(token, "\n");
    if(strlen(token) > ARG_LEN)
      token[ARG_LEN] = 0;
    strcpy(command->arg, token);
  }
  
  return strlen(command->cmd) + strlen(command->arg);
}

int get_command(Command* command, int sockfd, int print){
  char buffer[MSG_LEN], *token;
  int bytes_rcvd = recv(sockfd, buffer, PACKET_LEN, 0);
  
  if(bytes_rcvd > 0){
    buffer[bytes_rcvd] = 0;
    if(print)
      printf("%s\n", buffer);
    token = strtok(buffer, " ");
    if(strlen(token) > CMD_LEN)
      token[CMD_LEN] = 0;
    strcpy(command->cmd, token);
    if((token = strtok(NULL, " ")) != NULL && (token = strtok(token, "\r")) != NULL){
      if(strlen(token) > ARG_LEN)
	token[ARG_LEN] = 0;
      strcpy(command->arg, token);
    }
  }
  
  return bytes_rcvd;
}

COMMAND_ENUM cmd_str_to_enum(const char* cmd_str){
  for(size_t i = 0; i < NUM_COMMANDS; ++i)
    if(strcmp(cmd_str, COMMAND_STRING[i]) == 0)
      return i;
  return -1;
}

const char* cmd_enum_to_str(COMMAND_ENUM cmd_enum){
  if(cmd_enum > 0 && cmd_enum < NUM_COMMANDS)
    return COMMAND_STRING[cmd_enum];
  return NULL;  
}

int handle_login(int sockfd){
  Command c;
  
  /* Get user name (or die trying) */
  get_command(&c, sockfd, 1);
  while((strcmp(c.cmd, "USER") != 0)){
    send_response("332", "Need account for login", sockfd);
    get_command(&c, sockfd, 1);
  }
  
  for(size_t i = 0; i < NUM_USERS + 1; ++i)
    if(i == NUM_USERS){
      send_response("430", "Invalid User name", sockfd);
      return 1;
    }else if(strcmp(USERS[i], c.arg) == 0){
      send_response("331", "User name okay, need password", sockfd);
      break;
    }
  
  get_command(&c, sockfd, 1);
  while((strcmp(c.cmd, "PASS") != 0)){
    send_response("331", "User name okay, need password", sockfd);
    get_command(&c, sockfd, 1);
  }
  
  for(size_t i = 0; i < NUM_USERS + 1; ++i)
    if(i == NUM_USERS){
      send_response("430", "Invalid Password", sockfd);
      return 1;
    }else if(strcmp(PASSWORDS[i], c.arg) == 0){
      send_response("230", "User logged in, proceed.", sockfd);
      break;
    }
  
  return 0; 
}

const char* user_cmd_enum_to_str(USER_CMD_ENUM cmd_enum){
  if(cmd_enum > 0 && cmd_enum < NUM_USER_CMDS)
    return USER_CMD_STRING[cmd_enum];
  return NULL;  
}

USER_CMD_ENUM user_cmd_str_to_enum(char* cmd_str){
  if(cmd_str != NULL){
    for(size_t i = 0; i < NUM_USER_CMDS; ++i)
      if(strcmp(cmd_str, USER_CMD_STRING[i]) == 0)
	return i;
  }
  return -1;
}

int handle_list(char* arg, int sockfd, int data_sockfd){
  int status = -1, byte_count = 0;
  DIR *dr;
  struct dirent *de;
  char buffer[BUF];

  memset(buffer, 0, sizeof(buffer));

  if(arg == NULL || (dr = opendir(arg)) == NULL){
    send_response("451", "Requested action aborted. Local error in processing.", sockfd);
    return -1;
  }

  while ((de = readdir(dr)) != NULL)
    byte_count += (strlen(de->d_name) + 1);

  closedir(dr);
  
  sprintf(buffer, "ASCII data connection for /bin/ls (%d bytes)", byte_count);
  if(send_response("150", buffer, sockfd) < 0){
    perror("handle_list()\n");
    exit(1);
  }
  
  dr = opendir(arg);
  
  while ((de = readdir(dr)) != NULL){
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s\n", de->d_name);
    if((status = send(data_sockfd, buffer, strlen(buffer), 0)) < 0){
      send_response("451", "Requested action aborted. Local error in processing.", sockfd);
      return -1;
    }
  }
  
  closedir(dr);  
  
  if(status > 0)
    send_response("226", "ASCII Transfer Complete", sockfd);
  else
    send_response("550", "Requested action not taken.", sockfd);
  
  return status;
}

int handle_ls(char* arg, int sockfd, int data_sockfd){
  int byte_count = 0, bytes_rcvd = 0, status = 0;
  char buffer[BUF], *ptr;
  Command c;

  memset(&c, 0, sizeof(c));
  memset(buffer, 0, sizeof(buffer));
  
  build_command(&c, "LIST", arg);
  send_command(&c, sockfd);

  status = get_response(buffer, sockfd, 1);
  
  if(status == 451) return -1;
  
  printf("\n");

  ptr = strtok(buffer, "(");
  ptr = strtok(NULL, "(");
  ptr = strtok(ptr, " ");
  
  assert(ptr != NULL);
  
  for(size_t i = 0; i < strlen(ptr); ++i)
    byte_count += (ptr[i] - 0x30) * pow(10, strlen(ptr) - i - 1);

  memset(buffer, 0, sizeof(buffer));
  
  while(bytes_rcvd < byte_count && (status = recv(data_sockfd, buffer, sizeof(buffer), 0)) > 0){
    buffer[sizeof(buffer)] = 0;
    printf("%s", buffer);
    memset(buffer, 0, sizeof(buffer));
    bytes_rcvd += status;
  }
  
  return get_response(buffer, sockfd, 1);
}

int handle_pasv(int cmd_port, const char* ip, int sockfd){
  int data_socket = 0, client_socket, data_port = cmd_port + 1, status = 0;
  char buffer[BUF];
  struct sockaddr_in server_addr;
  socklen_t sckln = sizeof(struct sockaddr_in);
  
  memset(buffer, 0, BUF);
  
  data_socket = create_socket();
  server_addr = create_socket_address(data_port, ip);
  while(bind_connection(data_socket, (struct sockaddr*)&server_addr) < 0)
    server_addr = create_socket_address(++data_port, ip);

  sprintf(buffer, "Entering Passive Mode (%d)", data_port);
  status = send_response("227", buffer, sockfd);
  
  if(status < 0 ){
    perror("send_data_port()\n");
    exit(1);
  }
  
  listen_for_connection(data_socket, BACKLOG);
  client_socket = accept(data_socket,(struct sockaddr *)&server_addr,&sckln);

  return client_socket;
}

int data_port_connect(int sockfd, char* ip){
  int data_port = 0,  data_socket = 0;
  char buffer[BUF], *ptr;
  struct sockaddr_in server_addr;

  memset(buffer, 0, sizeof(buffer));
  
  if(recv(sockfd, buffer, PACKET_LEN, 0) < 0){
    perror("data_port_connect()\n");
    exit(1);
  }
  
  buffer[PACKET_LEN] = 0;
  
  ptr = strtok(buffer, "(");
  ptr = strtok(NULL, "(");
  ptr = strtok(ptr, ")");

  assert(ptr != NULL);

  for(size_t i = 0; i < strlen(ptr); ++i)
    data_port += (ptr[i] - 0x30) * pow(10, strlen(ptr) - i - 1);

  data_socket = create_socket();
  server_addr = create_socket_address(data_port, ip);
  if((connect_to_server(data_socket,(struct sockaddr*)&server_addr)) < 0){
    perror("data_port_connect()\n");
    exit(1);
  }
  
  return data_socket;
}

int handle_rm(char* arg, int sockfd){
  struct stat statbuf;
  char buffer[BUF];
  
  memset(buffer, 0, sizeof(buffer));
  sprintf(buffer, "rm -r %s", arg);
  
  if(stat(arg, &statbuf) == 0 && system(buffer) == 0)
    if(S_ISDIR(statbuf.st_mode))
      return send_response("250", "The directory was successfully removed", sockfd);
    
  return send_response("550", "Can't remove directory: No such directory", sockfd);
}

int handle_delete(char* arg, int sockfd){
  struct stat statbuf;
  char buffer[BUF];
  
  memset(buffer, 0, sizeof(buffer));
  sprintf(buffer, "rm %s", arg);
  
  if(stat(arg, &statbuf) == 0 && system(buffer) == 0)
    if(S_ISREG(statbuf.st_mode)){
      memset(buffer, 0, sizeof(buffer));
      sprintf(buffer, "Deleted %s", arg);
      return send_response("250", buffer, sockfd);
    }

  memset(buffer, 0, sizeof(buffer));
  sprintf(buffer, "Could not delete %s: No such file", arg);
  return send_response("550", buffer, sockfd);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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
  "PWD",  /* Print working directory */
  "PASS", /* Send password (NOTE: THE FTP USES PLAINTEXT PASSWORDS)*/
  "PORT", /* Request open port on specific IP addr/Port No */
  "QUIT", /* Log off of server */
  "RETR", /* Retrieve a file */
  "STOR", /* Send or put a file */
  "SYST", /* Identify system type */
  "TYPE", /* Specify type (A for ASCII, I for binary) */
  "USER" /* Send Username */
};


int get_response(char* buffer, size_t len, int sockfd, int print){
  int  response = 0;
  if(recv(sockfd, buffer, len, 0) > 0){
    buffer[len] = 0;
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
  int bytes_sent = send(sockfd, cmd, cmd_length, 0);
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
  int bytes_sent = send(sockfd, cmd, cmd_length, 0);
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
  int bytes_rcvd = recv(sockfd, buffer, sizeof(buffer), 0);
  
  if(bytes_rcvd > 0){
    buffer[bytes_rcvd] = 0;
    
    if(print)
      printf("%s\n", buffer);

    token = strtok(buffer, " ");
    if(strlen(token) > CMD_LEN)
      token[CMD_LEN] = 0;
    strcpy(command->cmd, token);
    
    if((token = strtok(NULL, " "))!= NULL){
      if(strlen(token) > ARG_LEN)
	token[ARG_LEN] = 0;
      strcpy(command->arg, token);
    }
  }
  
  return bytes_rcvd;
}

int send_data_port(int cmd_port, const char* ip, int sockfd){
  int sockfd2 , i = 2, bytes_sent;
  struct sockaddr_in socket_addr;
  char msg[ARG_LEN];
  Command c;

  memset(&sockfd2, 0, sizeof(sockfd2));
  memset(&c, 0, sizeof(c));
  memset(msg, 0, sizeof(msg));
  
  sockfd2 = create_socket();
  socket_addr = create_socket_address(cmd_port + 1, ip);
  while(bind_connection(sockfd2, (struct sockaddr*)&socket_addr) < 0)
    socket_addr = create_socket_address(cmd_port + i, ip);

  sprintf(msg,"%s,%d\r\n", ip,cmd_port + i);
  build_command(&c, "PORT", msg);
  bytes_sent = send_command(&c, sockfd);
  if(bytes_sent < 0) return bytes_sent;
  return sockfd2;
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
  
  for(size_t i = 0; i < NUM_USERS + 1; ++i){
    if(i == NUM_USERS){
      send_response("430", "Invalid User name", sockfd);
      return 1;
    }
    if(strcmp(USERS[i], c.arg) == 0){
      send_response("331", "User name okay, need password", sockfd);
      break;
    }
  }
  
  /* Get password (WARNING: FTP TRANSMITS PASSWORDS IN PLAINTEXT) */
  get_command(&c, sockfd, 1);
  while((strcmp(c.cmd, "PASS") != 0)){
    send_response("331", "User name okay, need password", sockfd);
    get_command(&c, sockfd, 1);
  }
  
  for(size_t i = 0; i < NUM_USERS + 1; ++i){
    if(i == NUM_USERS){
      send_response("430", "Invalid Password", sockfd);
      return 1;
    }
    if(strcmp(PASSWORDS[i], c.arg) == 0){
      send_response("230", "User logged in, proceed.", sockfd);
      break;
    }
  }
  return 0; 
}

int handle_port(char* arg, int sockfd){
  char* ip = strtok(arg, ",");
  char* port = strtok(NULL, ",");
  int sockfd2, port_int = atoi(port);;

  memset(&sockfd2, 0, sizeof(sockfd2));
  sockfd += 0;
  sockfd2 = create_socket();
  struct sockaddr_in sockin = create_socket_address(port_int, ip);
  return bind_connection(sockfd2, (struct sockaddr*) &sockin);
}

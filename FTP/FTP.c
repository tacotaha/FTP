#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../Stream/Stream.h"
#include "FTP.h"

int get_response(char* buffer, int sockfd, int print){
  int bytes_rcvd = recv(sockfd, buffer, sizeof(buffer), 0);
  if(bytes_rcvd > 0){
    buffer[bytes_rcvd] = 0;
    if(print)
      printf("%s\n", buffer);
  }
  return bytes_rcvd;
}

int send_command(const Command* command, int sockfd){
  const size_t cmd_length = CMD_LEN + strlen(command->arg);
  char cmd[cmd_length];
  sprintf(cmd, "%s %s\r\n", command->cmd, command->arg);
  int bytes_sent = send(sockfd, cmd, cmd_length, 0);
  return bytes_sent;
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

int handle_login(int sockfd){
  Command c;
  
  /* Get user name (or die trying) */
  get_command(&c, sockfd, 1);
  while((strcmp(c.cmd, "USER") != 0)){
    send_response("332", "Need account for login", sockfd);
    get_command(&c, sockfd, 1);
  }
  
  printf("READ USERNAME: %s\n", c.arg);
  printf("In hex:\n");
  print_hex(c.arg, strlen(c.arg));
  
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

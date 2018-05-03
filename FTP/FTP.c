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

int read_command(Command* command, FILE* fp){
  char buffer[MSG_LEN + 1], *token;
  fgets(buffer, MSG_LEN + 1, fp);

  token = strtok(buffer, " ");
  if(strlen(token) > CMD_LEN)
    token[CMD_LEN] = 0;
  strcpy(command->cmd, token);
  
  if((token = strtok(NULL, " "))!= NULL){
    if(strlen(token) > ARG_LEN)
      token[ARG_LEN] = 0;
    strcpy(command->arg, token);
  }
  
  return strlen(command->cmd) + strlen(command->arg);
}

int get_command(Command* command, int sockfd, int print){
  char buffer[MSG_LEN];
  int bytes_rcvd = recv(sockfd, buffer, sizeof(buffer), 0);
  
  if(bytes_rcvd > 0){
    buffer[bytes_rcvd] = 0;
    
    if(print)
      printf("%s\n", buffer);
    
    memcpy(command->cmd, buffer, CMD_LEN - 1);
    command->cmd[CMD_LEN] = 0;
    
    memcpy(command->arg, buffer + CMD_LEN - 2, ARG_LEN);
    command->arg[ARG_LEN] = 0;
  }
  
  return bytes_rcvd;
}



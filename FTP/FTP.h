#ifndef __FTP_H__
#define __FTP_H__

#include "../Stream/Stream.h"

static const char* welcome =  "-=(<*>)=-.:. (( Welcome to The FTP server)) .:.-=(<*>)=-";
typedef struct command{
  char cmd[5];
  char arg[BUF];
}Command;

/* 
   Read the server's response and return the 
   status code as an integer.

   @param buffer : a char buffer of size no less than BUF
   @param sockfd : the socket file descriptor associated with the server
   @param print  : an boolean value specifying wheather or not to print the response.
   @return int   : The status code (first three bytes) associated with the response.
*/
int get_response(char* buffer, int sockfd, int print);

/*
  Send a valid FTP command to the server.
  
  @param command : A null-terminated string containing the command.
  @param sockfd  : The socket file descriptor associated with the server.
  @return int    : The number of bytes sent to the server, or a int < 0 
                   if there occured an error.
 */
int send_command(const Command* command, int sockfd);

/* Server Functions for handling each command each have the format...
   @param arg    : The null-terminated paramater of the command, usually
                   same as command->arg.
   @param sockfd : The socket file descriptor of the associated client.
   @return int   : The number of bytes sent to the server, or an int < 0
                   if there occured an error.
*/

int handle_pwd(const char* arg, int sockfd);















#endif

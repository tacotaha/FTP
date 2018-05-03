#ifndef __FTP_H__
#define __FTP_H__

#include <stdio.h>
#include "../Stream/Stream.h"

#define CMD_LEN 5
#define ARG_LEN 507
#define MSG_LEN CMD_LEN + ARG_LEN
#define NUM_COMMANDS 14
#define NUM_USERS 5

#define PASSWORD password123

static const char* USERS[NUM_USERS] = {"Euclid", "Newton", "Gauss", "Euler", "Hilbert"};
static const char* PASSWORDS[NUM_USERS] = {"geometry", "calculus", "electrostatics", "konigsberg", "spaces"};

static const char* welcome_message = "-=(<*>)=-.:. (( Welcome to The FTP server)) .:.-=(<*>)=-";

static const char* COMMAND_STRING[14] = {
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
  "USER"  /* Send Username */
};

typedef enum Command_Enum{
  ABOR, 
  LIST,
  DELE,
  RMD,
  MKD,
  PWD,
  PASS,
  PORT,
  QUIT,
  RETR,
  STOR,
  SYST,
  TYPE,
  USER
}COMMAND_ENUM;

typedef struct command{
  char cmd[CMD_LEN];
  char arg[ARG_LEN];
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
  
  @param command : A nonempty command struct
  @param sockfd  : The socket file descriptor associated with the server.
  @return int    : The number of bytes sent to the server, or a int < 0 
                   if there occured an error.
 */
int send_command(const Command* command, int sockfd);

/*
  Send a response from the server containing a status code followed by a message
  
  @param status  : Status code (see STATUS)
  @param msg     : Message to accompany the status code
  @param sockfd  : The socket file descriptor associated with the recipient.
  @return int    : The number of bytes sent to the server, or a int < 0 
                   if there occured an error.
 */
int send_response(const char* status, const char* msg, int sockfd);

/*
  Read and convert an FTP command from a file (usually stdin)
  
  @param command : A command struct
  @param sockfd  : The socket file descriptor associated with the server.
  @return int    : The number of bytes read from the file (max is BUF2)
 */
int read_command(Command* command, FILE* fp);

/*
  Get a valid FTP command to the server.
  
  @param command : A nonempty command struct.
  @param sockfd  : The socket file descriptor associated with the server.
  @param print   : A boolean value specifying wheather or not to print the response.
  @return int    : The number of bytes sent to the server, or a int < 0 
                   if there occured an error.
 */

int get_command(Command* command, int sockfd, int print);

/*
  Handle user login to the server
  
  @param sockfd  : The socket file descriptor associated with the server.
  @return int    : 0 if user logged in, nonzero otherwise.

 */
int handle_login(int sockfd);

/* Server Functions for handling each command each have the format...
   @param arg    : The null-terminated paramater of the command, usually
                   same as command->arg.
   @param sockfd : The socket file descriptor of the associated client.
   @return int   : The number of bytes sent to the server, or an int < 0
                   if there occured an error.
*/


int handle_pwd(const char* arg, int sockfd);
int handle_pass(const char* arg, int sockfd);
#endif

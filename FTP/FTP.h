#ifndef __FTP_H__
#define __FTP_H__

#include <stdio.h>
#include "../Stream/Stream.h"

#define CMD_LEN 5
#define ARG_LEN 507
#define MSG_LEN CMD_LEN + ARG_LEN
#define NUM_COMMANDS 15
#define NUM_USER_CMDS 10
#define NUM_USERS 5

/* Defined in FTP.c */
extern const char* USERS[NUM_USERS];
extern const char* PASSWORDS[NUM_USERS];
extern const char* welcome_message;
extern const char* COMMAND_STRING[NUM_COMMANDS];
extern const char* USER_CMD_STRING[NUM_USER_CMDS];

typedef enum Command_Enum{
  ABOR = 0, 
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
  USER,
  PASV
}COMMAND_ENUM;

typedef enum User_Command_Enum{
  LS = 0,
  CD,
  DELETE,
  GET,
  HELP,
  MKDIR,
  PUT,
  PWD_,
  QUIT_,
  RMDIR,
} USER_CMD_ENUM;

typedef struct command{
  char cmd[CMD_LEN];
  char arg[ARG_LEN];
}Command;

/* 
   Read the server's response and return the 
   status code as an integer.

   @param buffer : A char buffer of size no less than BUF
   @param len    : Number of bytes to receive (must be <= BUF)
   @param sockfd : The socket file descriptor associated with the server
   @param print  : A Boolean value specifying whether or not to print the response.
   @return int   : The status code (first three bytes) associated with the response.
*/
int get_response(char* buffer, size_t len, int sockfd, int print);

/*
  Send a valid FTP command to the server.
  
  @param command : A nonempty command struct
  @param sockfd  : The socket file descriptor associated with the server.
  @return int    : The number of bytes sent to the server, or a int < 0 
                   if there occured an error.
 */
int send_command(const Command* command, int sockfd);

/*
  Send a response to the client containing a status code followed by a message
  
  @param status  : Status code (see STATUS)
  @param msg     : Message to accompany the status code
  @param sockfd  : The socket file descriptor associated with the recipient
  @return int    : The number of bytes sent to the server, 
  		   or an int < 0 if there occured an error
 */
int send_response(const char* status, const char* msg, int sockfd);

/*
  Parse a string of the form "CMND This is an agrument" from a 
  file into a valid command struct.
  
  @param command : A command struct
  @param sockfd  : The socket file descriptor associated with the server.
  @return int    : The number of bytes read from the file (<= BUF2)
 */
int read_command(Command* command, FILE* fp);

/*
  Get a valid FTP command from the server.
  
  @param command : A nonempty command struct.
  @param sockfd  : The socket file descriptor associated with the server.
  @param print   : A boolean value specifying wheather or not to print the response.
  @return int    : The number of bytes sent to the server, or a int < 0 
                   if there occured an error.
 */
int get_command(Command* command, int sockfd, int print);

/*
  Fill a command struct with the specified parameters.
  
  @param command : An empty command struct.
  @param cmd     : C-string representing the command
  @param arg     : Argument accompanying the command.
 */
void build_command(Command* command, char* cmd, char* arg);

/*
  Handle user login to the server
  
  @param sockfd  : The socket file descriptor associated with the server.
  @return int    : 0 if user logged in, nonzero otherwise.
 */
int handle_login(int sockfd);

/*
  Convert a command's C-string representation to it's 
  corresponding enum representation.
  
  @param cmd_str   : The C-string represention of the command. 
                     Must be an element of COMMAND_STRING.
  @return CMD_ENUM : The Enum representation of the command.
                     -1 if no such command.
 */
COMMAND_ENUM cmd_str_to_enum(const char* cmd_str);

/*
  Convert a command's Enum representation to it's 
  corresponding C-string representation.
  
  @param cmd_enum     : An enum represention of the command. 
                        Must be an element of COMMAND_ENUM.
  @return const char* : The C-string representation of the command.
                        Must be an element of COMMAND_STRING.
			NULL if no such command
 */
const char* cmd_enum_to_str(COMMAND_ENUM cmd_enum);

/*
  Convert a user command's C-string representation to it's 
  corresponding enum representation.
  
  @param cmd_str        : The C-string represention of the command. 
                          Must be an element of USER_CMD_STRING.
  @return USER_CMD_ENUM : The Enum representation of the cnnommand.
                          -1 if no such command.
 */
USER_CMD_ENUM user_cmd_str_to_enum(char* cmd_str);

/*
  Convert a user command's Enum representation to it's 
  corresponding C-string representation.
  
  @param cmd_enum     : An enum represention of the command. 
                        Must be an element of COMMAND_ENUM.
  @return const char* : The C-string representation of the command.
                        Must be an element of USER_CMD_STRING.
			NULL if no such command
 */
const char* user_cmd_enum_to_str(USER_CMD_ENUM cmd_enum);


/*
  Responds to the passive command by creating a data port, 
  sending it out to the client in the form of a response
  and waiting for the client to connect. Client should be
  connected to the data port after this is exits!
  
  @param cmd_port : The port command port no.
  @param ip       : C-string represenation of the ip addr
  @param sockfd   : The socket file descriptor associated with the cmd port.
  @return int     : The data socket file descriptor
*/
int handle_pasv(int cmd_port, const char* ip, int sockfd);

/*
  Receives the server's response to the PASV command,
  parses is, and connects to the data port it opened up.
  @param sockfd   : The socket file descriptor associated with the cmd port.
  @return int     : The data socket file descriptor
*/
int data_port_connect(int sockfd, char* ip);

/* Server Functions for handling each command each have the format...
   @param arg    : The null-terminated paramater of the command, usually
                   the same as command->arg.
   @param sockfd : The socket file descriptor of the associated client.
   @return int   : Status code
*/
int handle_pass(const char* arg, int sockfd);
int handle_port(char* arg, int sockfd, int* data_port);
int handle_list(char* arg, int sockfd, int data_sockfd);
int handle_ls(char* arg, int sockfd, int data_sockfd);
#endif

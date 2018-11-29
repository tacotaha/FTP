#ifndef __FTP_H__
#define __FTP_H__

#include <stdio.h>
#include "../Connect/Connect.h"

#define CMD_LEN 5
#define ARG_LEN 507
#define MSG_LEN CMD_LEN + ARG_LEN
#define NUM_COMMANDS 12
#define NUM_USER_CMDS 10
#define NUM_USERS 5
#define PACKET_LEN 512
#define BUF 1024
#define BUF2 2048
#define BUF3 4096
#define MAX_CLIENTS 100
#define PORT_NO 4444
#define IP "127.0.0.1"
#define BACKLOG 10

/* Defined in FTP.c */
extern const char* USERS[NUM_USERS];
extern const char* PASSWORDS[NUM_USERS];
extern const char* welcome_message;
extern const char* COMMAND_STRING[NUM_COMMANDS];
extern const char* USER_CMD_STRING[NUM_USER_CMDS];

typedef enum Command_Enum {
  LIST,
  DELE,
  RMD,
  MKD,
  CWD,
  PWD,
  PASS,
  QUIT,
  RETR,
  STOR,
  USER,
  PASV
} COMMAND_ENUM;

typedef enum User_Command_Enum {
  LS,
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

typedef struct command {
  char cmd[CMD_LEN];
  char arg[ARG_LEN];
} Command;

/*
   Read the server's response and return the
   status code as an integer.

   @param buffer : A char buffer of size no less than BUF
   @param sockfd : The socket file descriptor associated with the server
   @param print  : A Boolean value specifying whether or not to print the
   response.
   @return int   : The status code (first three bytes) associated with the
   response.
*/
int get_response(char* buffer, int sockfd, int print);

/*
  Send a valid FTP command to the server.

  @param command : A nonempty command struct
  @param sockfd  : The socket file descriptor associated with the server.
  @return int    : The number of bytes sent to the server, or a int < 0
                   if there occurred an error.
 */
int send_command(const char* cmd, const char* arg, int sockfd);

/*
  Send a response to the client containing a status code followed by a message

  @param status  : Status code (see STATUS)
  @param msg     : Message to accompany the status code
  @param sockfd  : The socket file descriptor associated with the recipient
  @return int    : The number of bytes sent to the server,
                   or an int < 0 if there occurred an error
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
  @param print   : A Boolean value specifying whether or not to print the
  response.
  @return int    : The number of bytes sent to the server, or a int < 0
                   if there occurred an error.
 */
int get_command(Command* command, int sockfd, int print);

/*
  Fill a command struct with the specified parameters.

  @param command : An empty command struct.
  @param cmd     : C-string representing the command
  @param arg     : Argument accompanying the command.
 */
void build_command(Command* command, const char* cmd, const char* arg);

/*
  Handle user login to the server

  @param sockfd  : The socket file descriptor associated with the server.
  @return int    : 0 if user logged in, nonzero otherwise.
 */
int handle_login(int sockfd);

/*
  Convert a command's C-string representation to it's
  corresponding enum representation.

  @param cmd_str   : The C-string representation of the command.
                     Must be an element of COMMAND_STRING.
  @return CMD_ENUM : The Enum representation of the command.
                     -1 if no such command.
 */
COMMAND_ENUM cmd_str_to_enum(const char* cmd_str);

/*
  Convert a command's Enum representation to it's
  corresponding C-string representation.

  @param cmd_enum     : An enum representation of the command.
                        Must be an element of COMMAND_ENUM.
  @return const char* : The C-string representation of the command.
                        Must be an element of COMMAND_STRING.
                        NULL if no such command
 */
const char* cmd_enum_to_str(COMMAND_ENUM cmd_enum);

/*
  Convert a user command's C-string representation to it's
  corresponding enum representation.

  @param cmd_str        : The C-string representation of the command.
                          Must be an element of USER_CMD_STRING.
  @return USER_CMD_ENUM : The Enum representation of the command.
                          -1 if no such command.
 */
USER_CMD_ENUM user_cmd_str_to_enum(char* cmd_str);

/*
  Convert a user command's Enum representation to it's
  corresponding C-string representation.

  @param cmd_enum     : An enum representation of the command.
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
  @param ip       : C-string representation of the ip addr
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

/*
   Cross-check provided password against the elements of PASSWORDS
   @param arg       : The user-provided password
   @param user_name : The corresponding user name
   @param sockfd    : The client socket file descriptor
   @return int      : < 0 on failure, success otherwise
*/
int handle_pass(const char* arg, const char* user_name, int sockfd);

/*
   Cross-check provided username against the elements of USERS
   @param arg       : The user-provided username
   @param user_name : The corresponding user name (to be saved by caller)
   @param sockfd    : The client socket file descriptor
   @return int      : < 0 on failure, success otherwise
*/
int handle_user(const char* arg, char* user_name, int client_socket);

/*
   Send the contents of the directory arg to the client.
   @param arg         : The path of the directory being listed
   @param sockfd      : The client socket file descriptor
   @param data_sockfd : The data socket file descriptor
   @return int        : < 0 on failure, success otherwise
*/
int handle_list(const char* arg, int sockfd, int data_sockfd);

/*
   Receive the contents of the directory arg from the server.
   @param arg         : The path of the directory being listed
   @param sockfd      : The client socket file descriptor
   @param data_sockfd : The data socket file descriptor
   @return int        : < 0 on failure, success otherwise
*/
int handle_ls(const char* arg, int sockfd, int data_sockfd);

/*
   Remove the directory specified by arg.
   @param arg         : The path of the directory to be removed
   @param sockfd      : The client socket file descriptor
   @return int        : # of bytes transmitted in response
*/
int handle_rm(const char* arg, int sockfd);

/*
   Remove the file specified by arg.
   @param arg         : The path of the file to be deleted
   @param sockfd      : The client socket file descriptor
   @return int        : # of bytes transmitted in response
*/
int handle_delete(const char* arg, int sockfd);

/*
   Create a directory by the name of arg in the cwd
   @param arg         : The name of the directory to be created
   @param sockfd      : The client socket file descriptor
   @return int        : # of bytes transmitted in response
*/
int handle_mkdir(const char* arg, int sockfd);

/*
   Retrieve a the file specified by arg from the server
   @param arg         : The file to be retrieved
   @param sockfd      : The client socket file descriptor
   @param data_sockfd : The data socket file descriptor
   @return int        : # of bytes transmitted in response
*/
int handle_get(const char* arg, int sockfd, int data_sockfd);

/*
   Send the current working directory to the client.
   @param arg         : The current working directory
   @param sockfd      : The client socket file descriptor
   @return int        : # of bytes transmitted in response
*/
int handle_pwd(const char* arg, int sockfd);

/*
   Send the file specified by arg to the client.
   @param arg         : The name of the file to be retrieved.
   @param sockfd      : The client socket file descriptor.
   @param data_sockfd : The data socket file descriptor
   @return int        : # of bytes transmitted in response.
*/
int handle_retr(const char* arg, int sockfd, int data_sockfd);

/*
   Send the server the passive command and connect to the
   port in its response.
   @param sockfd      : The client socket file descriptor.
   @param data_sockfd : The addr of the data socket file descriptor
   @return int        : The data socket file descriptor
*/
int send_passive(int sockfd, int* data_sockfd, char* ip);

/*
   Retrieve the file arg from the client
   @param arg         : The name of the file to be stored.
   @param sockfd      : The client socket file descriptor.
   @param data_sockfd : The data socket file descriptor
   @return int        : # of bytes transmitted in response.
*/
int handle_stor(char* arg, int sockfd, int data_sockfd);

/*
   Send the file specified by arg to the server.
   @param arg         : The name of the file to be sent.
   @param sockfd      : The client socket file descriptor.
   @param data_sockfd : The data socket file descriptor
   @return int        : # of bytes transmitted in response.
*/
int handle_put(char* arg, int sockfd, int data_sockfd);

/*
   Change the server's current working directory
   @param arg         : The path of the new working dir.
   @param sockfd      : The client socket file descriptor.
   @return int        : # of bytes transmitted in response.
*/
int handle_cwd(const char* arg, int client);

/*Print a list of available commands to the user*/
void handle_help(void);
#endif

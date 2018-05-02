#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF2 2048
#define BUF1 1024
#define BUF 512

#define PORT 21

int main(int argc, char* argv[]){

  FILE *fp;
  int sockfd;
  char user_name[BUF], password[BUF], response[BUF2];
  
  struct sockaddr_in servaddr;

  if(argc < 2){
    fprintf(stderr, "Usage: %s Server_IP\n", argv[0]);
    exit(-1);
  }
  
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("Failed to Create socket()\n");
    exit(-1);
  }else{
    printf("Created Socket!\n");
  }

  memset((char*) &servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);
  servaddr.sin_port = htons(PORT);
  
  if(connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0){
    perror("Could not connect!\n");
    exit(-1);
  }else
    printf("Connected to %s!\n", argv[1]);

  printf("Enter your username > ");
  fgets(user_name, BUF, stdin);

  printf("Enter your password > ");
  fgets(password, BUF, stdin);

  /* First thing the server requests is the user's credentials*/
  send(sockfd, user_name, BUF, 0);
  send(sockfd, password, BUF, 0);
  
  recv(sockfd, response, BUF2, 0);
  response[BUF2] = 0;
  printf("%s\n",response);

  close(sockfd);
  
  return 0;
}

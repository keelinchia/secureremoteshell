#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <sys/wait.h>

#define PORT_NUMBER 53953

void run_shell(int connfd);

void err_sys(char *msg)
{
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[])
{
  int listenfd, connfd;
  socklen_t cli_length; 
  struct sockaddr_in serv_addr, cli_addr;
  struct hostent *client;

  /* Create a socket for communication. */
  if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    err_sys("socket error");
  }

  /* Initialize server struct to zero. */
  memset((char *) &serv_addr, 0, sizeof(struct sockaddr_in));
  
  /* Specify the address family. */
  serv_addr.sin_family = AF_INET;

  /* Specify and convert the port number to network byte order. */
  serv_addr.sin_port = htons(PORT_NUMBER);

  /* Specify the IP address of the server. */
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  /* Bind server and socket. */
  if (bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    err_sys("bind error");
  }
  
  /* Make socket listen with client queue of size 5. */
  if (listen(listenfd, 5) < 0) {
    err_sys("listen error");
  }

  printf("Listening...\n");
  
  for(;;) {
    cli_length = sizeof(cli_addr);
    /* Blocks until client connects. */
    connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &cli_length);
    if (connfd < 0) {
      err_sys("accept error");
    }

    /* Get client. */
    client = gethostbyaddr((char *) &cli_addr.sin_addr, cli_length, AF_INET);
    
    /* Fork a child to handle a client. */
    if(fork() == 0) {// child
      printf("Connected client: %s\n", client->h_name);
      close(listenfd);
      run_shell(connfd);
      close(connfd);
      exit(0);
    }
    close(connfd);
  }
}

void run_shell(int connfd)
{
  int i, n, f;
  char *args[256];
  char *token;
  char buff[256];
  
  /* Receive client's command into buff. */
  if ((n = recv(connfd, buff, sizeof(buff), 0)) < 0) {
    err_sys("error receiving");
  }
  
  printf("Received command: ");
  printf("%s ", buff);
  printf("\n");
  
  /* Tokenize buff and store tokens into args. */
  i = 0;
  token = strtok(buff, " ");
  while (token != NULL) {  
    args[i] = token; 
    token = strtok(NULL, " ");
    i++;
  }
  
  /* Execute the command. */
  printf("Executing client's command...\n");
  execvp(args[0], args);

  /* Attempting to redirect stdout and stderr to the client. */
  dup2(1, connfd);
  dup2(2, connfd);
  close(1);
  close(2);
  
  /* Send output of execvp to client. */
  if ((n = send(connfd, buff, sizeof(buff), 0)) < 0) {
    err_sys("error sending");
  }
}

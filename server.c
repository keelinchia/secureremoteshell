#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

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

  /* Create a socket for communication. */
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    err_sys("socket error");
  }

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
  
  cli_length = sizeof(cli_addr);
  for(;;) {
    /* Blocks until client connects. */
    connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &cli_length);
    if (connfd < 0) {
      err_sys("accept error");
    }
    
    /* Fork a child to handle a client. */
    if(fork() == 0) {// child
      printf("Connected\n");
      close(listenfd);
      run_shell(connfd);
      close(connfd);
      exit(0);
    }
    close(connfd);
  }
  
  return 0;
}

void run_shell(int connfd)
{
  int i, j;
  char *args[256];

  /* Read client's command. */
  j = 0;
  while (args[j] != NULL) {
    if ((i = read(connfd, args[j], sizeof(args[j]))) < 0) {
      err_sys("error writing");
    }
    j++;
  }

  /* Associate connfd with stdout and stderror. */
  dup2(connfd, 1);
  dup2(connfd, 2);
  
  /* Execute the command. */
  execvp(args[0], args);
}

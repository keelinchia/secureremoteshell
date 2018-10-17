#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>  
#include <unistd.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT_NUMBER 53953

void err_sys(char *msg)
{
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[])
{
  int listenfd, i;
  char *args[256];
  struct sockaddr_in serv_addr;
  struct hostent *server; // For the definition of the server on the internet.

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    err_sys("socket error");
  }

  /* Get server name from the command line. */
  server = gethostbyname(argv[1]);
  
  /* Specify the address family. */
  serv_addr.sin_family = AF_INET;
  
  /* Specify and convert the port number to network byte order. */
  serv_addr.sin_port = htons(PORT_NUMBER);
  
  /* Specify the IP address of the server using the struct hostent type defined above. */
  memmove((char* )server->h_addr, 
	  (char* )&serv_addr.sin_addr.s_addr,
	  server->h_length);

  /* Connect the socket descriptor. */
  if (connect(listenfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        err_sys("connect error");
  }

  /* Parse user's command. */
  i = 0;
  while (argv[i] != NULL) {
    memmove(args[i], argv[i+2], strlen(argv[i+2]));
    i++;
  }
  
  /* Write the command to the socket. */
  if ((i = write(listenfd, args, sizeof(args))) < 0) {
    err_sys("error writing");
  }

  /* Read output of the server. */
  if ((i = read(listenfd, args, sizeof(args))) < 0) {
    err_sys("error reading");
  }
    
  /* Print the output. */
  i = 0;
  while (args[i] != NULL) {
    printf("%s\n",args[i]);
    i++;
  }
  
  return 0;
}

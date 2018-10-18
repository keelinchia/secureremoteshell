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
  int listenfd, i, n;
  char *buff;
  
  struct sockaddr_in serv_addr;
  struct hostent *server; // For the definition of the server on the internet.

  if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    err_sys("socket error");
  }

  /* Initialize server struct to zero. */
  memset((char *) &serv_addr, 0, sizeof(struct sockaddr_in));

  /* Get server name from the command line. */
  server = gethostbyname(argv[1]);
  
  /* Specify the address family. */
  serv_addr.sin_family = AF_INET;
  
  /* Specify and convert the port number to network byte order. */
  serv_addr.sin_port = htons(PORT_NUMBER);
  
  /* Specify the IP address of the server using the struct hostent type defined above. */
  memcpy((char* )&serv_addr.sin_addr.s_addr,
	 (char* )server->h_addr,
	 server->h_length);
  
  /* Connect the socket descriptor. */
  if (connect(listenfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    err_sys("connect error");
  }
  
  /* Parse user's command. */
  i = 0;
  buff = malloc(100 * sizeof(argv[0]));
  printf("User's command:");
  while (argv[i+2] != NULL) {
    strcat(buff, argv[i+2]);
    strcat(buff, " ");
    i++;
  }
  printf("%s\n", buff);
  
  /* Send the command to the server. */ 
  n = send(listenfd, buff, sizeof(buff), 0);
  if (n < 0) {
    err_sys("error sending");
  }
  
  printf("Executing on server...\n");
  
  /* Receive output from the server to args. */ 
  if ((n = recv(listenfd, buff, sizeof(buff), 0)) < 0) {
    err_sys("error receiving");
  }
  
  /* Print the output. */
  printf("%s\n", buff);

  free(buff);
  close(listenfd);
  
  exit(0);
}

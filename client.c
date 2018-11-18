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

void encrypt(char *buff);

void err_sys(char *msg)
{
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[])
{
  int listenfd, i, n, length;
  char *buff, *token;
  
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

  buff = malloc(16384);
  /* Parse user's command from argv into one string. */
  i = 0;
  while (argv[i+2] != NULL) {
    strcat(buff, argv[i+2]);
    i++;
  }  
 
  do {
    /* Display user's command. */
    printf("User's command: %s\n", buff);
    
    /* Encrypt the command */
    encrypt(buff);
    
    /* Display the ciphertext for testing. */
    printf("Command in ciphertext: %s\n", buff);   
    
    n = send(listenfd, buff, sizeof(buff), 0);
    if (n < 0) {
      err_sys("error sending");
    }
    
    printf("Executing on server...\n");
    
    /* Receive output from the server into buff. */ 
    for(;;) {
      if ((n = recv(listenfd, buff, sizeof(buff), 0)) < 0) {
	err_sys("error receiving");
      }
      /* Print the output in the same format. */
      printf("%.*s", n, buff);
      
      if (n < sizeof(buff)) {
	break;
      }  
    }
    printf("\n");

    memset(buff, '\0', 16384);
    
    /* Wait for more command from the user. */
    printf("Enter command (\"quit\" to exit): ");

    fgets(buff, 16384, stdin);
    
    /* Get rid of the '\n'. */
    length = strlen(buff); 
    if (buff[length - 1] == '\n') { 
      buff[length - 1] = '\0';
    }

    if (strcmp(buff, "quit") == 0) {
      break;
    }
    
    //free(buff);
    //buff = malloc(16384);
    
  } while (1);

  free(buff);
  
  close(listenfd);
  
  exit(0);
}

/* Helper Functions */
void encrypt(char *buff)
{
  int i;
  
  i = 0;
  while (buff[i] != '\0') {
    buff[i] += 3; // isu id: 53953 
    i++;
  }
}

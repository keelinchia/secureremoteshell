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

int run_shell(int connfd);
void decrypt(char *buff);

void err_sys(char *msg)
{
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[])
{
  int listenfd, connfd, status;
  pid_t pid;
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
    pid = fork(); 
    if(pid == 0) {// child 
      printf("Connected client: %s\n", client->h_name);
      close(listenfd);    
      
      while(!run_shell(connfd)) {
	printf("Done.\n\n");
      }
      
      printf("Client has terminated.\n\n");
      close(connfd);
      exit(0);
    } else {
      pid = wait(&status);
    }
  }
}

int run_shell(int connfd)
{
  int i, j, n, f, cmd, num_cmd, multi, status;
  pid_t pid, kill_pid; 
  char *args[100], *commands[100], *token, *buff, *copy;

  buff = malloc(16384);
  copy = malloc(16384);
  
  /* Receive client's command into buff. */
  if ((n = recv(connfd, buff, sizeof(buff), 0)) < 0) {
    err_sys("error receiving");
  }

  /* If client has terminated, return 1 to break the loop. */
  if (n == 0) {
    return 1; 
  }
  
  printf("Received command: %s\n", buff);
 
  /* Decrypt the command.  */
  decrypt(buff);
  
  /* Print the plaintext for testing. */
  printf("Command in plaintext: %s\n", buff);
  
  /* Tokenize the command in buff and store tokens into commands[]. */
  strcpy(copy, buff);

  i = 0;
  token = strtok(copy, ";");
  printf("token: %s\n", token);
  while (token != NULL) {
    commands[i] = token;
    token = strtok(NULL, ";");
    i++;
  }
  commands[i] = NULL;

  printf("command: %s\n", commands[0]);
  printf("command: %s\n", commands[1]);
  
  cmd = 0;
  while (commands[cmd] != NULL) {
    memset(buff, '\0', 16384);
    memset(copy, '\0', 16384);
    
    /* Look for flags in a command. 
       Separate a command its flags with white spaces. 
       Store the command with its flags in args[] for execution. */
    strcpy(copy, commands[cmd]);
    
    j = 1;
    token = strtok(copy, "-");
    args[0] = token;
    token = strtok(NULL, "-");
    while (token != NULL) {
      args[j] = "-"; //adding back "-"
      strcat(args[j], token);
      token = strtok(NULL, "-");
      j++;
    }
    args[j] = token;
    
    printf("Executing client's command...\n");
    printf("Executing command \"%s\"...\n", commands[cmd]); 
    
    /* Fork a child to execute the command. */
    pid = fork();
    if (pid == 0) { //child      
      /* Redirect stdout and stderr to connfd. */
      dup2(connfd, 1);
      dup2(connfd, 2);
    
      execvp(args[0], args);  	
    } else {
      kill_pid = wait(&status);
    }
    
    cmd ++;
  }

  free(buff);
  free(copy);

  return 0;
}

/* Helper Functions */
void decrypt(char *buff)
{
  int i;

  i = 0;
  while (buff[i] != '\0') {
    buff[i] -= 3; // isu id: 53953
    i++;
  }
}

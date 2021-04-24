
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#define MAX 80 
#define PORT 9000 
#define SA struct sockaddr 

// Function designed for chat between client and server. 
int func(int sockfd) 
{ 
	char buff[MAX]; 
	int n; 

	for (;;) 
	{ 
		bzero(buff, MAX); 

		// read the message from client and copy it in buffer 
		n = read(sockfd, buff, sizeof(buff));
		if (n < 0)
		{
			perror("read");
			return -1;
		}
		// print buffer which contains the client contents 
		printf("From client: %s\n\r ", buff); 
		n = write(sockfd, buff, sizeof(buff));
		if (n < 0)
		{
			perror("write");
			return -1;
		}

	} 
} 

/**
 * Server Application Entry
 */
int main(int argc, char *argv[]) 
{
	int sockfd, connfd, len, i; 
	struct sockaddr_in servaddr, cli;
	for(i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-d") == 0)
		{
			printf("Daemon\n\r");
			daemon(1, 1);
		}	
	} 

	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// Bind server ip 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Now server is ready to listen
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 
	len = sizeof(cli); 

	// Accept the data from client 
	connfd = accept(sockfd, (SA*)&cli, (socklen_t*)&len); 
	if (connfd < 0) { 
		printf("server acccept failed...\n"); 
		exit(0); 
	} 
	else
		printf("server acccepted the client...\n"); 

	// Function for chatting between client and server 
	func(connfd); 

	// After chatting close the socket 
	close(sockfd); 
} 


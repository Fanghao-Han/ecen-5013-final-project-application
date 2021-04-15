/**
 * aesdsocket.c - a socket based program for assignment-6
 * 
 * Reference: 
 * [1] socket,
 *     https://beej.us/guide/bgnet/html/#bind 
 * [2] signal handler, lecture 9 example code
 * [3] Singly Linked List,
 *     https://github.com/stockrt/queue.h/blob/master/sample.c
 *     https://raw.githubusercontent.com/freebsd/freebsd/stable/10/sys/sys/queue.h
 * [4] https://dzone.com/articles/parallel-tcpip-socket-server-with-multi-threading
 * [5] strftime(),
 *     https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
 * [6] POSIX Timer,
 *     https://riptutorial.com/posix/example/16306/posix-timer-with-sigev-thread-notification
 */

//#define DEBUG
//#define USE_AESD_CHAR_DEVICE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in.h>

#include <signal.h>
#include <pthread.h>
#include "queue.h"  // Singly-linked List

#if USE_AESD_CHAR_DEVICE==1
#include <time.h>
#endif
/*********************************************************************
 * Constants
 *********************************************************************/

#define PORT 9000
#define BACKLOG 5    // how many pending connections queue will hold
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 32

#if USE_AESD_CHAR_DEVICE==1
	#define FILE_PATH "/dev/aesdchar"
#else
	#define FILE_PATH "/var/tmp/aesdsocketdata.txt"
	#define WAIT_TIME   (10)	
#endif

/*********************************************************************
 * Global Variables
 *********************************************************************/
int total_len=0;
int sockfd;		// server
int sockfd_new;	// client

bool caught_sigint=false;// SIGINT global flag
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*********************************************************************
 * Structure
 *********************************************************************/
// thread SLIST
typedef struct slist_thread_s slist_thread_t;
struct slist_thread_s {
    pthread_t tid;	// thread id
    SLIST_ENTRY(slist_thread_s) entries;
};

/*********************************************************************
 * Function Definition
 *********************************************************************/
#ifdef DEBUG
	#define DEBUG_LOG(msg,...) printf("server.c| " msg "\n" , ##__VA_ARGS__)
	#define ERROR_LOG(msg,...) printf("server.c|ERROR| " msg "\n" , ##__VA_ARGS__)
#else
	#define DEBUG_LOG(msg,...) syslog(LOG_DEBUG,"" msg "\n" , ##__VA_ARGS__) 
	#define ERROR_LOG(msg,...) syslog(LOG_ERR,"" msg "\n" , ##__VA_ARGS__)
#endif

void signal_handler_init(void);
void signal_handler(int);

#if !USE_AESD_CHAR_DEVICE
void timerfunc ();
void timer_init (void);
#endif

int writer(char* writefile, char* writestr, size_t len);
void* thread_sock(void* arg);


/********************************************************************
 * Application Entry
 *********************************************************************/ 
int main(int argc, char *argv[]){
	// getopt
    bool dflag = false; // Daemon mode flag
	for (int i=0;i<(argc-1);i++) {
        int opt = getopt(argc, argv, "-d");
        if (opt == -1)
            break;
        switch (opt) {
        case 'd':
            DEBUG_LOG("Daemon Mode");
        	dflag = true;
            break;
        default:
            DEBUG_LOG("Common Mode");
			dflag = false;
			break;
        }
    }
	
	// variables
	struct sockaddr_in server;
	struct sockaddr_in client;

    char clientip[INET_ADDRSTRLEN];
	socklen_t addr_size;

	// syslog init
	openlog("aesdsocket.c", LOG_CONS, LOG_USER);

#if !USE_AESD_CHAR_DEVICE
	// Timer initialization
   	timer_t timerid;
#endif

	// Signal handler init
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);


    /**
     * TODO: Init thread slist
     * */ 
	slist_thread_t *thread_param=NULL;

    SLIST_HEAD(slisthead, slist_thread_s) head;
    SLIST_INIT(&head); 

	// mutex init
    pthread_mutex_init(&mutex, NULL);

	// server conf init
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = INADDR_ANY; 
    server.sin_port = htons( PORT );

    // Creating socket file descriptor 
    if ((sockfd = socket(server.sin_family, SOCK_STREAM, 0)) < 0) 
    { 
        ERROR_LOG("socket failed"); 
        exit(1); 
    } 
    DEBUG_LOG("Socket created");

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) 
    { 
        ERROR_LOG("setsockopt(SO_REUSEADDR) failed");
        exit(1); 
    }
 
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server))<0) 
    { 
        DEBUG_LOG("bind failed"); 
        exit(1); 
    }
  
    // daemon
	pid_t pid;
    if(dflag == 1){  	
    	pid = fork();
    	if(pid<0){
    		ERROR_LOG("fork failed!");
    		exit(1);
    	}
        if (pid > 0) {
			// PARENT PROCESS. Need to kill it.
			DEBUG_LOG("fork created in parent, exit");
			exit(0);
        }
        umask(0);// Change the file mode mask
    }
    
	// Listens for and accepts a connection
    if (listen(sockfd, BACKLOG) != 0) 
    { 
        ERROR_LOG("socket listen failed");
        exit(1); 
    } else{
        DEBUG_LOG("Listening...");
    }

#if !USE_AESD_CHAR_DEVICE
	// timer handler init
    struct sigevent sev;
    struct itimerspec trigger;

    memset(&sev, 0, sizeof(struct sigevent));
    memset(&trigger, 0, sizeof(struct itimerspec));

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_notify_function = &timerfunc;

    timer_create(CLOCK_REALTIME, &sev, &timerid);
    trigger.it_value.tv_sec = 10;
    trigger.it_interval.tv_sec = WAIT_TIME;
    timer_settime(timerid, 0, &trigger, NULL);
	DEBUG_LOG("Timer started");
#endif
	// loop
    while(!caught_sigint){	
		DEBUG_LOG("-------------------New Connection------------------\n");	
    	pthread_t tid = 0;
    	addr_size = sizeof(client);

	    sockfd_new = accept(sockfd, (struct sockaddr *)&client,  &addr_size);
	    if(caught_sigint){
			break;
		}
	    
	    if (sockfd_new==-1) 
	    { 
	        ERROR_LOG("at accept() client socket"); 
	        exit(1);
	    }
		DEBUG_LOG("Connection accepted, sockfd = %d", sockfd_new);

		inet_ntop(client.sin_family, &client.sin_addr, clientip, sizeof(clientip));
		//syslog(LOG_DEBUG,"Accepted connection from %s",clientip);
		DEBUG_LOG("Accepted connection from %s",clientip);
		

		thread_param = (slist_thread_t*)malloc(sizeof(slist_thread_t));
		thread_param->tid = tid++;


		SLIST_INSERT_HEAD(&head, thread_param, entries);

		int rc = pthread_create(&thread_param->tid, NULL, thread_sock, (void*)thread_param);

		if(rc!=0){
			printf("Error with thread creation");
			exit(1);
		}

		SLIST_FOREACH(thread_param, &head, entries){	
			pthread_join(thread_param->tid, NULL);
		}

		DEBUG_LOG("CLOSED ip address: %s", clientip);
	}

	DEBUG_LOG("End loop, start cleaning up!");

	pthread_mutex_destroy(&mutex);

	if (remove(FILE_PATH) != 0){
		ERROR_LOG("Delete data file");
	}

	//SLIST_FOREACH(thread_param, &head, entries){	
	//	pthread_join(thread_param->tid, NULL);
	//}

	while (!SLIST_EMPTY(&head)) {
        thread_param = SLIST_FIRST(&head);        
        SLIST_REMOVE_HEAD(&head, entries);
        free(thread_param);
    }

    close(sockfd);
    closelog();
    DEBUG_LOG("Gracefully Exited");

    exit(0);
}

/*********************************************************************
 * Functions Declaration
 *********************************************************************/
void signal_handler(int sig){
	caught_sigint = true;
	if((sig == SIGINT)||(sig == SIGTERM)){
		//syslog(LOG_DEBUG,"Caught signal, exiting");
		DEBUG_LOG( "Caught signal, exiting");
	}
	shutdown(sockfd,SHUT_RDWR);
}

int writer(char* writefile, char* writestr, size_t len){
	int fd;
	size_t count = len;
	ssize_t nr;

	// 1. open/create (if exist then append)
	fd = open(writefile, O_RDWR|O_CREAT|O_APPEND, S_IRWXU|S_IRWXG|S_IRWXO);
    if(fd == -1){
		ERROR_LOG("write: invalid path");   
		return(-1);                  
    }
    
	// 2. write
    DEBUG_LOG("Writing: %s", writestr);
	
	nr = write(fd, writestr, count);
	if (nr == -1){
        perror("write");	
		exit(1);
	}

	// 3. close
	close(fd);
	
	return count;
}

void* thread_sock(void* arg){
	char *buffer = (char*) malloc(BUFFER_SIZE*sizeof(char));
	//int curr_loc = 0;
  	int sum_len=0, recv_len, send_len;
	int writed_len,read_len;
    int fd; 
	
	// load data from client
	while(1){

		if(sum_len==0){
			recv_len = recv(sockfd_new , buffer+sum_len, BUFFER_SIZE, 0);
			sum_len += recv_len;
			break;
		}
		if(buffer[sum_len-1] == '\n'){
			buffer[sum_len] = '\0';
			break;
		}
		recv_len = recv(sockfd_new , buffer+sum_len, BUFFER_SIZE, 0);
		sum_len += recv_len;
	}
    if(recv_len <0){
    	DEBUG_LOG("no received data");
    }

	total_len += sum_len;

	// write data into file
	pthread_mutex_lock(&mutex);
	writed_len = writer(FILE_PATH, buffer, sum_len);
	if(writed_len<0){
		ERROR_LOG("at write()");
	}
	pthread_mutex_unlock(&mutex);

	// read data from file
	sum_len = 0;
	fd = open(FILE_PATH, O_RDWR, NULL);
	read_len = read(fd, buffer, total_len);
	if(read_len<0){
		ERROR_LOG("at read()");
	}
	else{
		DEBUG_LOG("Read from file, len = %d",read_len);
	}
	buffer[total_len] = '\0';

	// send
	send_len = send(sockfd_new, buffer, strlen(buffer), 0);
	if(send_len<0)
	{
		ERROR_LOG("Send error");
	}
	else
	{
		DEBUG_LOG("sent len = %d",send_len);
	}
	
	// Clean up
	free(buffer);
	close(fd);

	return NULL;
}

#if !USE_AESD_CHAR_DEVICE
void timerfunc(){
    int len = 0;

    time_t rawtime;
    struct tm *info;

    char timestr[32];
    char writestr[50]; 

    time( &rawtime );
    info = localtime( &rawtime );
    strftime(timestr,32,"%a %b %d %Y %T", info);

    strcpy(writestr,"timestamp:");
    strcat(writestr,timestr);
    strcat(writestr,"\n");
    
    len = strlen(writestr);

    pthread_mutex_lock(&mutex);
    writer(FILE_PATH,writestr,len);
    pthread_mutex_unlock(&mutex);

    total_len += len; // update total length

}
#endif

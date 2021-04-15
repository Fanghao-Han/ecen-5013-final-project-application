
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

#include <sys/stat.h>

#include <time.h>

#include <wiringPi.h>
#include <lcd.h>

#define MAX 64
#define PORT 9000
#define SA struct sockaddr

#define FILE_PATH "/var/tmp/socketdata.txt"

#define LCD_RS  25               //Register select pin
#define LCD_E   24               //Enable Pin
#define LCD_D4  23               //Data pin 4
#define LCD_D5  22               //Data pin 5
#define LCD_D6  21               //Data pin 6
#define LCD_D7  14               //Data pin 7
#define MAXTIMINGS 85
#define DHTPIN      7
#define OUTPUTPIN   29

#define DEBUG_PIN_OUT_ENABLE 1
#if DEBUG_PIN_OUT_ENABLE
#define DEBUG_PIN     0
#define DEBUG_PIN_LOW()    digitalWrite( DEBUG_PIN, LOW )
#define DEBUG_PIN_HI()     digitalWrite( DEBUG_PIN, HIGH )
#endif//DEBUG_PIN_OUT_ENABLE

#define WAIT_DHTPIN_HI()   l =50000; while (digitalRead(DHTPIN) != HIGH && (l-- > 0))//wait hi
#define WAIT_DHTPIN_LOW()  l = 50000; while (digitalRead(DHTPIN) != LOW && (l-- > 0))//wait low


#define DEBUG_LOG(msg,...) printf("server.c| " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("server.c|ERROR| " msg "\n" , ##__VA_ARGS__)

typedef struct {
	int unique_id;
	int tmp;
} data;

char temp_str[16];

char buff[MAX]; 
size_t total_len = 0;

uint32_t l;
int lcd;
int fan_status = 0;

void func(int sockfd);
void timerfunc(); 
int writer(char* writefile, char* writestr, size_t len);
void read_dht11_dat();

int main(int argc, char *argv[]) 
{ 
	int sockfd, i; 
	struct sockaddr_in servaddr;
	const char server_ip[] = "192.168.1.106";
	
	for(i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-d") == 0)
		{
			printf("Daemon\n\r");
			daemon(1, 1);
		}	
	}
	
	wiringPiSetup();
    pinMode (OUTPUTPIN, OUTPUT);
    lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);
    
    fan_status = 0;
    digitalWrite(OUTPUTPIN,LOW);
    printf("init: turn off\n");
	
	
	// socket create and varification 
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
	servaddr.sin_addr.s_addr = inet_addr(server_ip); 
	servaddr.sin_port = htons(PORT); 

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else
		printf("connected to the server %s\n",server_ip); 

	while(1)
	{
		read_dht11_dat();
		
		// function for chat 
		if(strlen(buff)>0)
			func(sockfd);
		
		sleep(1);
	}

	// close the socket 
	close(sockfd); 
}


/*	Function to send and receive data	*/
void func(int sockfd) 
{
	write(sockfd, buff, sizeof(buff)); 
	bzero(buff, sizeof(buff));  
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
    //DEBUG_LOG("Writing: %s", writestr);
	
	nr = write(fd, writestr, count);
	if (nr == -1){
        perror("write");	
		exit(1);
	}

	// 3. close
	close(fd);
	
	return count;
}

void timerfunc(){
    int len = 0;

    time_t rawtime;
    struct tm *info;

    char timestr[32];
    char writestr[50]; 

    time( &rawtime );
    info = localtime( &rawtime );
    strftime(timestr,32,"%a %b %d %Y %T", info);

    strcat(writestr,timestr);
    strcat(writestr,"\n");
    
    len = strlen(writestr);

    //pthread_mutex_lock(&mutex);
    writer(FILE_PATH,writestr,len);
    //pthread_mutex_unlock(&mutex);
	
	//sprintf(buff, "%s", writestr);
	bzero(buff,sizeof(buff));
	strcat(buff,writestr);
    total_len += len; // update total length

}

void read_dht11_dat()
{
	
   uint8_t i;
      uint32_t t1,t2;
      int dht11_dat[5] = { 0 };
      
      int dht11_bittime[40] = { 0 };
#if DEBUG_PIN_OUT_ENABLE
      pinMode(DEBUG_PIN, OUTPUT);
      DEBUG_PIN_LOW();
#endif//DEBUG_PIN_OUT_ENABLE


//------
//start handshake
   pinMode( DHTPIN, OUTPUT );
   digitalWrite( DHTPIN, LOW );
   delay( 18 );
#if DEBUG_PIN_OUT_ENABLE
      DEBUG_PIN_HI();
#endif//DEBUG_PIN_OUT_ENABLE

   digitalWrite( DHTPIN, HIGH );
   delayMicroseconds( 40 );
   /* prepare to read the pin */
   pinMode( DHTPIN, INPUT );


#if DEBUG_PIN_OUT_ENABLE
      DEBUG_PIN_LOW();
#endif//DEBUG_PIN_OUT_ENABLE
      WAIT_DHTPIN_HI();
//end handshake
//--------



#if DEBUG_PIN_OUT_ENABLE
      DEBUG_PIN_HI();
#endif//DEBUG_PIN_OUT_ENABLE

      WAIT_DHTPIN_LOW();
      for (i = 0; i < 40; i++)
      {
             WAIT_DHTPIN_HI();
#if DEBUG_PIN_OUT_ENABLE
             DEBUG_PIN_LOW();
#endif//DEBUG_PIN_OUT_ENABLE

			
             t1 = micros();
             WAIT_DHTPIN_LOW();
             
             t2 = micros();
#if DEBUG_PIN_OUT_ENABLE
             DEBUG_PIN_HI();
#endif//DEBUG_PIN_OUT_ENABLE
             dht11_bittime[i] = t2 - t1;
      }
#if DEBUG_PIN_OUT_ENABLE
      DEBUG_PIN_HI();
      delay(1);
      DEBUG_PIN_LOW();
      delay(1);
      DEBUG_PIN_HI();
#endif//DEBUG_PIN_OUT_ENABLE
     
      
      for (int i = 0; i < sizeof(dht11_bittime) / sizeof(int); i++)
      {
             dht11_dat[i / 8] <<= 1;

             if (dht11_bittime[i] > 60)
             {
                    dht11_dat[i / 8] |= 1;
             }
      }
      
      if (dht11_dat[4] == ( (dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF) )
	{
       
        if((dht11_dat[2]>23) && fan_status==0){
            digitalWrite(OUTPUTPIN,HIGH);
            fan_status = 1;
            printf("turn on\n");
        }
        else if((dht11_dat[2]<=23) && fan_status==1){     
            digitalWrite(OUTPUTPIN,LOW);
            fan_status = 0;
            printf("turn off\n");
        }
        
		sprintf(temp_str,"Temp: %d.%d C",
            dht11_dat[2], dht11_dat[3]);
		
        printf( "%s\n",temp_str );
  
        lcdPosition(lcd, 0, 0);      
        if(fan_status)
            lcdPrintf(lcd, "Fan Status: ON\n");
        else
            lcdPrintf(lcd, "Fan Status: OFF\n");

        lcdPosition(lcd, 0, 1);
        lcdPrintf(lcd, "Temp: %d.%d C @ ", dht11_dat[2],dht11_dat[3]);
		
		bzero(buff,sizeof(buff));
		strcpy(buff,temp_str);
		strcat(buff," @ ");
		//strcat(buff,);
		
		
		//timerfunc();
		time_t rawtime;
		struct tm *info;

		char timestr[32];
		//char writestr[50]; 

		time( &rawtime );
		info = localtime( &rawtime );
		strftime(timestr,32,"%a %b %d %Y %T", info);

		strcat(buff,timestr);
		strcat(buff,"\n");
 
	}
}
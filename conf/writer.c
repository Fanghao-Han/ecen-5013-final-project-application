/*
 * Setup syslog logging for your utility using the LOG_USER facility.
 * Use the syslog capability to write a message “Writing <string> to 
 * <file>” where <string> is the text string written to file (second  
 * argument) and <file> is the file created by the script.  
This should be written with LOG_DEBUG level.
Use the syslog capability to log any unexpected errors with LOG_ERR level.

 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>	// header for write(),close()
#include <string.h>
#include <syslog.h>

#define MODE 0644 // owner(rd+wr),group(rd),everyone(rd)

#if 1
	#define PRINT printf
#else
	#define PRINT ;
#endif

/*
 * Application Entry
 * Write a string to the file.
 * - First Argument is a full path to a file (including filename) on the filesystem
 * - Second Argument is a text string which will be written within this file
 */

int main(int argc, char* argv[]){

	int fd;
	size_t count;
	ssize_t nr;

	openlog(NULL,0,LOG_USER);

	// Inputs pre-check, 
	// this step will ensure the two args are not empty.
	if(argc<3){ // Missing argument
		PRINT("Error: Missing arguments, need 2 but have %d\n",(argc-1));
		fd=open(argv[1],O_RDONLY);
		if(fd==-1){
			PRINT("Error: Invalid writefile path\n");
			syslog(LOG_ERR,"Error: Invalid writefile path!");
		}
		if((argc==1)|(argv[2]==NULL)){
			PRINT("Error: Invalid writestr\n");
			syslog(LOG_ERR,"Error: Invalid writestr!");
		}	
		
		close(fd);
		closelog();
		return 1;
	}	
	
	const char* writefile = argv[1];
	const char* writestr = argv[2];

	// 0. Debug msg, will be saved in "/var/log/syslog"
	syslog(LOG_DEBUG,"Writing %s to %s", writestr, writefile);
	
	// 1. open/create (if exist then overwrite)
	fd = creat (writefile, MODE);
	if((fd == -1)|(writestr==NULL)){
		if(fd == -1){
			PRINT("Error: Invalid writefile path\n");
			syslog(LOG_ERR,"Error: Invalid writefile path!");
		}
		closelog();
		return 1;
	}

	// 2. write
	count = strlen(writestr);
	nr = write (fd, writestr, count);
	if (nr == -1){
		PRINT("\tError: Write\n");
		syslog(LOG_ERR,"Error: Write string failed!");
		closelog();
		return 1;
	}
/*
	else if (nr != count){
		PRINT("\tWARNING: Insufficient writing, %ld/%ld byte\n",nr,count);		
	}
*/
	// 3. close
	close(fd);
	closelog();
	
	return 0;
}

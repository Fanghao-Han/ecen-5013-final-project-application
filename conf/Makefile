CROSS_COMPLE=gcc
CC=$(CROSS_COMPILE)

#CROSS_COMPILE=arm-unknown-linux-gnueabi-gcc

CFLAGS=-g -Wall -Werror


all: 
	$(CC) $(CFLAGS) writer.c -o writer -lpthread
	$(CC) $(CFLAGS) aesdsocket.c -o aesdsocket -lpthread	

writer:
	$(CC) $(CFLAGS) writer.c -o writer -lpthread

socket:
	$(CC) $(CFLAGS) server/aesdsocket.c -o server/aesdsocket -lpthread

clean:
	rm -f *.o writer aesdsocket ./server/aesdsocket
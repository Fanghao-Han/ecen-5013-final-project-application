CROSS_COMPILE=gcc
CC=$(CROSS_COMPILE)

#CROSS_COMPILE=arm-unknown-linux-gnueabi-gcc

CFLAGS=-g -Wall -Werror


all: 
	$(CC) $(CFLAGS) aesdsocket.c -o aesdsocket -lpthread -lrt

socket:
	$(CC) $(CFLAGS) aesdsocket.c -o aesdsocket.o -lpthread -lrt

dht:
	$(CC) dht.c -o dht -lwiringPi -lwiringPiDev

clean:
	rm -f aesdsocket dht 

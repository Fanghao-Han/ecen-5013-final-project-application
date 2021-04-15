
ifeq ($(CC),)
	  CC = $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	 CFLAGS = -g -Wall -Werror
endif

#CFLAGS += -D DEBUG

all: server client
	
server: dht_server.c
	$(CC) $(CFLAGS) $(INCLUDES) dht_server.c -o server -pthread -lrt

client: dht_client.c
	$(CC) $(CFLAGS) $(INCLUDES) dht_client.c -o client -pthread -lrt -lwiringPi -lwiringPiDev

clean:	
	rm -rf *o client server

CROSS_COMPILE=gcc
#CROSS_COMPILE=arm-unknown-linux-gnueabi-gcc
ifeq ($(CC),)
	CC = $(CROSS_COMPILE)
endif

ifeq ($(CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lrt
endif


all: 
	$(CC) $(CFLAGS) writer.c -o writer $(LDFLAGS)
	$(CC) $(CFLAGS) server/aesdsocket.c -o aesdsocket $(LDFLAGS)
	$(CC) dht/dht.c -o dht -lwiringPi -lwiringPiDev
writer:
	$(CC) $(CFLAGS) writer.c -o writer $(LDFLAGS)
socket:
	$(CC) $(CFLAGS) server/aesdsocket.c -o aesdsocket $(LDFLAGS)
dht:
	$(CC) $(CFLAGS) dht11/dht.c -o dht -lwiringPi -lwiringPiDev
clean:
	rm -f *.o writer aesdsocket dht

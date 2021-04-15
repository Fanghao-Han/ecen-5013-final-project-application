CROSS_COMPILE=gcc
#CROSS_COMPILE=arm-unknown-linux-gnueabi-gcc
ifeq ($(CC),)
	CC = $(CROSS_COMPILE)
endif

ifeq ($(CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lrt -lwiringPi -lwiringPiDev
endif

CFLAGS += -D DEBUG

all: socket dht
socket:
	$(CC) $(CFLAGS) server/aesdsocket.c -o aesdsocket $(LDFLAGS)
dht:
	$(CC) $(CFLAGS) dht11/dht.c -o dht -lwiringPi -lwiringPiDev
clean:
	rm -f *.o aesdsocket dht

#ifndef _PACKETDEF
#define _PACKETDEF

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>

#define CHAR_SIZE 1
#define INT_SIZE 4

#define PORT 8888 //change if required
#define SERVERIP "127.0.0.1" //change if required
#define TIMEOUT 2 //2 secs

#define PAYLOAD 100

#define MAXPENDING 5

#define PDR 10 //change packet drop rate here - cannot be 0

#define DESTFILENAME "destination.txt" //change destination file name here as reqd


struct packet_struct
{
	int payload_size;
	int seqnum;
	int isLast;
	int isAck;
	int channel;
	char data[PAYLOAD + 1];
};

typedef struct packet_struct* packet;

struct packet_list {
	packet pckt;
	struct packet_list *next_packet;
};

typedef struct packet_list* plist;

extern plist head, tail;


#endif
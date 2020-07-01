#ifndef _PACKETDEF
#define _PACKETDEF

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>


#define CHARSIZE 1
#define INT_SIZE 4

#define RELAYPORT_0 12345
#define RELAYPORT_1 54321
#define SERVERPORT 8888

#define RELAYIP_0 "127.0.0.1"
#define RELAYIP_1 "127.0.0.1"
#define SERVERIP "127.0.0.1"

#define WINDOWSIZE 6

#define TIMEOUT 5 //seconds

#define PAYLOAD 100 //number of characters

#define DESTFILENAME "destination.txt" //change destination file name here as reqd

#define MAXPENDING 5

#define MAXDELAY 2 //maximum delay time in milliseconds 

#define PDR 10 //cant be <=0 and >100


extern int DATA_PACKET_NUMBER;

struct packet_struct
{
	int pcktnum; //-1 for acks, packet number otherwise
	int payload_size;
	int seqnum;
	int isLast;
	int isAck;
	int channel;
	char data[PAYLOAD+1];
};

typedef struct packet_struct* packet;

struct packet_list {
	packet pckt;
	struct packet_list *next_packet;
};

typedef struct packet_list* plist;

extern plist head, tail;


#endif
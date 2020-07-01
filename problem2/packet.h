#ifndef _PACKET
#define _PACKET

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>
#include "packetDef.h"

extern packet createNewPacket(int ps, int seq, int iL, int iA, int c, char *str);
plist createPlistNode(packet p);
extern void insertSorted(packet p);
extern packet getNext();
extern char* timeprint();


#endif
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "packet.h"

plist head, tail;

int main(){

	/*********** VARIABLES **********/

	head = NULL;
	tail = NULL;

	fd_set clientfds;
	int maxsd = -1;
	int activity;

	packet precv0 = (packet) malloc(sizeof(struct packet_struct));
	packet precv1 = (packet) malloc(sizeof(struct packet_struct));
	packet precv = (packet) malloc(sizeof(struct packet_struct));

	packet pack0, pack1;
	packet pack;

	struct sockaddr_in serveraddr, relayaddr_0, relayaddr_1, recvaddr;
	int relayaddrlen_0 = sizeof(relayaddr_1);
	int relayaddrlen_1 = sizeof(relayaddr_0);
	int serveraddrlen = sizeof(serveraddrlen);
	int recvaddrlen = sizeof(recvaddr);

	int serversocket, channel0, channel1;
	int bindval;

	int expected = 0;

	int recvbytes_even;
	int recvbytes_odd;

	int bytes_sent, bytes_recved;

	/**********connections*********/

	FILE *fp;
	fp = fopen(DESTFILENAME, "w");
	if(fp == NULL){
		printf("Error while opening %s\n", DESTFILENAME);
		exit(0);
	}

	serversocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serversocket<0){
		printf ("Error in Opening a socket\n");
		exit(0);
	}
	printf ("server Socket created\n");

	memset (&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT); //port number can be changed
	serveraddr.sin_addr.s_addr = htonl (INADDR_ANY); //change server IP address here
	printf("server address assigned\n");

	bindval = bind(serversocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (bindval<0){
		printf("Error while binding\n");
		exit(0);
	}
	printf("binding successful\n");

	while(1){

		bytes_recved = recvfrom(serversocket, precv, sizeof(struct packet_struct),0, (struct sockaddr *) &recvaddr, &recvaddrlen);
		if(bytes_recved == 0) break;
		if(bytes_recved<0){
			printf("Error in recv\n");
			exit(0);
		}
		printf("RECV PCKT: Seq no - %d of size %d from relay %d at time - %s\n", precv->seqnum, bytes_recved, precv->pcktnum%2, timeprint());

		if(expected == precv->seqnum){

			fprintf(fp, "%s", precv->data);
			expected = expected + precv->payload_size;

			while(head != NULL && head->pckt->seqnum == expected){
				packet node = getNext();
				if(node == NULL){
					printf("error in deque\n");
					exit(0);
				}
				fprintf(fp, "%s", node->data);
				expected = expected + node->payload_size;
			}
		}
		else if(expected>precv->seqnum){
			//printf("Duplicate packet of seq num %d\n", precv->seqnum);
			//duplicate
			continue;
		}
		else{
			// expected < seqnum means enque the packet
			//enque(0, precv0);
			insertSorted(precv);
		}
		pack = precv;
		pack->isAck = 1;
		int r = precv->pcktnum%2;
		pack->pcktnum = -1;


		int bytes_sent = sendto(serversocket, pack, sizeof(struct packet_struct),0, (struct sockaddr*) &recvaddr, recvaddrlen);
		if (bytes_sent!=sizeof(struct packet_struct)){
			printf("error while sending msg\n");
			exit(0);
		}
		printf("SENT ACK: Seq no - %d of size %d to relay %d at time - %s\n", pack->seqnum, bytes_sent, r, timeprint());
	}
	close(serversocket);
	fclose(fp);




}

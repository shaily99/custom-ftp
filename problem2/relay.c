#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "packet.h"

int main(){

	/***************************************** VARIABLES ***************************************************/
	int relaynumber = 0;

	struct sockaddr_in myaddr, serveraddr, recvaddr, clientaddr;
	int myaddrlen = sizeof(myaddr);
	int serveraddrlen = sizeof(serveraddr);
	int recvaddrlen = sizeof(recvaddr);
	int clientaddrlen = sizeof(clientaddr);

	int relaysocket;
	int bindval;

	packet precv = (packet) malloc(sizeof(struct packet_struct));
	int bytes_recved, bytes_sent;

	srand(time(NULL));
	int x = 100/PDR;

	/***************************************** Connections ***************************************************/

	printf("Enter 0 to run as Relay 0 and Enter 1 to run as Relay 1\n");
	scanf("%d", &relaynumber);

	relaysocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(relaysocket<0){
		printf("error in opening socket\n");
		exit(0);
	}
	printf("socket created\n");

	memset (&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT); //port number can be changed
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP); //change server IP address here
	printf("server address assigned\n");

	memset (&myaddr, 0, sizeof(serveraddr));
	myaddr.sin_family = AF_INET;
	if(relaynumber == 0){
		myaddr.sin_port = htons(RELAYPORT_0); //port number can be changed
		myaddr.sin_addr.s_addr = htonl(INADDR_ANY); //change server IP address here
	}
	else{
		myaddr.sin_port = htons(RELAYPORT_1); //port number can be changed
		myaddr.sin_addr.s_addr = htonl(INADDR_ANY); //change server IP address here	
	}	
	
	printf("relay address assigned\n");

	bindval = bind(relaysocket, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if (bindval<0){
		printf("Error while binding\n");
		exit(0);
	}
	printf("binding successful\n");

	bytes_recved = recvfrom(relaysocket, precv, sizeof(struct packet_struct),0, (struct sockaddr *) &clientaddr, &clientaddrlen);
	if(bytes_recved == 0){
		//socket closed 
		close(relaysocket);
		return 0;
	}
	if(bytes_recved<0){
		printf("error in recieve\n");
		exit(0);
	}

	printf("RECVED by Relay %d from client: DATA Packet with Seq num %d at time - %s\n", relaynumber, precv->seqnum, timeprint());

	usleep(rand()%(MAXDELAY*1000));

	bytes_sent = sendto(relaysocket, precv, sizeof(struct packet_struct), 0, (struct sockaddr *) &serveraddr, serveraddrlen);
	if(bytes_sent != sizeof(struct packet_struct)){
		printf("error in send\n");
		exit(0);
	}

	printf("SEND by relay %d to Server: DATA Packet with Seq num - %d at time - %s\n", relaynumber, precv->seqnum, timeprint() );


	while (1){
		bytes_recved = recvfrom(relaysocket, precv, sizeof(struct packet_struct),0, (struct sockaddr *) &recvaddr, &recvaddrlen);

		if(bytes_recved == 0){
			//socket closed 
			break;
		}
		if(bytes_recved<0){
			printf("error in recieve\n");
			exit(0);
		}

		

		if(precv->isAck == 1){
			//ack recieved from server send it to client
			//introduce delay
			printf("RECVED by Relay %d from Server: ACK Packet with Seq num %d at time - %s\n", relaynumber, precv->seqnum, timeprint());

			bytes_sent = sendto(relaysocket, precv, sizeof(struct packet_struct), 0, (struct sockaddr *) &clientaddr, clientaddrlen);
			if(bytes_sent != sizeof(struct packet_struct)){
				printf("error in send\n");
				exit(0);
			}

			printf("SEND by relay %d to Client: ACK Packet with Seq num - %d at time - %s\n", relaynumber, precv->seqnum, timeprint() );


		}
		else{
			if(((rand()%100)%x) == 0) continue; //random drop
			//send to server
			printf("RECVED by Relay %d from client: DATA Packet with Seq num %d at time - %s\n", relaynumber, precv->seqnum, timeprint());

			usleep(rand()%(MAXDELAY*1000));

			bytes_sent = sendto(relaysocket, precv, sizeof(struct packet_struct), 0, (struct sockaddr *) &serveraddr, serveraddrlen);
			if(bytes_sent != sizeof(struct packet_struct)){
				printf("error in send\n");
				exit(0);
			}

			printf("SEND by relay %d to Server: DATA Packet with Seq num - %d at time - %s\n", relaynumber, precv->seqnum, timeprint());

		}

	}
	int *a = NULL;
	int b = sendto(relaysocket, a, 0,0, (struct sockaddr *) &serveraddr, serveraddrlen);
	close(relaysocket);
	return 0;
}
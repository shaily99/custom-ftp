#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>
#include "packet.h"

plist head, tail;

int main(){

	/*********** VARIABLES **********/

	head = NULL;
	tail = NULL;

	int serversocket;
	int listenfd;
	int bindval;
	int channel0, channel1;

	struct sockaddr_in serveraddr, clientaddr0, clientaddr1;
	int clientlen0 = sizeof(clientaddr0);
	int clientlen1 = sizeof(clientaddr1);


	fd_set clientfds;
	int maxsd = -1;
	int activity;

	int recvbytes_even;
	int recvbytes_odd;

	packet precv0 = (packet) malloc(sizeof(struct packet_struct));
	packet precv1 = (packet) malloc(sizeof(struct packet_struct));

	packet pack0, pack1;

	int expected = 0;

	int x = 100/PDR;

	srand(time(NULL));


	/**********Socket creation, binding and listening*********/

	serversocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serversocket<0){
		printf ("Error in Opening a socket\n");
		exit(0);
	}
	printf ("server Socket created\n");

	memset (&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT); //port number can be changed
	serveraddr.sin_addr.s_addr = htonl (INADDR_ANY); //change server IP address here
	printf("server address assigned\n");

	bindval = bind(serversocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (bindval<0){
		printf("Error while binding\n");
		exit(0);
	}
	printf("binding successful\n");

	listenfd = listen(serversocket, MAXPENDING);
	if (listenfd<0){
			printf("error while listening\n");
			exit(0);
		}
	printf("now listening for first channel of client\n");

	channel0 = accept(serversocket, (struct sockaddr*) &clientaddr0, &clientlen0);
	if (clientlen0<0){
		printf("error in client socket channel0\n");
		exit(0);
	}
	printf(" channel0 connected\n");

	printf("now listening for second channel of client\n");

	channel1 = accept(serversocket, (struct sockaddr*) &clientaddr1, &clientlen1);
	if (clientlen1<0){
		printf("error in client socket channel1\n");
		exit(0);
	}
	printf(" channel1 connected\n");



	FILE *fp;
	fp = fopen(DESTFILENAME, "w");
	if(fp == NULL){
		printf("Error while opening %s\n", DESTFILENAME);
		exit(0);
	}

	while(1){

		FD_ZERO(&clientfds);
		FD_SET(channel0, &clientfds);
		if(channel0>maxsd)
			maxsd = channel0;
		FD_SET(channel1, &clientfds);
		if(channel1>maxsd)
			maxsd = channel1;

		activity = select(maxsd+1, &clientfds, NULL, NULL, NULL);

		if ((activity < 0))   
		{   
			printf("select error\n");
			exit(0);   
		}

		if(FD_ISSET(channel0, &clientfds)){

			recvbytes_even = recv(channel0, precv0, sizeof(struct packet_struct),0);

			if(recvbytes_even == 0){
				//close connection
				break;
			}
			if(recvbytes_even<0){
				printf("Error in recv channel0\n");
				exit(0);
			}

			if((rand()%100)%x == 0) continue; //random pckt drop

			printf("RECV PCKT: Seq no - %d of size %d from channel %d\n", precv0->seqnum, recvbytes_even, 0);

			//check whether to write of buffer
			if(expected == precv0->seqnum){
				fprintf(fp, "%s", precv0->data);
				expected = expected + precv0->payload_size;

				while(head != NULL && head->pckt->seqnum == expected){
					packet node = getNext();
					if(node == NULL){
						printf("error in deque\n");
						exit(0);
					}
					fprintf(fp, "%s", node->data);
					expected = expected + node->payload_size;
				}

				// while((head_even != NULL && head_even->pckt->seqnum == expected) || (head_odd != NULL && head_odd->pckt->seqnum == expected)){
				// 	plist node;
				// 	if(head_even != NULL && head_even->pckt->seqnum == expected) node = deque(0);
				// 	else if(head_odd != NULL && head_odd->pckt->seqnum == expected) node = deque(1);
					
				// 	if(node == NULL){
				// 		printf("deq error\n");
				// 		exit(0);
				// 	}
				// 	fprintf(fp, "%s\n", node->pckt->data);
				// 	expected = expected + node->pckt->payload_size;
				// }
				// if(head_odd != NULL){
				// 	while(expected == head_odd->pckt->seqnum){
				// 		plist node = deque(1);
				// 		if(node == NULL){
				// 			printf("deq error\n");
				// 			exit(0);
				// 		}
				// 		fprintf(fp, "%s\n", node->pckt->data);
				// 		expected = expected + node->pckt->payload_size;
				// 	}
				// }
			}
			else if(expected>precv0->seqnum){
				printf("Duplicate packet of seq num %d\n", precv0->seqnum);
				//duplicate
				continue;
			}
			else{
				// expected < seqnum means enque the packet
				//enque(0, precv0);
				insertSorted(precv0);
			}


			pack0 = precv0;
			pack0->isAck = 1;

			int bytes_sent = send(channel0, pack0, sizeof(struct packet_struct),0);
			if (bytes_sent!=sizeof(struct packet_struct)){
				printf("error while sending msg\n");
				exit(0);
			}
			printf("SENT ACK: Seq no - %d of size %d from channel %d\n", pack0->seqnum, bytes_sent, 0);

		}
		if(FD_ISSET(channel1, &clientfds)){

			recvbytes_odd = recv(channel1, precv1, sizeof(struct packet_struct),0);
			//printf("recvbytes odd %d\n", recvbytes_odd);
			if(recvbytes_odd == 0){
				//printf("close1\n");
				//close
				break;
			}
			if(recvbytes_odd<0){
				printf("Error in recv channel1\n");
				exit(0);
			}

			if((rand()%100)%x == 0) continue; //random packet drop

			//check whether to write of buffer
			if(expected == precv1->seqnum){
				fprintf(fp, "%s", precv1->data);
				expected = expected + precv1->payload_size;

				while(head != NULL && head->pckt->seqnum == expected){
					packet node = getNext();
					if(node == NULL){
						printf("error in deque\n");
						exit(0);
					}
					fprintf(fp, "%s", node->data);
					expected = expected + node->payload_size;
				}

				// while((head_even != NULL && head_even->pckt->seqnum == expected) || (head_odd != NULL && head_odd->pckt->seqnum == expected)){
				// 	plist node;
				// 	if(head_even != NULL && head_even->pckt->seqnum == expected) node = deque(0);
				// 	else node = deque(1);
				// 	if(node == NULL){
				// 		printf("deq error\n");
				// 		exit(0);
				// 	}
				// 	fprintf(fp, "%s\n", node->pckt->data);
				// 	expected = expected + node->pckt->payload_size;
				// }
				// if(head_even != NULL){
				// 	while(expected == head_even->pckt->seqnum){
				// 		plist node = deque(0);
				// 		if(node == NULL){
				// 			printf("deq error\n");
				// 			exit(0);
				// 		}
				// 		fprintf(fp, "%s\n", node->pckt->data);
				// 		expected = expected + node->pckt->payload_size;
				// 	}
				// }
			}
			else if(expected>precv1->seqnum){
				printf("duplicate packet of seqnum %d\n", precv1->seqnum);
				//duplicate
				continue;
			}
			else{
				// expected < seqnum means enque the packet
				// enque(1, precv1);
				insertSorted(precv1);
			}

			printf("RECV PCKT: Seq no - %d of size %d from channel %d\n", precv1->seqnum, recvbytes_odd, 1);
			pack1 = precv1;
			pack1->isAck = 1;

			int bytes_sent = send(channel1, pack1, sizeof(struct packet_struct),0);
			if (bytes_sent!=sizeof(struct packet_struct)){
				printf("error while sending msg\n");
				exit(0);
			}
			printf("SENT ACK: Seq no - %d of size %d from channel %d\n", pack1->seqnum, bytes_sent, 1);

		}
	}
	while(head != NULL && head->pckt->seqnum == expected){
		packet node = getNext();
		if(node == NULL){
			printf("error in deque\n");
			exit(0);
		}
		fprintf(fp, "%s", node->data);
		expected = expected + node->payload_size;
	}
	close(channel0);
	close(channel1);
	close(serversocket);
	fclose(fp);
	return 0;
}
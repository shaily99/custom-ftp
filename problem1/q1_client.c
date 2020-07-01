#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include "packet.h"

int main(int argc, char const *argv[])
{

	/****************************************** VARIABLE DECLARATION****************************************************/

	int bytes_even, bytes_odd;

	int file_size;
	int ack0pending = 0;
	int ack1pending = 0;

	char srcfilename[100];
	char even_data[PAYLOAD + 1];
	char odd_data[PAYLOAD + 1];

	packet peven, podd;
	packet precv0 = (packet) malloc(sizeof(struct packet_struct));
	packet precv1 = (packet) malloc(sizeof(struct packet_struct));

	int channel0, channel1;

	fd_set clientfds;
	int maxsd = -1;
	int activity_client;

	struct timeval tout;
	tout.tv_sec = TIMEOUT;
	tout.tv_usec = 0;

	clock_t t0, t1, curr;
	double delta0, delta1;

	int offset = 0;

	int last = 0;

	int lastChannel = 0;

	int lastcame = 0;



	printf("Enter name of file you wish to upload: \n");
	scanf("%s", srcfilename);

	FILE *fp;
	fp = fopen(srcfilename, "r"); 
	if(NULL == fp)
	{
		printf("Error while opening file %s\n", srcfilename);
		exit(0);
	}
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);

	fp = fopen(srcfilename, "r");
	if(fp == NULL){
		printf("Error while opening file %s\n", srcfilename);
		exit(0);
	}

	
	
	/***************************************ADDRESS ASSIGNMENT AND CONNECTION**********************************************/

	struct sockaddr_in serveraddr;
	memset (&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT); //port number can be changed from the define macro PORT
	serveraddr.sin_addr.s_addr = inet_addr (SERVERIP); //change server IP address by changing the define macro SERVERIP
	printf("address assigned\n");

	

	channel0 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (channel0<0){
		printf ("Error in Opening a socket\n");
		exit(0);
	}
	printf ("Client Socket for channel0 created\n");

	channel1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (channel1<0){
		printf ("Error in Opening a socket\n");
		exit(0);
	}
	printf ("Client Socket for channel1 created\n");

	int c0 = connect(channel0, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
	printf("c0 = %d\n",c0);
	if (c0<0){
		printf("error while establishing connection of channel0\n");
		exit(0);int maxsd = -1;
	}
	printf("connection for channel0 established\n");

	int c1 = connect(channel1, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
	printf("c1 = %d\n",c1 );
	if (c1<0){
		printf("error while establishing connection of channel1\n");
		exit(0);
	}
	printf("connection for channel1 established\n");


	/************************************************SENDING FIRST PACKETS***************************************************/




	fseek(fp, offset, SEEK_SET);
	
	bytes_even = fread(even_data, CHAR_SIZE, PAYLOAD, fp);
	if(bytes_even>0){
		even_data[bytes_even] = '\0';
		if(bytes_even<PAYLOAD) last = 1;
		peven = createNewPacket(bytes_even, offset, last, 0, 0, even_data);

		int bytes_sent = send(channel0, peven, sizeof(struct packet_struct),0);
		if (bytes_sent!=sizeof(struct packet_struct)){
			printf("error while sending msg\n");
			exit(0);
		}
		printf("SENT PACKET: Seq no - %d of size %d from channel %d\n", peven->seqnum, bytes_sent, 0);

		ack0pending = offset;

		offset = offset + bytes_even;

		lastChannel = 0;
	}

	if(file_size<offset){
		printf("SENDING COMPLETE\n");
		//add code for recv here
		int bytesrcvd = recv(channel0, precv0, sizeof(struct packet_struct), 0);
		if (bytesrcvd<0){
			printf("error while recieving\n");
			exit(0);
		}
		if(precv0->isAck != 1 || precv0->seqnum != ack0pending){
			printf("wrong acknowledgement on channel0\n");
			exit(0);
		}
		else if(precv0->isAck == 1 && precv0->isLast == 1){
			printf("RECVD ACK: seq number - %d from channel 0\n", precv0->seqnum);
			ack0pending = -1;
			printf("last ack arrived\n");
			lastcame = 1;

		}

		else{


			printf("RECVD ACK: seq number - %d from channel 0\n", precv0->seqnum);
			ack0pending = -1;
		}


		return 0;
	}

	fseek(fp, offset, SEEK_SET);

	bytes_odd = fread(odd_data, CHAR_SIZE, PAYLOAD, fp);

	if(bytes_odd>0){
		odd_data[bytes_odd] = '\0';
		if(bytes_odd<PAYLOAD) last = 1;
		podd = createNewPacket(bytes_odd, offset, last, 0, 1, odd_data);

		int bytes_sent = send(channel1, podd, sizeof(struct packet_struct),0);
		if (bytes_sent!=sizeof(struct packet_struct)){
			printf("error while sending msg\n");
			exit(0);
		}
		printf("SENT PACKET: Seq no - %d of size %d from channel %d\n", podd->seqnum, bytes_sent, 1);

		ack1pending = offset;
		offset = offset + bytes_odd;
		lastChannel = 1;
	}


	while(1){
		if(lastcame == 1 && ack0pending == -1 && ack1pending == -1) break;

		tout.tv_sec = TIMEOUT;
		tout.tv_usec = 0;

		FD_ZERO(&clientfds);
		FD_SET(channel0, &clientfds);
		if(channel0>maxsd)
			maxsd = channel0;
		FD_SET(channel1, &clientfds);
		if(channel1>maxsd)
			maxsd = channel1;

		activity_client = select(maxsd+1, &clientfds, NULL, NULL, &tout);

		if ((activity_client < 0))   
		{   
			printf("select error\n");
			exit(0);   
		} 
		if(activity_client == 0){
			//timeout send last two packets again
			int toutchannel;
			if(lastChannel == 0) toutchannel = 1;
			else toutchannel = 0;

			printf("TIMEOUT on channel %d\n", toutchannel);

			if(toutchannel == 0){
				int bytes_sent = send(channel0, peven, sizeof(struct packet_struct),0);
				if (bytes_sent!=sizeof(struct packet_struct)){
					printf("error while sending msg\n");
					exit(0);
				}
				printf("SENT PACKET: Seq no - %d of size %d from channel %d\n", peven->seqnum, bytes_sent, 0);

			}
			else if (toutchannel == 1){
				int bytes_sent = send(channel1, podd, sizeof(struct packet_struct),0);
				if (bytes_sent!=sizeof(struct packet_struct)){
					printf("error while sending msg\n");
					exit(0);
				}
				printf("SENT PACKET: Seq no - %d of size %d from channel %d\n", podd->seqnum, bytes_sent, 1);

			}	

				
		}

        if(FD_ISSET(channel0, &clientfds)){
        	//ack on channel0, send next even packet
        	
			int bytesrcvd = recv(channel0, precv0, sizeof(struct packet_struct), 0);
			if (bytesrcvd<0){
				printf("error while recieving\n");
				exit(0);
			}

			if(precv0->isAck != 1 || precv0->seqnum > ack0pending){
				printf("wrong acknowledgement on channel0\n");
				exit(0);
			}
			else if(precv0->isAck == 1 && precv0->isLast == 1){
				printf("RECVD ACK: seq number - %d from channel 0\n", precv0->seqnum);
				printf("last ack arrived\n");
				ack0pending = -1;
				lastcame = 1;
				if(ack1pending == -1)break;
				else continue;
			}

			else{				
				//send next

				
				if(ack0pending > precv0->seqnum){
					//dup ack
					continue;
				}
				else {
					printf("RECVD ACK: seq number - %d from channel 0\n", precv0->seqnum);
					ack0pending = -1;
					//if(ack1pending == -1) break;
				}

				if(file_size<offset){
					continue;
				}

				fseek(fp, offset, SEEK_SET);
	

				bytes_even = fread(even_data, CHAR_SIZE, PAYLOAD, fp);
				if(bytes_even>0){
					even_data[bytes_even] = '\0';
					if(bytes_even<PAYLOAD) last = 1;

					peven = createNewPacket(bytes_even, offset, last, 0, 0, even_data);

					int bytes_sent = send(channel0, peven, sizeof(struct packet_struct),0);
					if (bytes_sent!=sizeof(struct packet_struct)){
						printf("error while sending msg\n");
						exit(0);
					}
					//t0 = clock();
					printf("SENT PACKET: Seq no - %d of size %d from channel %d\n", peven->seqnum, bytes_sent, 0);

					ack0pending = offset;

					offset = offset + bytes_even;

					lastChannel = 0;
				}

			}

        }
        else if(FD_ISSET(channel1, &clientfds)){
        	//ack on channel1

			int bytesrcvd = recv(channel1, precv1, sizeof(struct packet_struct), 0);
			if (bytesrcvd<0){
				printf("error while recieving\n");
				exit(0);
			}

			if(precv1->isAck != 1 || precv1->seqnum > ack1pending){
				printf("wrong acknowledgement on channel1\n");
				exit(0);
			}
			else if(precv1->isAck == 1 && precv1->isLast == 1){
				printf("RECVD ACK: seq number - %d from channel 1\n", precv1->seqnum);
				ack1pending = -1;
				printf("last ack arrived\n");
				lastcame = 1;
				if(ack0pending = -1)break;
				else continue;
			}
			else{

				
				if(ack1pending > precv1->seqnum){
					//dup ack
					continue;
				}
				else{
					printf("RECVD ACK: seq number - %d from channel 1\n", precv1->seqnum);
					ack1pending = -1;
					//if(ack0pending = -1)break;

				} 
	
				if(file_size<offset){
					continue;
				}

				fseek(fp, offset, SEEK_SET);
	
				bytes_odd = fread(odd_data, CHAR_SIZE, PAYLOAD, fp);
				if(bytes_odd>0){
					odd_data[bytes_odd] = '\0';
					if(bytes_odd<PAYLOAD) last = 1;
					podd = createNewPacket(bytes_odd, offset, last, 0, 1, odd_data);

					// int bytes_sent = send(channel1, odd_data, strlen(even_data),0);
					int bytes_sent = send(channel1, podd, sizeof(struct packet_struct),0);
					if (bytes_sent!=sizeof(struct packet_struct)){
						printf("error while sending msg\n");
						exit(0);
					}
					//t1 = clock();
					printf("SENT PACKET: Seq no - %d of size %d from channel %d\n", podd->seqnum, bytes_sent, 1);

					ack1pending = offset;

					offset = offset + bytes_odd;

					lastChannel = 1;
				}

			}
        }
        else continue;
	}
	close(channel0);
	close(channel1);
	fclose(fp);

	return 0;
}
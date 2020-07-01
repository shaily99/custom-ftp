#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "packet.h"

int DATA_PACKET_NUMBER;

int main(){

	DATA_PACKET_NUMBER = 0;

	/********************************************** VARIABLES ******************************************************/

	struct sockaddr_in relay0, relay1;
	int raddrlen0, raddrlen1;

	int chnl = 0;
	int last = 0;
	int offset = 0;
	int file_size;

	int lastAck = 0;

	fd_set clientfds;
	int maxsd = -1;
	int activity_client;

	struct timeval tout;
	tout.tv_sec = TIMEOUT;
	tout.tv_usec = 0;

	packet pckt_window[WINDOWSIZE];
	int ack_pending[WINDOWSIZE]; //-2 if no pckt exists, -1 if pckt exists but not yet sent, 0 for pending 1 for arrived

	for(int i=0; i<WINDOWSIZE; i++){
		pckt_window[i] = NULL;
		ack_pending[i] = -2;
	}

	packet precv0 = (packet) malloc(sizeof(struct packet_struct));
	packet precv1 = (packet) malloc(sizeof(struct packet_struct));

	// char even_data[PAYLOAD];
	// char odd_data[PAYLOAD];
	char data[PAYLOAD+1];

	int channel0, channel1;
	//int bytes_even, bytes_odd;
	int bytes;

	char srcfilename[100];


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

	/********************************************** CONNECTIONS **************************************************/

	relay0.sin_family = AF_INET;
	relay0.sin_port = htons(RELAYPORT_0); //port number can be changed from the define macro RELAYPORT_0
	relay0.sin_addr.s_addr = inet_addr (RELAYIP_0); //change server IP address by changing the define macro RELAYIP_0
	printf("address assigned for relay 0\n");

	relay1.sin_family = AF_INET;
	relay1.sin_port = htons(RELAYPORT_1); //port number can be changed from the define macro RELAYPORT_1
	relay1.sin_addr.s_addr = inet_addr (RELAYIP_1); //change server IP address by changing the define macro RELAYIP_1
	printf("address assigned for relay 1\n");

	raddrlen0 = sizeof(relay0);
	raddrlen1 = sizeof(relay1);

	channel0 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (channel0<0){
		printf ("Error in Opening a socket\n");
		exit(0);
	}
	printf ("Client Socket for channel0 created\n");

	channel1 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (channel1<0){
		printf ("Error in Opening a socket\n");
		exit(0);
	}
	printf ("Client Socket for channel1 created\n");

	/************************************************ FIRST MSSG ***************************************************/

	for (int i = 0; i<WINDOWSIZE; i++){
		if(offset<file_size){
			fseek(fp, offset, SEEK_SET);
			bytes = fread(data, CHARSIZE, PAYLOAD, fp);
			
			if(bytes>0){
				data[bytes] = '\0';
				if(bytes<PAYLOAD) last = 1;
				chnl = DATA_PACKET_NUMBER%2;

				pckt_window[i] = createNewPacket(bytes, offset, last, 0, chnl, data);
				pckt_window[i]->pcktnum = DATA_PACKET_NUMBER;
				DATA_PACKET_NUMBER++;

				ack_pending[i] = -1;

				offset = offset + bytes;
			}

		}
		else {
			//file is over
			pckt_window[i] = NULL;
		}
	}

	for(int i = 0; i<WINDOWSIZE; i++){
		if(pckt_window[i] == NULL) break;
		if(pckt_window[i]->pcktnum%2 == 0){
			int bytes_sent = sendto(channel0, pckt_window[i], sizeof(struct packet_struct), 0, (struct sockaddr*) &relay0, raddrlen0);
			if(bytes_sent == -1){
				printf("error in sendto\n");
				exit(0);
			}
			printf("SENT PACKET: Seq no - %d of size %d from channel %d at time - %s\n", pckt_window[i]->seqnum, bytes_sent, 0, timeprint());
			ack_pending[i] = 1;
		}
		else{
			int bytes_sent = sendto(channel1, pckt_window[i], sizeof(struct packet_struct), 0, (struct sockaddr*) &relay1, raddrlen1);
			if(bytes_sent == -1){
				printf("error in sendto\n");
				exit(0);
			}
			printf("SENT PACKET: Seq no - %d of size %d from channel %d at time - %s\n", pckt_window[i]->seqnum, bytes_sent, 1, timeprint());
			ack_pending[i] = 1;
		}
	}

	while(1){
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
			//timeout 
			printf("TIMEOUT \n");

			for(int i = 0; i<WINDOWSIZE; i++){
				if(ack_pending[i] == 1){
					if(pckt_window[i]->pcktnum%2 == 0){
						int bytes_sent = sendto(channel0, pckt_window[i], sizeof(struct packet_struct), 0, (struct sockaddr*) &relay0, raddrlen0);
						if(bytes_sent == -1){
							printf("error in sendto\n");
							exit(0);
						}
						printf("SENT PACKET: Seq no - %d of size %d from channel %d at time - %s\n", pckt_window[i]->seqnum, bytes_sent, 0, timeprint());
						ack_pending[i] = 1;
					}
					else{
						int bytes_sent = sendto(channel1, pckt_window[i], sizeof(struct packet_struct), 0, (struct sockaddr*) &relay1, raddrlen1);
						if(bytes_sent == -1){
							printf("error in sendto\n");
							exit(0);
						}
						printf("SENT PACKET: Seq no - %d of size %d from channel %d at time - %s\n", pckt_window[i]->seqnum, bytes_sent, 1, timeprint());
						ack_pending[i] = 1;
					}

				}
				else{
					//ack is there. do nothing
					continue;
				}
			}
		}

		if(FD_ISSET(channel0, &clientfds)){
			//ack on channel0
			int bytesrcvd = recvfrom(channel0, precv0, sizeof(struct packet_struct), 0, (struct sockaddr*) &relay0, &raddrlen0);
			if (bytesrcvd<0){
				printf("error while recieving\n");
				exit(0);
			}
			if(precv0->isAck != 1){
				printf("not an ack on channel 0\n");
				exit(0);
			}
			int ind = -1;
			for (int i = 0; i < WINDOWSIZE; i++){
				if(pckt_window[i]->seqnum == precv0->seqnum){
					ind = i;
					break;
				}
			}
			if(ind == -1){
				printf("wrong ack on channel 0\n");
				exit(0);
			}
			ack_pending[ind] = 0;
			printf("RECVED ACK: Seq no - %d on channel %d\n", precv0->seqnum, 0);
			if(precv0->isLast == 1) lastAck = 1;

			int alldone = 1;
			for(int i = 0; i<WINDOWSIZE; i++){
				if(ack_pending[i] == 1){
					alldone = 0;
					break;
				}
			}
			if(alldone == 0) continue;
			else{
				if(lastAck == 1) break; //last ack has come and all in the window have

				//entire window has been completed send next set
				for (int i = 0; i<WINDOWSIZE; i++){
					if(offset<file_size){
						fseek(fp, offset, SEEK_SET);
						bytes = fread(data, CHARSIZE, PAYLOAD, fp);
						
						if(bytes>0){
							data[bytes] = '\0';
							if(bytes<PAYLOAD) last = 1;
							chnl = DATA_PACKET_NUMBER%2;

							pckt_window[i] = createNewPacket(bytes, offset, last, 0, chnl, data);
							pckt_window[i]->pcktnum = DATA_PACKET_NUMBER;
							DATA_PACKET_NUMBER++;

							ack_pending[i] = -1;

							offset = offset + bytes;
						}

					}
					else {
						//file is over
						pckt_window[i] = NULL;
					}
				}

				for(int i = 0; i<WINDOWSIZE; i++){
					if(pckt_window[i] == NULL) break;
					if(pckt_window[i]->pcktnum%2 == 0){
						int bytes_sent = sendto(channel0, pckt_window[i], sizeof(struct packet_struct), 0, (struct sockaddr*) &relay0, raddrlen0);
						if(bytes_sent == -1){
							printf("error in sendto\n");
							exit(0);
						}
						printf("SENT PACKET: Seq no - %d of size %d from channel %d at time - %s\n", pckt_window[i]->seqnum, bytes_sent, 0, timeprint());
						ack_pending[i] = 1;
					}
					else{
						int bytes_sent = sendto(channel1, pckt_window[i], sizeof(struct packet_struct), 0, (struct sockaddr*) &relay1, raddrlen1);
						if(bytes_sent == -1){
							printf("error in sendto\n");
							exit(0);
						}
						printf("SENT PACKET: Seq no - %d of size %d from channel %d at time - %s\n", pckt_window[i]->seqnum, bytes_sent, 1, timeprint());
						ack_pending[i] = 1;
					}
				}
			}

		}

		if(FD_ISSET(channel1, &clientfds)){
			//ack on channel1
			int bytesrcvd = recvfrom(channel1, precv1, sizeof(struct packet_struct), 0, (struct sockaddr*) &relay1, &raddrlen1);
			if (bytesrcvd<0){
				printf("error while recieving\n");
				exit(0);
			}
			if(precv1->isAck != 1){
				printf("not an ack on channel 1\n");
				exit(0);
			}
			int ind = -1;
			for (int i = 0; i < WINDOWSIZE; i++){
				if(pckt_window[i]->seqnum == precv1->seqnum){
					ind = i;
					break;
				}
			}
			if(ind == -1){
				printf("wrong ack on channel 0\n");
				exit(0);
			}
			ack_pending[ind] = 0;
			printf("RECVED ACK: Seq no - %d on channel %d\n", precv1->seqnum, 1);
			if(precv1->isLast == 1) lastAck = 1;

			int alldone = 1;
			for(int i = 0; i<WINDOWSIZE; i++){
				if(ack_pending[i] == 1){
					alldone = 0;
					break;
				}
			}
			if(alldone == 0) continue;
			else{
				if(lastAck == 1) break;
				//entire window has been completed send next set
				for (int i = 0; i<WINDOWSIZE; i++){
					if(offset<file_size){
						fseek(fp, offset, SEEK_SET);
						bytes = fread(data, CHARSIZE, PAYLOAD, fp);
						
						if(bytes>0){
							data[bytes] = '\0';
							if(bytes<PAYLOAD) last = 1;
							chnl = DATA_PACKET_NUMBER%2;

							pckt_window[i] = createNewPacket(bytes, offset, last, 0, chnl, data);
							pckt_window[i]->pcktnum = DATA_PACKET_NUMBER;
							DATA_PACKET_NUMBER++;

							ack_pending[i] = -1;

							offset = offset + bytes;
						}

					}
					else {
						//file is over
						pckt_window[i] = NULL;
					}
				}

				for(int i = 0; i<WINDOWSIZE; i++){
					if(pckt_window[i] == NULL) break;
					if(pckt_window[i]->pcktnum%2 == 0){
						int bytes_sent = sendto(channel0, pckt_window[i], sizeof(struct packet_struct), 0, (struct sockaddr*) &relay0, raddrlen0);
						if(bytes_sent == -1){
							printf("error in sendto\n");
							exit(0);
						}
						printf("SENT PACKET: Seq no - %d of size %d from channel %d at time - %s\n", pckt_window[i]->seqnum, bytes_sent, 0, timeprint());
						ack_pending[i] = 1;
					}
					else{
						int bytes_sent = sendto(channel1, pckt_window[i], sizeof(struct packet_struct), 0, (struct sockaddr*) &relay1, raddrlen1);
						if(bytes_sent == -1){
							printf("error in sendto\n");
							exit(0);
						}
						printf("SENT PACKET: Seq no - %d of size %d from channel %d at time - %s\n", pckt_window[i]->seqnum, bytes_sent, 1, timeprint());
						ack_pending[i] = 1;
					}
				}
			}

		}

	}
	int *a = NULL;
	int b = sendto(channel0, a, 0,0,(struct sockaddr*) &relay0, raddrlen0);
	int c = sendto(channel0, a, 0,0,(struct sockaddr*) &relay1, raddrlen1);
	close(channel0);
	close(channel1);
	fclose(fp);

	return 0;


}





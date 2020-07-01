#include "packet.h"

plist head, tail;

packet createNewPacket(int ps, int seq, int iL, int iA, int c, char *str){
	packet new_packet = (packet) malloc(sizeof(struct packet_struct));
	new_packet->pcktnum = -1;
	new_packet->payload_size = ps;
	new_packet->seqnum = seq;
	new_packet->isLast = iL;
	new_packet->isAck = iA;
	new_packet->channel = c;
	strcpy(new_packet->data, str);
	return new_packet;
}

plist createPlistNode(packet p){
	plist newnode = (plist) malloc(sizeof(struct packet_list));
	newnode->pckt = newnode->pckt = createNewPacket(p->payload_size, p->seqnum, p->isLast, p->isAck, p->channel, p->data);
	newnode->next_packet = NULL;
	return newnode;
}

void insertSorted(packet p){
	if(p==NULL) return;
	plist node = createPlistNode(p);
	if(head == NULL){
		head = node;
		tail = node;
		return;
	}
	else if(head->pckt->seqnum > p->seqnum){
		node->next_packet = head;
		head = node;
		return;
	}
	else if(tail->pckt->seqnum < p->seqnum){
		tail->next_packet = node;
		tail = node;
	}
	else{
		plist ptr = head;
		while(ptr->next_packet!= NULL && ptr->next_packet->pckt->seqnum < p->seqnum)
			ptr = ptr->next_packet;
		node->next_packet = ptr->next_packet;
		ptr->next_packet = node;
		return;
	}
}

packet getNext(){
	if(head == NULL) return NULL;
	if(head == tail){
		packet p = head->pckt;
		head = NULL;
		tail = NULL;
		return p;
	}
	else{
		packet p = head->pckt;
		head = head->next_packet;
		return p;
	}
}

char* timeprint(){
    char *str = (char*)malloc(sizeof(char)*20);
    int rc;
    time_t curr;
    struct tm* timeptr;
    struct timeval tv;

    curr = time(NULL);
    timeptr = localtime(&curr);
    gettimeofday(&tv,NULL);

    rc = strftime(str, 20, "%H:%M:%S", timeptr);

    char ms[8];
    sprintf(ms, ".%06ld",tv.tv_usec);
    strcat(str, ms);
    return str;
}
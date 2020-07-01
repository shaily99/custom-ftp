Computer Networks Assignment
-----------------------------------------------------------------------------------------------------

Submitted by:
Shaily Bhatt
2017A7PS0040P

-----------------------------------------------------------------------------------------------------
QUESTION 1
...........


Execution Instructions
----------------------

1. open terminal and change to directory ../2017A7PS0040P_ShailyBhatt/q1
2. run 
	$make
3. (2) will create the binary files and executable files. Now open two seperate terminals
4. In first terminal run
	$./server
5. In second terminal run
	$./client
6. Input the name of .txt file that you want to upload to the server
7. The copy will be created by the name of destination.txt (To change destination - refer Constants (line no - 51))

Files:
------

	1. packetDef.h and packet.h:
	-----------------------------
		a. Defines the structure of packet (packet_struct).
		- packet_struct contains - 
			payload_size
			sequence number of packet (offset from where pachet starts in original file - starting at 0)
			isLast (1 for the last packet and 0 otherwise)
			isAck (1 if it is acknowledgement packet, 0 for data packet)
			channel (channel through which packet is sent)
			data (actual string data from file)
		b. Defines structure of a wrapper node that forms the linklist for buffer to deal with out of order packets
		- contains pointer to packet node and a next node to facilitate linking
		c. Global variables head and tail of the buffer list
		d. Constants
		- These can be changed as required
			CHAR_SIZE, INT_SIZE - to be defined in accordance to machine (taken as 1 and 4 by default)
			PORT - port on which server listens (default - 8888)
			SERVERIP - IP address of server (default - "127.0.0.1" - localhost)
			TIMEOUT - in seconds
			PAYLOAD - number of characters to be carried in one packet
			PDR - Packet Drop Rate: should be an integer greater than 0 and less than 100
			DESTFILENAME - file name where server writes (the copy created)
		e. Function Definitions
	
	2. packet.c
	------------
		Defines all auxilliary functions
		a. createNewPacket - creates new packet with given attributes
		b. createPlistNode - creates the linked list node for a packet
		c. insertSorted - inserts packet into the buffer in increasing order of sequence number
		d. getNext - gets the next packet available in the buffer list

	3. server.c
	------------
		- Creates server socket
		- binds it to port and ip appropriately
		- listens for connection request
		- makes two connection with client
		- accepts packets through the two connections
		- activity on the two connections is monitored through select() system call and fd_set data structure
		- drops about PDR% of packets randomly
		- if the packet is in order, it is printed in the destinatation.txt file
		- if an out of order packet arrives it buffers it by inserting in buffer list in increasing order of sequence number
		- appropriately extracts packets from buffer list as the missing packets come and writes it to destination.txt
		- the code runs in an infinite while loop that facilitates constant recieving on packets on the server side
		- when the client has finished file transfer and closed its connection, the server recieves a packet of length 0, which is an indication for breaking from the while loop
		- closes all connection and exits

	4. client.c
	------------
		- Creates sockets corresponding to two channels - channel 0 and channel 1
		- maintains variable "offset" for reading next set of characters from file
		- reads from file 'PAYLOAD' number of characters and creates a packet for them
		- sends the packet over available channel (starting at 0 then 1 then 0 and so on)
		- As a soon as a channel sends a packet, it waits till ack is recieved
		- as soon as ack is recieved, next packet is sent through the channel
		- the channel activity is monitored using select() system call with fd_set data structure
		- timeout timer is also maintaned using the timer argument of select()
		- if timer of a packet runs out, the packet is resent
		- runs in an infinite while loop that continuously reads from the file, creates packet, sends it on available channel and waits for ack.
		- when all packets have been sent and all acknowledgements have been recieved, the program breaks out of while loop, closes connections and exits.


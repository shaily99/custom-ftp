Computer Networks Assignment
-----------------------------------------------------------------------------------------------------

Submitted by:
Shaily Bhatt
2017A7PS0040P

QUESTION 2



Execution Instructions
----------------------

1. open terminal and change to directory ../2017A7PS0040P_ShailyBhatt/q2
2. run 
	$make
3. (2) will create the binary files and executable files. Now open four seperate terminals
4. In first terminal run
	$./server
5. In second terminal run
	$./client
6. In third and fourth terminal run
	$./relay
7. In one of the terminals in which you ran relay, input 0 to run it as relay 0 and in the second input 1 to run it as relay 1
8. Input the name of .txt file that you want to upload to the server
9. The copy will be created by the name of destination.txt 

Files:
------

	1. packetDef.h and packet.h:
	-----------------------------
		a. Defines the structure of packet (packet_struct).
		- packet_struct contains - 
			pcktnum - data packet number number startng at 0 used for routing through relay 0 or 1
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
			CHARSIZE, INT_SIZE - to be defined in accordance to machine (taken as 1 and 4 by default)
			RELAYPORT_0, RELAYPOST_1, SERVERPORT - port on which relay0, relay1 and server listens (default - 12345, 54321, 8888)
			RELAYIP_0, RELAYIP_1, SERVERIP - IP address of relay0, relay1 and server (default - "127.0.0.1" - localhost)
			TIMEOUT - in seconds
			PAYLOAD - number of characters to be carried in one packet
			PDR - Packet Drop Rate: should be an integer greater than 0 and less than 100
			DESTFILENAME - file name where server writes (the copy created)
			WINDOWSIZE - for changine number of packets that can window can accomodate (default - 6)
			MAXDELAY - integer in milliseconds, maximum delay that relay introduces for a packet
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
		- accepts packets continuously through infinite while loop
		- if the packet is in order, it is printed in the destinatation.txt file
		- if an out of order packet arrives it buffers it by inserting in buffer list in increasing order of sequence number
		- appropriately extracts packets from buffer list as the missing packets come and writes it to destination.txt
		- the code runs in an infinite while loop that facilitates constant recieving on packets on the server side
		- appropriately prints log on the terminal itself.
		- when the client has finished file transfer and closed its connection, a packet of length 0 is sent through the relay to server, which is an indication for breaking from the while loop
		- closes all connection and exits

	4. client.c
	------------
		- Creates sockets corresponding to two channels - channel 0 and channel 1 that are connected to relay 0 and relay 1 respectively
		- maintains variable "offset" for reading next set of characters from file
		- reads from file 'PAYLOAD' number of characters and creates a packet for them
		- maintains a window of packets of size WINDOWSIZE which are sent one after the other without waiting for acknowledgement
		- packets with even packet numbers are sent through relay 0 and odd packet numbers are sent through relay 1
		- once a window has been sent, client waits for acknowledgements
		- a seperate array is maintained to keep a track of the acknowledgements
		- when an acknowledgement comes, that packet is marked at recieved
		- when all acknowledgements for the window have come, the entire window moves forward 
		- a single timer is maintained for the whole window
		- in case timeout occurs, those packets from the current window for which acknowledgement has not arrived are resent through appropriate channels
		- runs in an infinite while loop that continuously reads from the file, creates packet window, sends it to appropriate channel and waits for ack.
		- edge cases, where in the final window, all slots may not be in use is handled
		- prints logs on terminal itself
		- when all packets have been sent and all acknowledgements have been recieved, the program breaks out of while loop, closes connections and exits.

	5. relay.c
	-----------
		- the executable file of relay has to be run twice on seperate terminals so that two different relays can be run
		- command line argument of 0 or 1 makes the relay run as relay 0 or relay 1
		- based on this argument the realy makes appropriate connections and binds itself to a port
		- relay continuously listens for packets from server as well as client
		- if message comes from client side, it introduces a random delay (<2ms)
		- if message comes from client side it may randomly drop the packet
		- when packet from client is recieved it is passed on to server as it is 
		- when a packet from server is recieved no dropping or delaying is done and packet is transferred to appropriate client channel
		- the program runs in an infinite loop that continuously listens for messages.
		- if the client closes its connection, the relay exits while loop and closes it connections and exits from the program


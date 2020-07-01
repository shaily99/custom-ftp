# custom-ftp
This repository contains the assignment submission for the course 'Computer Networks' at BITS Pilani by me.

### Problem Statement 



#### Problem 1: Multi-Channel FTP using stop-and-wait over TCP.

1. The sender transmits packets through two different channels (TCP connections).
2. The server acknowledges the receipt of a packet via the same channel through which the corresponding packet has been received.
3. The sender transmits a new packet using the same channel on which it has received an ACK for its one of the previously transmitted packet. Note that, at a time, there can be at most two outstanding unacknowledged packets at the sender side.
4. On the server-side, the packets transmitted through different channels may arrive out of order. In that case, the server has to buffer the packets temporarily to finally construct in-order data stream.
5. Packets are dropped with a PDR of 10%. No ACKs are dropped.

#### Problem 2: Multi-channel FTP using selective-repeat over UDP.

1. The sender transmits packets through two different channels (TCP connections).
2. The server acknowledges the receipt of a packet via the same channel through which the
corresponding packet has been received.
3. There are two relay switches. All packets from first channel go through the first relay and all packets from second channel go through the second relay. The server sends ACK through the same relay from which it recieves packet.
4. Packets are dropped with a PDR of 10%. No ACKs are dropped.

The complete problem description with details of all features is [here](CN F303 Programming Assignment.pdf)

The implemented solution is in the respective folders.

###### A video demonstration of both the problems is available here: [problem1] and [problem2]

all:
	gcc packet.c -c -o packet.o
	gcc q1_client.c -c -o client.o
	gcc q1_server.c -c -o server.o

	gcc -g -o client packet.o client.o
	gcc -g -o server packet.o server.o
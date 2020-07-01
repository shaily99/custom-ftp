all:
	gcc packet.c -c -o packet.o
	gcc client.c -c -o client.o
	gcc server.c -c -o server.o
	gcc relay.c -c -o relay.o

	gcc -g -o client packet.o client.o
	gcc -g -o server packet.o server.o
	gcc -g -o relay packet.o relay.o
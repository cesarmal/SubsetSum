# 
# Makefile Projeto SSP ==> Subset Sum Paralel.
#
CC=g++ -pthread
CFLAGS=-c -Wall

all: servidor cliente

cliente: Client.o ServerSocket.o ClientSocket.o Socket.o
	$(CC) Client.o ServerSocket.o ClientSocket.o Socket.o -o cliente

servidor: Server.o ServerSocket.o ClientSocket.o Socket.o
	$(CC) Server.o ServerSocket.o ClientSocket.o Socket.o -o servidor

cliente.o: Client.cpp
	$(CC) $(CFLAGS) Client.cpp

servidor.o: Server.cpp
	$(CC) $(CFLAGS) Server.cpp

Socket.o: Socket.cpp
	$(CC) $(CFLAGS) Socket.cpp

ClientSocket.o: ClientSocket.cpp
	$(CC) $(CFLAGS) ClientSocket.cpp

ServerSocket.o: ServerSocket.cpp
	$(CC) $(CFLAGS) ServerSocket.cpp

clean:
	rm -rf *o cliente servidor


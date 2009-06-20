# 
# Makefile Projeto P2P ==> Protocols de Comunicação.
#
CC=g++ -pthread
CFLAGS=-c -Wall

all: servidor cliente

cliente: ServentP2P.o ServerSocket.o ClientSocket.o Socket.o
	$(CC) ServentP2P.o ServerSocket.o ClientSocket.o Socket.o -o cliente

servidor: RegistroP2P.o ServerSocket.o ClientSocket.o Socket.o
	$(CC) RegistroP2P.o ServerSocket.o ClientSocket.o Socket.o -o servidor

cliente.o: ServentP2P.cpp
	$(CC) $(CFLAGS) ServerSocket.cpp

servidor.o: RegistroP2P.cpp
	$(CC) $(CFLAGS) RegistroP2P.cpp

Socket.o: Socket.cpp
	$(CC) $(CFLAGS) Socket.cpp

ClientSocket.o: ClientSocket.cpp
	$(CC) $(CFLAGS) ClientSocket.cpp

ServerSocket.o: ServerSocket.cpp
	$(CC) $(CFLAGS) ServerSocket.cpp

clean:
	rm -rf *o cliente servidor


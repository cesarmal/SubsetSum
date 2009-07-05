# 
# Makefile Projeto SSP ==> Subset Sum Paralel.
#
CC=g++ -pthread
CFLAGS=-c -Wall

all: servidor cliente

cliente: Client.o ServerSocket.o ClientSocket.o Socket.o Solver.o BasicSSP.o 
	$(CC) Client.o ServerSocket.o ClientSocket.o Socket.o Solver.o BasicSSP.o -o cliente

servidor: Server.o ServerSocket.o ClientSocket.o Socket.o ClientData.o BasicSSP.o
	$(CC) Server.o ServerSocket.o ClientSocket.o Socket.o ClientData.o BasicSSP.o -o servidor

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

ClientData.o: ClientData.cpp
	$(CC) $(CFLAGS) ClientData.cpp

BasicSSP.o: BasicSSP.cpp
	$(CC) $(CFLAGS) BasicSSP.cpp

Solver.o: Solver.cpp
	$(CC) $(CFLAGS) Solver.cpp

clean:
	rm -rf *o cliente servidor


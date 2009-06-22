#include "ServerSocket.h"
#include "ClientSocket.h"
#include "SocketException.h"
#include "BasicSSP.h"
#include <string>
#include <string.h>
#include <map>
#include <list>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <fstream>

#define UDP_BUF_LEN 100

using namespace std;

/* 
 *
 * Classe para guardar dados
 * dos clients registrados no servidor.
 *
 */
class ClientData {
	private:
		int last_hello_in_secs;
		list<std::string> subsets;

	public:
		ClientData() { last_hello_in_secs = 0; }

		ClientData(const ClientData &s) {
			last_hello_in_secs = s.last_hello_in_secs;
		}

		~ClientData() { ; }

		void increment_hello_time() {
			last_hello_in_secs += 1;
		}

		void received_hello() {
			last_hello_in_secs = 0;
		}

		int get_hello_time() {
			return last_hello_in_secs;
		}
};


/*
 *
 * Classe que implementa o servidor P2P.
 *
 */
class Server {
	
	char server_ip[30];
	pthread_t receive_hellos_thread;
	pthread_t remove_dead_clients_thread;
	pthread_t expect_cmds_thread;

	/* Map (um tipo de hash do C++)
	 * que guarda os clientes que fizeram join no servidor.
	 * A chave é o IP do cliente e o valor associado a cada chave são
	 * os dados do cliente.
	 */
	static string path;
	static map<string, ClientData> clients;

	public:

	Server(const string &p) {
		cout << p << endl;
		path = p;

		/* thread para receber de pacotes de keep alive */
		pthread_create(&receive_hellos_thread, NULL, &Server::receive_hellos, NULL);

		/* thread para verificar dead client (tempo do ultimo alive > MAX_HELLO_INTERVAL) */
		pthread_create(&remove_dead_clients_thread, NULL, 
						&Server::remove_dead_clients, NULL);

		/* cria thread para receber comandos de Servents */
		pthread_create(&expect_cmds_thread, NULL, &Server::expect_cmds, NULL);
	}

	static void* remove_dead_clients(void *param) {
		while(true) {
			map<std::string, ClientData>::iterator it;
      		for(it = clients.begin(); it != clients.end(); ++it) {
				if(it->second.get_hello_time() > MAX_HELLO_INTERVAL) {
					cout << "Removing DEAD client: " << it->first << endl;
					clients.erase(it->first);
				} else {
					it->second.increment_hello_time();
				}
       		}
			sleep(1);
		}
	}

	static void process_join_cmd(std::string &ip, ServerSocket &sock) {
		if(Server::clients.count(ip) > 0) {
			; // cliente já fez join antes ...
			sock << "You are ALREADY registered on this server" ;
		} else {
			std::string key = ip;
			ClientData data;
			Server::clients.insert(make_pair(key, data));
			sock << "OK;2,3,4,5,10,13;23;" + path;
		}
	}

	static void process_publish_cmd(std::string &ip, ServerSocket &sock, string &data) {
		if(Server::clients.count(ip) > 0) {
			// colocar arquivos nos dados dele
            string filename = path+"/myip.out";
            ifstream myfile (filename.c_str());
            
            // file opened? 
            if (! myfile) { 
                // NO, abort program 
                cerr << "can't open input file \"" << filename << "\"" 
                     << endl; 
                exit(EXIT_FAILURE); 
            } 
            // copy file contents to cout 
            
            char c; 
            while (myfile.get(c)) { 
                cout.put(c); 
            } 
            
            
			sock << "NOT OK";
		} else {
			; // cliente NÃO existe
			sock << "You are NOT registered on this server";
		}
	}

	static void* expect_cmds(void *param) {
		ServerSocket server(SERVER_PORT);
		while(true) {
			ServerSocket new_sock;
			server.accept(new_sock);
			try {
				while (true) {
					std::string data;
					std:string ip;
					new_sock >> data;
					server.get_ip(ip);
					cout << "IP: " << ip << endl;
					
					switch(data[0]) {
						case 'J':
							{
								cout << "Command join received" << endl;
								process_join_cmd(ip, new_sock);
								break;
							}
						case 'P':
							{
								cout << "Command publish received" << endl;
								process_publish_cmd(ip, new_sock, data);
								break;
							}
					}
				}
			} catch(SocketException&) {}
		}
	} catch(SocketException& e) {
		cout << "Exception was caught:" << e.description() << "\nExiting.\n";
	}

	/*
	 *
	 * Recebe os HELLO packets dos clientes e zera o tempo deles.
	 *
	 */ 
	static void* receive_hellos(void *param) {
		int iSockFd=-1;
		int iLength=0;
		struct sockaddr_in servAddr,cliAddr;
		char buff[1024];
		iSockFd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		memset(&servAddr,0,sizeof(servAddr));
		memset(&cliAddr,0,sizeof(cliAddr));
		servAddr.sin_family=AF_INET;
		servAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
		servAddr.sin_port=htons(HELLO_PORT);
		int cliAddrLen=sizeof(struct sockaddr_in);
		int bindRet=bind(iSockFd,(struct sockaddr*)&servAddr,sizeof(servAddr));
		//cout <<"Bind returned "<<bindRet<<endl;
		while(true) {
			int num_bytes = recvfrom(iSockFd,buff,1024,0,
								(struct sockaddr*)&cliAddr,(socklen_t*)&cliAddrLen);
			/* pegar IP do client que mandou HELLO */
			std::string client_ip(inet_ntoa(cliAddr.sin_addr));

			/* zerar tempo de HELLO do client SE ele existir */
			if(clients.count(client_ip) != 1) {
				/* recebeu HELLO de client NAO registrado */
				cout << "Received HELLO packet from non registered client";
				cout << ": ip=" << client_ip << endl;
			} else {
				clients[client_ip].received_hello();
			}
			//close(iSockFd);
		}
	}
};

map<std::string, ClientData> Server::clients;
string Server::path;

int main(int argc, char *argv[]) {	
	if(argc != 2) {
		cout << "usage: " << argv[0] << " shared_dir" << endl;
		exit(-1);
	}
	string path(argv[1]);
	Server server(path);
	cout << "GET TO WORK !\n" ;
	sleep(20000);
	return 0;
}


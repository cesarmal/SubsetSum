#include "ServerSocket.h"
#include "ClientSocket.h"
#include "SocketException.h"
#include "BasicP2P.h"
#include <string>
#include <string.h>
#include <map>
#include <list>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>

#define UDP_BUF_LEN 100

using namespace std;

/* 
 *
 * Classe para guardar dados
 * dos servents registrados no servidor.
 *
 */
class ServentData {
	private:
		int last_hello_in_secs;
		list<std::string> published_files;

	public:
		ServentData() { last_hello_in_secs = 0; }

		ServentData(const ServentData &s) {
			last_hello_in_secs = s.last_hello_in_secs;
			published_files = s.published_files;
		}

		~ServentData() { published_files.clear(); }

		void add_file(string file) {
			published_files.push_back(file);
		}

		const list<std::string> &get_files() {
			return published_files;
		}

		void files_that_match_search(const std::string &s, list<std::string> &l) {
			list<string>::iterator it;
			size_t found;
			for(it=published_files.begin(); it != published_files.end(); it++) {
				found = it->find(s);
				if(found != string::npos) {
					l.push_back(*it);
				}
			}
		}
	
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
class RegistroP2P {
	
	char server_ip[30];
	pthread_t receive_hellos_thread;
	pthread_t remove_dead_servents_thread;
	pthread_t expect_cmds_thread;

	/* Map (um tipo de hash do C++)
	 * que guarda os clientes que fizeram join no servidor.
	 * A chave é o IP do cliente e o valor associado a cada chave são
	 * os dados do cliente.
	 */
	static map<string, ServentData> servents;

	public:

	RegistroP2P(const char *msg) {
		cout << msg << endl;

		/* thread para receber de pacotes de keep alive */
		pthread_create(&receive_hellos_thread, NULL, &RegistroP2P::receive_hellos, NULL);

		/* thread para verificar dead servent (tempo do ultimo alive > MAX_HELLO_INTERVAL) */
		pthread_create(&remove_dead_servents_thread, NULL, 
						&RegistroP2P::remove_dead_servents, NULL);

		/* cria thread para receber comandos de Servents */
		pthread_create(&expect_cmds_thread, NULL, &RegistroP2P::expect_cmds, NULL);
	}

	static void* remove_dead_servents(void *param) {
		while(true) {
			map<std::string, ServentData>::iterator it;
      		for(it = servents.begin(); it != servents.end(); ++it) {
				if(it->second.get_hello_time() > MAX_HELLO_INTERVAL) {
					cout << "Removing DEAD servent: " << it->first << endl;
					servents.erase(it->first);
				} else {
					it->second.increment_hello_time();
				}
       		}
			sleep(1);
		}
	}

	static void process_join_cmd(std::string &ip, ServerSocket &sock) {
		if(RegistroP2P::servents.count(ip) > 0) {
			; // cliente já fez join antes ...
			sock << "You are ALREADY registered on this server" ;
		} else {
			std::string key = ip;
			ServentData data;
			RegistroP2P::servents.insert(make_pair(key, data));
			sock << "OK" ;
		}
	}

	static void process_publish_cmd(std::string &ip, ServerSocket &sock, string &data) {
		if(RegistroP2P::servents.count(ip) > 0) {
			// colocar arquivos nos dados dele
			int file_init_index = 3;
			for(int i=3; i < data.size() -1; i++) {
				if(data[i] == ';' and data[i+1] == ';') {
					string file = "";
					file = data.substr(file_init_index, i - file_init_index);
					//cout << "FILE: " << file << endl;
					servents[ip].add_file(file);
					file_init_index = i + 2;
					i = i + 2;
				}
			}
			// pegar ultimo arquivo
			string file = "";
			file = data.substr(file_init_index, data.size() - file_init_index);
			servents[ip].add_file(file);

			//cout << data << endl;
			sock << "OK";
		} else {
			; // cliente NÃO existe
			sock << "You are NOT registered on this server";
		}
	}

	static void process_search_cmd(std::string &ip, ServerSocket &sock, 
									std::string &look_for) {
		cout << "BUSCANDO POR: " << look_for << endl;
		if(RegistroP2P::servents.count(ip) > 0) {
			/* buscar arquivos com a string enviada */
			std::string results("");
			map<std::string, ServentData>::iterator it;
			list<std::string> files;
			for(it=servents.begin(); it != servents.end(); it++) {
				files.clear();
				it->second.files_that_match_search(look_for, files);
				if(files.size() < 1) {
					continue;
				}
				/* 
				 * guardar arquivos nas string results (separados por ";;")
				 * uma sequencia ";;;" define uma separação de cliente
				 *
				 */
				if(results.size() > 0) {
					results += ";;;";
				}
				results += it->first;
				list<std::string>::iterator list_it;
				for(list_it=files.begin(); list_it != files.end(); list_it++) {
					results += std::string(";;");
					results += (*list_it);
				}
			}
			sock << results;
			cout << "RESULTADO DA BUSCA: " << results << endl;
		} else {
			; /* cliente NÃO existe neste servidor */
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
						case 'S':
							{
								cout << "Command search received" << endl;
								string look_for = data.substr(1, data.size() - 1);
								process_search_cmd(ip, new_sock, look_for);
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
			/* pegar IP do servent que mandou HELLO */
			std::string servent_ip(inet_ntoa(cliAddr.sin_addr));

			/* zerar tempo de HELLO do servent SE ele existir */
			if(servents.count(servent_ip) != 1) {
				/* recebeu HELLO de servent NAO registrado */
				cout << "Received HELLO packet from non registered servent";
				cout << ": ip=" << servent_ip << endl;
			} else {
				servents[servent_ip].received_hello();
			}
			//close(iSockFd);
		}
	}
};

map<std::string, ServentData> RegistroP2P::servents;

int main(int argc, char *argv[]) {
	RegistroP2P server("P2P SERVER STARTED");
	cout << "GET TO WORK !\n" ;
	sleep(20000);
	return 0;
}


#include <map>
#include <list>
#include <stdlib.h>
#include <pthread.h>
#include <fstream>
#include <sstream>
#include <string.h>

#include "BasicSSP.h"
#include "ClientData.h"
#include "Subset.h"

#define UDP_BUF_LEN 100

/*
 *
 * Classe que implementa o servidor P2P.
 *
 */
class Server: public BasicSSP {
	
	char server_ip[30];
	pthread_t receive_hellos_thread;
	pthread_t remove_dead_clients_thread;
	pthread_t expect_cmds_thread;

	/* Map (um tipo de hash do C++)
	 * que guarda os clientes que fizeram join no servidor.
	 * A chave é o IP do cliente e o valor associado a cada chave são
	 * os dados do cliente.
	 */
	static string shared_mem_fpath, problem_fpath;
	static map<string, ClientData> clients;
	static map<string, Subset> subsets;
	static list<string> set;
	static string sum;

	public:

	Server(const string shared_mem, const string &problem) {
		shared_mem_fpath = shared_mem;
		problem_fpath = problem;

		read_problem_file();

		break_set_in_subsets();

        //create or open map file
        struct shared *ptr;   
        ptr = initializa_mapper(shared_mem_fpath);

        /* initialize semaphore that is shared between processes */ 
        sem_init(&ptr->mutex,1, 1); 
        cout << "mutex\n";

		/* thread para receber de pacotes de keep alive */
		pthread_create(&receive_hellos_thread, NULL, &Server::receive_hellos, NULL);

		/* thread para verificar dead client (tempo do ultimo alive > MAX_HELLO_INTERVAL) */
		pthread_create(&remove_dead_clients_thread, NULL, 
						&Server::remove_dead_clients, NULL);

		/* cria thread para receber comandos de Servents */
		pthread_create(&expect_cmds_thread, NULL, &Server::expect_cmds, NULL);
	}

	static void check_valid_number(string &s) {
		if(s.size() == 0) {
			cout << "Can not accept a zero length number" << endl;
			exit(-1);
		}
		for(int i=0; i<s.size(); i++) {
			if(s[i] < '0' or s[i] > '9') {
				cout << "Invalid number string: " << s << " size: " << s.size() << endl;
				exit(-1);
			}
		}
	}

	static void break_set_in_subsets() {
		for(int i=0; i < ceil((float)set.size() / (float)SUBSET_SIZE); i++) {
			Subset s;
			s.status = SUBSET_NOT_READY;
			s.start = i*SUBSET_SIZE;
			if(i*SUBSET_SIZE + SUBSET_SIZE >= set.size())
				s.end = set.size() - 1;
			else
				s.end = i*SUBSET_SIZE + SUBSET_SIZE - 1;
			std::ostringstream stm;
		    stm	<< i + 1;
			string piece = stm.str();
			Server::subsets.insert(make_pair(piece, s));
			cout << "INSERTING SUBSET: " << i << " START: " << s.start;
			cout << " END: " << s.end << endl;
		}
	}

	/* leitura do problema subset em arquivo */
	static void read_problem_file() {
		string subset_problem;
		FILE *fd = fopen(problem_fpath.c_str(), "r");
		if(fd == NULL) {
			perror("Could not open subset input file");
			exit(-1);
		}
		// read set: "n1,n2,n3,n4,n5,n6;"
		string number;
		cout << "SET: ";
		while(true) {
			char c;
			int read_byte = fread(&c, 1, sizeof(char), fd);
			if(read_byte <= 0) {
				cout << "Error reading problem file - not a valid one" << endl;
				exit(-1);
			}
			if(c == ',' or c == ';') {
				check_valid_number(number);
				set.push_back(number);
				cout << number << " " ;
				number.clear();
				if(c == ';')
					break;
			} else {
				number.push_back(c);
			}
		}
		cout << endl;
		// read sum
		while(true) {
			char c;
			int read_byte = fread(&c, 1, sizeof(char), fd);
			if(read_byte != 1)
				break;
			if(c != '\n')
				sum.push_back(c);
		}
		check_valid_number(sum);
		cout << "SUM: " << sum << endl;
		fclose(fd);
	}

	static void* expect_cmds(void *func) {
		try {
			ServerSocket server(SERVER_PORT);
			while(true) {
				ServerSocket new_sock;
				server.accept(new_sock);
				try {
					while (true) {
						std::string data;
						std::string ip;
						new_sock >> data;
						server.get_ip(ip);
						cout << "IP: " << ip << endl;
						string answer;
						Server::process_cmd(ip, data, answer);
						new_sock << answer;
					}
				} catch(SocketException&) {}
			}
		} catch(SocketException& e) {
			cout << "Exception was caught:" << e.description() << "\nExiting.\n";
		}
		return 0;
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

	static void process_join(const string &ip, string &answer) {
		if(Server::clients.count(ip) > 0) {
			// cliente já fez join antes ...
			answer = "You are ALREADY registered on this server" ;
		} else {
			string key, subset, piece;
			int start, end;
			ClientData data;
			Server::clients.insert(make_pair(ip, data));
			answer = "OK";
		}
	}

	static void process_publish(const string &ip, string &answer) {
		if(Server::clients.count(ip) > 0) {           
			answer = "OK";
			// pegar primeiro subset NOT_READY e mandar pro cliente
			map<string, Subset>::iterator it;
			for(it=subsets.begin(); it != subsets.end(); it++) {
				piece = it->first;
				if(it->second.status == SUBSET_NOT_READY) {
					it->second.responsible = ip;

					start=it->second.start;
					end = it->second.end;
					break;
				}
			}
			
			// build subset to send
			list<string>::iterator set_it;
			int index = 0;
			for(set_it=set.begin(); set_it != set.end(); set_it++) {
				if(index > end)
					break;
				if(index >= start and index <= end) {
					if(index != start)
						subset += ",";
					subset += *set_it;
				}
				index += 1;
			}
			
			//       [status ; subset ; sum ; shared_mem_file ; piece]
			answer = "OK;" + subset + ";" + sum + ";" + shared_mem_fpath + ";" + piece; 

		} else {
			answer = "You are NOT registered on this server";
		}
	}

	static void process_cmd(const string &ip, const string &data, string &answer) {
		cout << data[0] << endl;
		switch(data[0]) {
			case 'J':
				{
					cout << "Command join received" << endl;
					process_join(ip, answer);
					break;
				}
			case 'P':
				{
					cout << "Command publish received" << endl;
					process_publish(ip, answer);
					break;
				}
		}
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

	/*
	 * Manda um comando de KILL para cada um dos clientes.
	 */
	void stop_clients() {
		map<std::string, ClientData>::iterator it;
      	for(it = clients.begin(); it != clients.end(); ++it) {
			const char *msg = "K";
			string answer;
			send_data_to(it->first, EXPECT_CMDS_PORT, msg, answer);
			if(answer != "OK") {
				cout << "Could NOT kill correctly client " << it->first << endl;
			}
		}
		clients.clear();
	}
};

map<std::string, ClientData> Server::clients;
map<std::string, Subset> Server::subsets;
string Server::shared_mem_fpath;
string Server::problem_fpath;
string Server::sum;
list<string> Server::set;

int main(int argc, char *argv[]) {	
	if(argc != 3) {
		cout << "usage: " << argv[0] << " shared_mem_file input_problem_file" << endl;
		exit(-1);
	}
	string shared_mem_fpath(argv[1]), problem_fpath(argv[2]);
	Server server(shared_mem_fpath, problem_fpath);
	cout << "GET TO WORK !\n" ;
	sleep(20);
	server.stop_clients();
	return 0;
}


#include <string>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <iostream>
#include <list>
#include <vector>
#include <fstream>

#include "Solver.h"


using namespace std;
/* teste de alteração */
class Client: public BasicSSP {

	static string server_ip;
	static int piece;
	static string shared_dir;
    static int sum;
	static list<int> subset;
	static pthread_t alive_thread;
	static pthread_t expect_cmds_thread;

	/* identifica se o client já conectou no servidor */
	static bool has_joined;

	public:

	Client(const string &ip) {
		server_ip = ip;

		/* thread para envio de pacotes alive */
		pthread_create(&alive_thread, NULL, &Client::alive, NULL);

		/* cria thread para receber comandos do servidor */
		pthread_create(&expect_cmds_thread, NULL, &Client::expect_cmds, NULL);

		/* faz join no servidor e publica seus arquivos */
		join();
		string piece, subset, sum, status, shared_mem_fpath;
		while(true) {
			cout << "Trying to get subset" << endl;
			get_subset(subset, sum, shared_mem_fpath, piece);
			if(subset == "") {
				cout << "Nothing to be done from server" << endl;
				exit(0);
			}
			// solve
			//cout << shared_mem_fpath << endl;
			//cout << subset << endl;
			//cout << sum << endl;
			Solver::solver_process(shared_mem_fpath, subset, atoi(sum.c_str()), atoi(piece.c_str()));
			// publica resultado
			status = "NOT_FOUND";
			publish(piece, status);
		}
	}

	static void* expect_cmds(void *func) {
		try {
			ServerSocket server(EXPECT_CMDS_PORT);
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
						process_cmd(ip, data, answer);
						new_sock << answer;
					}
				} catch(SocketException&) {}
			}
		} catch(SocketException& e) {
			cout << "Exception was caught:" << e.description() << "\nExiting.\n";
		}
		return 0;
	}


	static void process_cmd(const string &ip, const string &data, string &answer) {
		switch(data[0]) {
			case 'K':
				{
					cout << "Command Kill Received" << endl;
					process_kill(answer);
					break;
				}
			case 'P':
				{
					cout << "Command publish received" << endl;
					break;
				}
		}
	}

	static void process_kill(string &answer){
		bool ok = true;
		/*
		if(pthread_kill(alive_thread, SIGHUP) != 0)
			ok = false;
		if(pthread_kill(expect_cmds_thread, SIGHUP) != 0)
			ok = false;
		*/
		if(ok) {
			answer = "OK";
		} else {
			answer = "NOT OK";
		}
		// pediu pra sair, SAIU !
		cout << "Life is cruel - server asked me to die ..." << endl;
		//exit(0);
	}


	/*
	 * Envio de pacote UDP alive ao servidor.
	 */
	static void* alive(void *args) {
		int sk;
		struct sockaddr_in server;
		struct hostent *hp;
		char buf[100];
		int buf_len;
		int n_sent; 
		int n_read;
		if((sk = socket(PF_INET, SOCK_DGRAM, 0)) < 0)    {
			printf("Problem creating socket\n");
			exit(1);
		}
		server.sin_family=AF_INET;
		server.sin_addr.s_addr=inet_addr(Client::server_ip.c_str());  
		/* establish the server port number - we must use network byte order! */  
		server.sin_port = htons(HELLO_PORT);
		memset(buf,65,1);  /* send it to the echo server */    

		while(true) {
			if(!has_joined) { 
				sleep(MAX_HELLO_INTERVAL / 2);
				continue;
			}
			n_sent = sendto(sk,buf,1,0,  (struct sockaddr*) &server,sizeof(server));  
			if (n_sent<0) {
				perror("Problem sending data");
				exit(1);
			}
			sleep(MAX_HELLO_INTERVAL - 3);
		}
	}

	void split_subset(string msg) {
		vector<string> aux_v;
		StringSplit(msg,",",&aux_v);	
		for (int i=0;i<aux_v.size();i++) {
			subset.push_back(atoi(aux_v[i].c_str()));
		}		
	}

	void join() {
		if(has_joined) {
			cout << "Error: can not join twice !\n";
			exit(-1);
		}
		const char *msg = "J";
		string result;
		send_data_to(server_ip, SERVER_PORT, msg, result);
		if(result != "OK") {
			cout << "Error: could not join - server did not return OK !\n";
			cout << result << endl;
			exit(-1);
		}
		cout << "Join on server OK" << endl;
		has_joined = true;
	};

	/*
	 * Manda para o servidor um comando de publish (P)
	 * seguido de uma lista de arquivos separados por ";;".
	 *
	 */
	void publish(string &piece, string &status) {
		string msg = "P;" + piece + ";" + status;
		string result;
				
		//Manda mensagem para o servidor
		cout << msg << endl;
		send_data_to(server_ip, SERVER_PORT, msg.c_str(), result);
		if(result != "OK") {
			cout << "Error: could not publish - server did not return OK !\n";
			cout << result << endl;
			exit(-1);
		}
	}

	void get_subset(string &subset, string &sum, string &shared_mem_fpath, string &piece) {
		string result, msg = "G";
		send_data_to(server_ip, SERVER_PORT, msg.c_str(), result);

		cout << result << endl;
		vector<string> msg_parts;
		StringSplit(result,";",&msg_parts);
		//cout << "split" << endl;
		//cout << msg_parts[0] << endl;
		if(msg_parts[0] != "OK") {
			cout << "Error: could not get subset !\n";
			cout << msg_parts[0] << endl;
			exit(-1);
		}

		piece = msg_parts[4];
		shared_mem_fpath = msg_parts[3];
		sum = msg_parts[2];
		split_subset(msg_parts[1]);	
		subset = msg_parts[1];

		cout << "Subset received: " << msg_parts[1] << endl;
		cout << "Shared Dir: " << msg_parts[3] << endl;
	}  // closes file automatically

};

string Client::server_ip;
bool Client::has_joined = false;
string Client::shared_dir;
list<int> Client::subset;
//int Client::sum;
//int Client::piece;
pthread_t Client::alive_thread;
pthread_t Client::expect_cmds_thread;


/*
 * Recebe como argumentos o IP do servidor e o diretório com
 * os arquivos a serem compartilhados
 *
 */ 
int main(int argc, char *argv[]) {    
	if(argc != 2) {
		cout << "usage: " << argv[0] << " server_ip" << endl;
		exit(-1);
	}
    setbuf(stdout,NULL);
	Client c(argv[1]);
	sleep(200);
	return 0;
}


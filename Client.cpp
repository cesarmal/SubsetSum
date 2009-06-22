#include "ServerSocket.h"
#include "ClientSocket.h"
#include "SocketException.h"
#include "BasicSSP.h"
#include <string>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <list>
#include <vector>
#include <fstream>

using namespace std;
/* teste de alteração */
class Client {

	string server_ip;
	static string shared_dir;
    static int sum;
	static list<int> subset;
	pthread_t alive_thread;
	pthread_t send_files_thread;

	/* identifica se o client já conectou no servidor */
	static bool has_joined;

	public:

	Client(const string &ip) {
		server_ip = ip;

		/* faz join no servidor e publica seus arquivos */
		join();
		publish();
	
		/* thread para envio de pacotes alive */
		pthread_create(&alive_thread, NULL, &Client::alive, (void*)server_ip.c_str());

		/* cria thread para receber requisicoes de arquivos */
		//pthread_create(&send_files_thread, NULL, &Client::send_files, (void*)shared_dir.c_str());
	};


	/*
	 * Envio de pacote UDP alive ao servidor.
	 */
	static void* alive(void *ip) {
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
		server.sin_addr.s_addr=inet_addr("127.0.0.1");  
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
		vector<string> msg_parts;
		send_data_to(server_ip, SERVER_PORT, msg, result);
		StringSplit(result,";",&msg_parts);
		cout << msg_parts[0] << endl;
		if(msg_parts[0] != "OK") {
			cout << "Error: could not join - server did not return OK !\n";
			cout << msg_parts[0] << endl;
			exit(-1);
		}

		//Salva estrutura de dados no cliente
		shared_dir = msg_parts[3];
		split_subset(msg_parts[1]);	
		sum = atoi(msg_parts[2].c_str());

		has_joined = true;
		cout << "Has joined to server " << server_ip << endl;
		cout << "Subset received: " << msg_parts[1] << endl;
		cout << "Shared Dir: " << msg_parts[3] << endl;

	};

	/*
	 * Manda para o servidor um comando de publish (P)
	 * seguido de uma lista de arquivos separados por ";;".
	 *
	 */
	void publish() {
		const char *msg = "P";
		string result;
		
		//Mostra todos os elementos da lista
	    cout << "subset: ";
        list<int>::iterator p = subset.begin();
        while(p != subset.end()) {
            cout << *p << " ";
            p++;
        } 
		cout << endl; 
		
		//Salva Arquivo de Tratamento de Subset
		string filename = shared_dir+"/myip.out";
		ofstream myfile(filename.c_str(), ios::app);
		
        //file opened? 
        if (!myfile) { 
            // NO, abort program 
            cerr << "can't open output file \"" << filename << "\"" 
                 << endl; 
            exit (EXIT_FAILURE); 
        }
        myfile << "data to file" << endl;
		
		//Manda mensagem para o servidor
		cout << msg << endl;
		send_data_to(server_ip, SERVER_PORT, msg, result);
		if(result != "OK") {
			cout << "Error: could not publish - server did not return OK !\n";
			cout << result << endl;
			exit(-1);
		}
	}  // closes file automatically



	void send_data_to(const string &address, int port, const char *data, std::string &answer) {
		ClientSocket sock(address, port);
		sock << data;
		sock >> answer;
	}	

};

bool Client::has_joined = false;
string Client::shared_dir;
list<int> Client::subset;
int Client::sum;
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

	Client c(argv[1]);
	//c.search("est");
	//string fname("teste.txt");
	//c.get_file(fname);
	sleep(200);
	return 0;
}


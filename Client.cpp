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

using namespace std;

class Client {

	string server_ip;
	static string shared_dir;
    static int sum;
	static vector<int> subset;
	pthread_t alive_thread;
	pthread_t send_files_thread;

	/* identifica se o client já conectou no servidor */
	static bool has_joined;

	public:

	Client(const string &ip, const string &dir) {
		server_ip = ip;
		shared_dir = dir;

		/* faz join no servidor e publica seus arquivos */
		join();
		publish();
	
		/* thread para envio de pacotes alive */
		pthread_create(&alive_thread, NULL, &Client::alive, (void*)server_ip.c_str());

		/* cria thread para receber requisicoes de arquivos */
		pthread_create(&send_files_thread, NULL, &Client::send_files, (void*)shared_dir.c_str());
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

	static void* send_files(void *shared_dir) {
		ServerSocket server(FILE_TRANSFER_PORT);
		while(true) {
			ServerSocket new_sock;
			server.accept(new_sock);
			string fname;
			new_sock >> fname;
			//new_sock << "there goes the file ...";
			send_file(fname, new_sock);
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
		StringSplit(result,";",msg_parts);
		if(msg_parts[0] != "OK") {
			cout << "Error: could not join - server did not return OK !\n";
			cout << msg_parts[0] << endl;
			exit(-1);
		}

		//Salva estrutura de dados no cliente
		path = msg_parts[0];
		StringSplit(msg_parts[1],",",subset);	
		sum = msg_parts[3];

		has_joined = true;
		cout << "Has joined to server " << server_ip << endl;
		cout << "Subset received: " << msg_parts[1] << endl;

	};

	void get_file_list_from_shared_dir(list<string> &l) {
		DIR *dp;
		DIR *check_is_dir;
		struct dirent *ep;
     
		dp = opendir(shared_dir);
		if (dp != NULL) {
		   while (ep = readdir (dp)) {
			 string fpath(shared_dir);
			 fpath += string(ep->d_name);
			 check_is_dir = opendir(fpath.c_str());
			 if(check_is_dir == NULL) {
				l.push_back(ep->d_name);
			 }
		   }
		   (void) closedir (dp);
		 } else {
			 perror ("Couldn't open the directory");
			 exit(-1);
		 }
		 
	}

	/*
	 * Manda para o servidor um comando de publish (P)
	 * seguido de uma lista de arquivos separados por ";;".
	 *
	 */
	void publish() {
		string wts;
		string result;
		list<string> files;
		get_file_list_from_shared_dir(files);
		pass_to_string(files,wts);
		vector<char> v(wts.length() + 1);
		strcpy(&v[0], wts.c_str());
		char * msg = &v[0];
		wts = msg;
		cout << msg << endl;
		send_data_to(server_ip, SERVER_PORT, msg, result);
		if(result != "OK") {
			cout << "Error: could not publish - server did not return OK !\n";
			cout << result << endl;
			exit(-1);
		}
	}

	void pass_to_string(list<string> &l,string &stri) {
        stri= "P";
		l.sort();
		while (!l.empty()){
			if (l.front() != "."){
				if (l.front() != ".."){
					stri = stri + ";;" + l.front();
					}
			}	
			l.pop_front();
		}
	}

	/*
	 * Faz uma busca no servido, enviando um comando de publish (P)
	 * seguido de uma lista de arquivos separados por ";;".
	 *
	 */
	void search(const char *search_for) {
		std::string s(search_for);
		s = "S" + s;
		std::string result;
		send_data_to(server_ip, SERVER_PORT, s.c_str(), result);
		cout << "SEARCH RETURNED: " << result << endl;
	}

	void send_data_to(const char *address, int port, const char *data, std::string &answer) {
		ClientSocket sock(address, port);
		sock << data;
		sock >> answer;
	   	//cout << answer << endl;
	}	

	/*
	 * Pega um arquivo em outro client.
	 *
	 */
	//void get_file(char *fname, char *sev_addr, int serv_port) {
	void get_file(string &fname) {
		ClientSocket sock("127.0.0.1", FILE_TRANSFER_PORT);
		sock << fname;

		string file_data;
		string fpath = shared_dir;
	    fpath += "/";
		fpath += fname;
		FILE *fd = fopen(fpath.c_str(), "w");
		while(sock.recv(file_data) > 0) {
			fwrite(file_data.c_str(), sizeof(char), file_data.size(), fd);
		}
		//sock.close();
		fclose(fd);
	}

	/*
	 * Envia um arquivo para outro Servent.
	 */
	static bool send_file(string &fname, ServerSocket &sock) {
		string fpath = string(Client::shared_dir) + 
								string("/") + fname;
	
		cout << "Sending file: " << fpath << endl;
			
		FILE *fd = fopen(fpath.c_str(), "r");
		if(fd == NULL) {
			cout << "Error: could not send file " << fpath << endl;
			return false;
		}

		char buf[512];
		ssize_t len;

		while((len = fread(buf, sizeof(char), 512, fd)) > 0) {

			sock << string(buf);
			/*
			if(send(sock, buf, len, 0) != len) {
				perror("Couldn't send data:");
				return(false);
		  	}
			*/
		}
		fclose(fd);
		return true;
	}
};

bool Client::has_joined = false;
char Client::shared_dir[100];

/*
 * Recebe como argumentos o IP do servidor e o diretório com
 * os arquivos a serem compartilhados
 *
 */ 
int main(int argc, char *argv[]) {
	if(argc != 3) {
		cout << "usage: " << argv[0] << " server_ip shared_dir" << endl;
		exit(-1);
	}

	Client c(argv[1], argv[2]);
	c.search("est");
	string fname("teste.txt");
	c.get_file(fname);
	sleep(200);
	return 0;
}


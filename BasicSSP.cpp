// Implementation of the BasicSSP class.
#include "BasicSSP.h"


void BasicSSP::StringSplit(string str, string delim, vector<string> *results) {
	int cutAt;
	while( (cutAt = str.find_first_of(delim)) != (int)str.npos ) {
		if(cutAt > 0)
		{
			results->push_back(str.substr(0,cutAt));
		}
		str = str.substr(cutAt+1);
	}
	if(str.length() > 0) {
		results->push_back(str);
	}
}

void BasicSSP::StringSplit(string str, string delim, vector<int> *results) {
	int cutAt;
	while( (cutAt = str.find_first_of(delim)) != (int)str.npos ) {
		if(cutAt > 0)
		{
			results->push_back(atoi(str.substr(0,cutAt).c_str()));
		}
		str = str.substr(cutAt+1);
	}
	if(str.length() > 0) {
		results->push_back(atoi(str.c_str()));
	}
}

void BasicSSP::send_data_to(const string &address, int port, 
								const char *data, std::string &answer) {
	ClientSocket sock(address, port);
	sock << data;
	sock >> answer;
}

bool BasicSSP::file_exists(string filename) {
    struct stat stfileinfo;
    if (stat(filename.c_str(),&stfileinfo)==0) {
        return true;
    }
    return false;
}

/*
void* BasicSSP::expect_cmds(void *func) {
	//(void)(const string &, const string &, string &) process_cmd = func;
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
void* BasicSSP::process_cmd(const string &ip, const string &data, string &answer) {
	cout << "O Henrique tem razao" << endl;
}
*/

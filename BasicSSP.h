#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <sstream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

//POSIX Includes
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "ServerSocket.h"
#include "SocketException.h"
#include "ClientSocket.h"

using namespace std;

/**************************************************
 *  Valores que são usados pelo servidor
 *  e pelo cliente para interagirem.
 **************************************************/  

/**************************************************
 * Porta em que o servidor escuta por comandos
 * dos servents.
 **************************************************/  
#define SERVER_PORT 30000

/**************************************************
 * Porta UDP em que o servidor escuta por HELLO
 * packets dos servents.
 **************************************************/  
#define HELLO_PORT 30002

/**************************************************
 * Porta em que os servents escutam por pedido
 * de transferência de arquivo para outro servent.
 **************************************************/  
#define EXPECT_CMDS_PORT 30001

/**************************************************
 * Intervalo de tempo máximo para que um servent envie
 * um HELLO packet para o servidor para identificar
 * que ainda está ativo.
 **************************************************/  
#define MAX_HELLO_INTERVAL 10

#define MAX_SOLUTION_MAP 1000
#define MAX_HASH_SIZE 30

struct listMap {
    int key;
    int mark;
    char hash[MAX_HASH_SIZE];
};

struct shared { 
  sem_t mutex; /* the mutex: a Posix memory-based semaphore */ 
  int count; /* and the counter */ 
  listMap map[MAX_SOLUTION_MAP];
}; 

class BasicSSP  {

  public:
		static void StringSplit(string str, string delim, vector<string> *results);
		void StringSplit(string str, string delim, vector<int> *results);
		void send_data_to(const string &, int ,	const char *, string &);
		bool file_exists(string filename);
		struct shared* initializa_mapper(string filename);
	/*
	virtual void* process_cmd(const string &ip, const string &data, string &answer);
	static void* expect_cmds(void *args); 
	*/
};

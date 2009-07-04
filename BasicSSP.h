#include <string>
#include <vector>
#include <iostream>

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
#define MAX_HELLO_INTERVAL 30

class BasicSSP  {

  public:
		void StringSplit(string str, string delim, vector<string> *results);
		void send_data_to(const string &, int ,	const char *, string &);
	/*
	virtual void* process_cmd(const string &ip, const string &data, string &answer);
	static void* expect_cmds(void *args); 
	*/
};

#include <string>
#include <vector>

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
#define FILE_TRANSFER_PORT 30001

/**************************************************
 * Intervalo de tempo máximo para que um servent envie
 * um HELLO packet para o servidor para identificar
 * que ainda está ativo.
 **************************************************/  
#define MAX_HELLO_INTERVAL 60

void StringSplit(string str, string delim, vector<string> &results) {
	int cutAt;
	while( (cutAt = str.find_first_of(delim)) != str.npos ) {
		if(cutAt > 0)
		{
			results.push_back(str.substr(0,cutAt));
		}
		str = str.substr(cutAt+1);
	}
	if(str.length() > 0) {
		results.push_back(str);
	}
}

//header ClientData
#include <list>
#include <string>
/* 
 *
 * Classe para guardar dados
 * dos clients registrados no servidor.
 *
 */

class ClientData {
	private:
		int last_hello_in_secs;
		std::list<std::string> subsets;

	public:
		ClientData();

		ClientData operator=(const ClientData &s) ;

		~ClientData();

		void increment_hello_time();
		void received_hello();
		int get_hello_time();
};



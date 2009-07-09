
#include <string>

#define SUBSET_SIZE 16
#define SUBSET_NOT_READY 0
#define SUBSET_READY 1

class Subset {

	public:
		int start, end;
		std::string responsible;
		int status;
};

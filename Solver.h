//Subset Sum Meet-in-the-middle Algorithm Solver
//Based in http://www2-fs.informatik.uni-tuebingen.de/~reinhard/krypto/English/4.5.1.e.html
//Created by Heukirne

//POSIX Includes
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "BasicSSP.h"

#define FACTOR 100

struct listMap {
    float key;
    char value[30];
};

struct shared { 
  sem_t mutex; /* the mutex: a Posix memory-based semaphore */ 
  int count; /* and the counter */ 
  listMap map[1000];
}; 

struct thread_data {
    string values;
    int piece;
    shared *solution;
};

class Solver {
    int sum;
    vector<int> set;
    vector<int> table;
    shared *solution;
    int ini_set;
    int piece;
    float id; //constant split sides
    
public:  
    Solver (vector<int> set_init, int sum_init, shared *solution_init, int piece_init);
    void do_it();
    static void solver_process();
    static void* solver_thread(void* params);

private:  
    //inser value in shared memory with mutex control concurrence
    void solution_insert(float key, string value);
    pair<float,string> calcSum();
    string find_value(float value);
    void table_do(unsigned int ini, unsigned int b);    
};

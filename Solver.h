//Subset Sum Meet-in-the-middle Algorithm Solver
//Based in http://www2-fs.informatik.uni-tuebingen.de/~reinhard/krypto/English/4.5.1.e.html
//Created by Heukirne

#include "BasicSSP.h"

struct thread_data {
    vector<int> values;
    int piece;
    int sum;
    shared *solution;
};

class Solver {
    int sum;
    vector<int> set;
    vector<int> table;
    shared *solution;
    int ini_set;
    int piece;
    int mark; //constant split sides
    
public:  
    Solver (vector<int> set_init, int sum_init, shared *solution_init, int piece_init);
    void do_it();
    static void solver_process(string filename, string subset, int sum, int piece);
    static void* solver_thread(void* params);

private:  
    //inser value in shared memory with mutex control concurrence
    void solution_insert(int value, string hash);
    pair<int,string> calcSum();
    string find_value(int value);
    void table_do(unsigned int ini, unsigned int b);    
};

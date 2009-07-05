//Subset Sum Meet-in-the-middle Algorithm Solver
//Based in http://www2-fs.informatik.uni-tuebingen.de/~reinhard/krypto/English/4.5.1.e.html
//Created by Heukirne

#include <map>
#include <sstream>
#include <math.h>
#include <stdlib.h>

#include "BasicSSP.h"

#define FACTOR 100

class SubsetSolver {
    int sum;
    vector<int> set;
    vector<int> table;
    map<float,string> *solution;
    int ini_set;
    int piece;
    float id; //constant split sides
    
    public:
    
    SubsetSolver (vector<int> set_init, int sum_init, map<float,string> *solution_init, int piece_init) {
        set = set_init;
        sum = sum_init;
        solution = solution_init;
        piece = piece_init;
    }
    
    private: 
    
    float calcSum() { //Add all selected numbers
        float result = id;
        stringstream str;
        str << id*FACTOR;
        string hash = str.str();
        hash += "*";
        for(unsigned int i=0;i<table.capacity(); i++) {
            result += set[ini_set+i] * table[i];
            hash += table[i]+48;
        }
        solution->insert(make_pair(result, hash));
        //cout << hash << "=" << result << ", ";
        return result;
    }    
   
    string find_value(float value) {
        map<float,string>::iterator pos;
        float frac; float i;
        for(pos = solution->begin(); pos!=solution->end(); ++pos) {
            if ((int)(value - pos->first) == 0) { //verifica se é o valor esperado
                frac = FACTOR*modff(pos->first, &i);
                if ((int)(frac - FACTOR*id) != 0) { //verifica se não é do próprio id
                    //cout << endl << "compare: " << (int)(frac - FACTOR*id); 
                    return pos->second;    
                }
            }
        }
        return "";
    }
   
    void table_do(unsigned int ini, unsigned int b) {
        table.at(ini)=b;
        if (ini+1>=table.capacity()) { 
            float result = calcSum() - id;
            string other_piece = find_value(sum - result);
            if (other_piece.length() > 0) {
                //sleep(1);
                cout << endl << "SOLUTION(" << other_piece << "[" << (sum - result) << "] + [" << (result + id) << "]" << (*solution)[result + id] << ")";
            }
        } else { //call recursion
            //cout << "ini=" << ini << ", end=" << end << ", b=" << b << endl;
            table_do(ini+1,0);
            table_do(ini+1,1);
        }
    }

    public: 

    void do_it() {
        //calculate first piece        
        table.reserve((int)floor(set.size()/2));
        for(unsigned int i=0; i<table.capacity(); i++) {
            table.push_back(0);
        }
        //call recursion
        ini_set = 0;
        //id, defined by float the subsum side
        id = ((float)(ini_set+piece*10)/FACTOR);
        table_do(0,1);         
        table_do(0,0); 
             
        //calculate second piece 
        ini_set = (int)floor(set.size()/2);       
        table.clear();
        table.reserve(set.capacity()-ini_set);        
        for(unsigned int i=ini_set; i<set.capacity(); i++) {
            table.push_back(0);
        } 
        //call recursion
        
        //id, defined by float the subsum side
        id = ((float)(ini_set+piece*10)/FACTOR);
        table_do(0,1);
        table_do(0,0);
    }
    
};

struct thread_data {
    string values;
    int piece;
    map<float,string> *solution;
};

void* init_solver(void* params) {
    BasicSSP base;    

    struct thread_data *my_data;
    my_data = (struct thread_data *) params;
    
    //Initialize vector    
    vector<string> result;
    vector<int> set;
    string str = my_data->values;
    base.StringSplit(str,";",&result);
    base.StringSplit(result[0],",",&set);

    SubsetSolver subset_sum(set, atoi(result[1].c_str()), my_data->solution, my_data->piece);
    subset_sum.do_it();
    
    cout << "end piece: " << my_data->piece << endl;
    
    pthread_exit(NULL);
}

pthread_t first_solver;
pthread_t second_solver;
pthread_attr_t attr;
map<float,string> *solution;

int main() {
    cout << "GET TO SUBSOLVER !\n";
    
    //Set struct data
    map<float,string> solution;
    struct thread_data data[2];
    data[0].values = "1,3,6,7,12,2,4,5,8,9,10;70";
    data[0].piece = 1;
    data[0].solution = &solution;
    
    data[1].values = "1,3,6,7,12,2,4,5,8,9,10;70";
    data[1].piece = 2;
    data[1].solution = &solution;
    
    //Init Joinable Attribute
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    //Create Threads Solvers
    pthread_create(&first_solver, &attr, &init_solver, &data[0]);
    sleep(1);
    pthread_create(&second_solver, &attr, &init_solver, &data[1]);

    pthread_attr_destroy(&attr);

    //Waiting Joinable Threads
    pthread_join(first_solver,NULL);
    pthread_join(second_solver,NULL);

    cout << "JOIN THREADS! \n";
	
	sleep(1);

    pthread_exit(NULL);
}

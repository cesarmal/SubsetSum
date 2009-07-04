//Subset Sum Meet-in-the-middle Algorithm Solver
//Based in http://www2-fs.informatik.uni-tuebingen.de/~reinhard/krypto/English/4.5.1.e.html
//Created by Heukirne

#include <map>
#include <math.h>

#include "BasicSSP.h"

class SubsetSolver {
    int sum;
    vector<int> set;
    vector<int> table;
    map<float,string> solution;
    int ini_set;
    float factorM, factorL; //factorMore, factorLess: constant split sides
    
    public:
    
    SubsetSolver (vector<int> set_init, int sum_init) {
        set = set_init;
        sum = sum_init;
    }
    
    private: 
    
    float calcSum() { //Add all selected numbers
        float result= factorL;
        string hash;
        hash = (ini_set+48);
        hash += "*";
        for(unsigned int i=0;i<table.capacity(); i++) {
            result += set[ini_set+i] * table[i];
            hash += table[i]+48;
        }
        solution.insert(make_pair(result, hash));
        cout << hash << "=" << result << ", ";
        return result;
    }    
   
    void table_do(unsigned int ini, unsigned int b) {
        table.at(ini)=b;
        if (ini+1>=table.capacity()) { 
            float result = calcSum() - factorL;
            if (solution.count(sum - result - factorM) > 0) {
                cout << endl << "SOLUTION(" << solution[sum - result - factorM] << "+" << solution[result + factorL] << ")"  << endl;
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
        //factors, defined by float the subsum side
        factorL = ((float)ini_set/10);
        factorM = (int)floor(set.size()/2);
        factorM = factorM/10;
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
        
        //factors, defined by float the subsum side
        factorL = ((float)ini_set/10);
        factorM = 0;
        table_do(0,1);
        table_do(0,0);
    }
    
};

void* init_solver(void* params) {
    BasicSSP base;    

    //Initialize vector    
    vector<string> result;
    vector<int> set;
    base.StringSplit(*(string *)params,";",&result);
    base.StringSplit(result[0],",",&set);

    SubsetSolver subset_sum(set,atoi(result[1].c_str()));
    subset_sum.do_it();
    cout << endl;
    return 0;
}

pthread_t first_solver;
pthread_t second_solver;

int main() {

    string values = "1,3,6,7,12,2,4,5,8,9,10;50";
    pthread_create(&first_solver, NULL, &init_solver, &values);

	cout << "GET TO SUBSOLVER !\n" ;
	sleep(1);

    return 0;
}

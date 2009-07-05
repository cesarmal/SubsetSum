//Subset Sum Meet-in-the-middle Algorithm Solver
//Based in http://www2-fs.informatik.uni-tuebingen.de/~reinhard/krypto/English/4.5.1.e.html
//Created by Heukirne

#include <sstream>
#include <math.h>
#include <stdlib.h>
//POSIX Includes
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <string>

#include "BasicSSP.h"

#define FACTOR 100

struct list {
    float key;
    char value[20];
};

struct shared { 
  sem_t mutex; /* the mutex: a Posix memory-based semaphore */ 
  int count; /* and the counter */ 
  list map[1000];
} sharedStruct; 

class SubsetSolver {
    int sum;
    vector<int> set;
    vector<int> table;
    shared *solution;
    int ini_set;
    int piece;
    float id; //constant split sides
    
    public:
    
    SubsetSolver (vector<int> set_init, int sum_init, shared *solution_init, int piece_init) {
        set = set_init;
        sum = sum_init;
        solution = solution_init;
        piece = piece_init;
    }
    
    private: 
   
    //inser value in shared memory with mutex control concurrence
    void solution_insert(float key, string value) {
        int length;
        
        sem_wait(&solution->mutex); 
        printf("child:%d>", solution->count++); 
        solution->map[solution->count].key = key;
        length = value.copy(solution->map[solution->count].value,value.length(),0);
        solution->map[solution->count].value[length] = '\0';
        sem_post(&solution->mutex);   
    }
    
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
        solution_insert(result, hash);
        cout << hash << "=" << result << ", ";
        return result;
    }    
   
    string find_value(float value, bool owner_id) {
        float frac; float i;
        bool compare;
        for(int pos = 0; pos<=solution->count; ++pos) {
            if ((int)(value - solution->map[pos].key) == 0) { //verifica se é o valor esperado
                frac = FACTOR*modff(solution->map[pos].key, &i);
                compare = ((int)(frac - FACTOR*id) != 0);
                if (owner_id == true) compare = !compare;
                if (compare) { //verifica se não é do próprio id
                    //cout << endl << "compare: " << (int)(frac - FACTOR*id); 
                    return solution->map[pos].value;    
                }
            }
        }
        return "";
    }
   
    void table_do(unsigned int ini, unsigned int b) {
        table.at(ini)=b;
        if (ini+1>=table.capacity()) { 
            float result = calcSum() - id;
            string other_piece = find_value(sum - result,false);
            if (other_piece.length() > 0) {
                //sleep(1);
                string my_piece = find_value(sum - result,true);
                cout << endl << "SOLUTION(" << other_piece << "[" << (sum - result) << "] + [" << (result + id) << "]" << my_piece << ")";
            }
        } else { //call recursion
            //cout << "ini=" << ini << ", b=" << b << endl;
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

bool file_exists(string filename) {
    struct stat stfileinfo;
    if (stat(filename.c_str(),&stfileinfo)==0) {
        return true;
    }
    return false;
}

struct thread_data {
    string values;
    int piece;
    shared *solution;
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

pthread_attr_t attr;

int main() {
    cout << "GET TO SUBSOLVER !\n";

    //create mmap shared memory file
    struct shared *ptr; 
    string filename = "./file.o";
    bool fileExist=false;
    int fd, child;
    
    /* open file, initialize to 0, map into memory */ 
    fileExist = file_exists(filename);
    fd = open(filename.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO); 
    if (!fileExist) { write(fd,&sharedStruct, sizeof(struct shared));  }
    ptr = (struct shared *)mmap(NULL, sizeof(struct shared), PROT_READ | PROT_WRITE, MAP_SHARED,fd, 0); 
    close(fd); 
    
    /* initialize semaphore that is shared between processes */ 
    sem_init(&ptr->mutex,1, 1); 
    setbuf(stdout,NULL);
    
    //Init Joinable Attribute
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    //Create struct data
    struct thread_data data[4];
    
    if ((child=fork()) == 0) { /* child */ 
        //Create pthread var
        pthread_t first_solver;
        pthread_t second_solver;
        
        //Create Threads Solvers: First Fork
        data[0].values = "1,3,6,7,12,2,4,5,8,9,10;70";
        data[0].piece = 1;
        data[0].solution = ptr;
        pthread_create(&first_solver, &attr, &init_solver, &data[0]);
        
        data[1].values = "1,3,6,7,12,2,4,5,8,9,10;70";
        data[1].piece = 2;
        data[1].solution = ptr;
        pthread_create(&second_solver, &attr, &init_solver, &data[1]);
        
        //Waiting Joinable Threads
        pthread_join(first_solver,NULL);
        pthread_join(second_solver,NULL);

        cout << "JOIN THREADS! - CHILD FORK \n";
        
        pthread_exit(NULL);
    } else {
        //Create pthread var
        pthread_t first_solver;
        pthread_t second_solver;
        
        //Create Threads Solvers: First Fork
        data[0].values = "1,3,6,7,12,2,4,5,8,9,10;70";
        data[0].piece = 3;
        data[0].solution = ptr;
        pthread_create(&first_solver, &attr, &init_solver, &data[0]);
        
        data[1].values = "1,3,6,7,12,2,4,5,8,9,10;70";
        data[1].piece = 4;
        data[1].solution = ptr;
        pthread_create(&second_solver, &attr, &init_solver, &data[1]);
        
        //Waiting Joinable Threads
        pthread_join(first_solver,NULL);
        pthread_join(second_solver,NULL);

        cout << "JOIN THREADS! - PARENT FORK \n";
    }
    
    waitpid(child,0,0);
    cout << "JOIN FORK! \n";
	
    pthread_attr_destroy(&attr);    
    pthread_exit(NULL);
}

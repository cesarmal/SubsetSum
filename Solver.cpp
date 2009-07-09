// Implementation of the Solver class.
#include "Solver.h"
#define THREAD_NUM 2
#define PROCESS_NUM 2
#define countOf(array) ( sizeof (array)/sizeof(array[0]) )

Solver::Solver (vector<int> set_init, int sum_init, shared *solution_init, int piece_init) {
    set = set_init;
    sum = sum_init;
    solution = solution_init;
    piece = piece_init;
}

//inser value in shared memory with mutex control concurrence
void Solver::solution_insert(int value, string hash) {
    int length;
    
    sem_wait(&solution->mutex); //Lock
    solution->count++;
    solution->map[solution->count].key = value;
    solution->map[solution->count].mark = mark;
    length = hash.copy(solution->map[solution->count].hash,hash.length(),0);
    solution->map[solution->count].hash[length] = '\0';
    sem_post(&solution->mutex); //Unlock
}

pair<int,string> Solver::calcSum() { //Sum all selected numbers
    int result=0;
    stringstream str;
    str << mark;
    string hash = str.str();
    hash += "*";
    for(unsigned int i=0;i<table.capacity(); i++) {
        result += set[ini_set+i] * table[i];
        hash += table[i]+48;
    }
    solution_insert(result, hash);
    return make_pair(result,hash);
}    

string Solver::find_value(int value) {
    for(int pos = 0; pos<=solution->count; ++pos) {
        if ((value - solution->map[pos].key) == 0) { //verifica se é o valor esperado
            if ((mark -solution->map[pos].key) != 0) { //verifica se não é do próprio mark
                //cout << endl << "compare: " << (int)(frac - FACTOR*id); 
                return solution->map[pos].hash;    
            }
        }
    }
    return "";
}

void Solver::table_do(unsigned int ini, unsigned int b) {
    table.at(ini)=b;
    if (ini+1>=table.capacity()) { 
        pair<int, string> result = calcSum();
        string other_piece = find_value(sum - result.first);
        if (other_piece.length() > 0) {
            cout << endl << "SOLUTION(" << other_piece << "[" << (sum - result.first) << "] + [" << (result.first) << "]" << result.second << ")";
        }
    } else { //call recursion
        table_do(ini+1,0);
        table_do(ini+1,1);
    }
} 

void Solver::do_it() {
    //calculate first piece        
    table.reserve((int)floor(set.size()/2));
    for(unsigned int i=0; i<table.capacity(); i++) {
        table.push_back(0);
    }
    
    //mark, defined the subsum side
    ini_set = 0;
    mark = ini_set+piece*10;
    //call recursion    
    table_do(0,1);         
    table_do(0,0); 
         
    //calculate second piece 
    ini_set = (int)floor(set.size()/2);       
    table.clear();
    table.reserve(set.capacity()-ini_set);      
    for(unsigned int i=ini_set; i<set.capacity(); i++) {
        table.push_back(0);
    } 
    
    //mark, defined subsum side
    mark = ini_set+piece*10;
    //call recursion
    table_do(0,1);
    table_do(0,0);
}

void* Solver::solver_thread(void* params) {
    //Cast params struct
    struct thread_data *my_data;
    my_data = (struct thread_data *) params;

    //Print all Pieces
    cout << "piece(" << my_data->piece << "):";
    for (unsigned int j=0;j<my_data->values.size();j++) {
       cout << my_data->values[j] << ",";
    }
    cout << " finding:" << my_data->sum << endl;
    
    //Construct Solver Object
    Solver subset_sum(my_data->values, my_data->sum, my_data->solution, my_data->piece);
    //Call solver recursion
    subset_sum.do_it();
    
    cout << "end piece: " << my_data->piece << endl;
    
    pthread_exit(NULL);
};

int Solver::solver_process(string filename, string subset, int sum, int piece) {
    cout << "GET TO SUBSOLVER !\n";
    int child;

    //create mmap shared memory file
    BasicSSP base;
    struct shared *ptr;   
    ptr = base.initializa_mapper(filename);

    //Init Joinable Attribute
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    //Create struct data
    struct thread_data data[PROCESS_NUM*THREAD_NUM];
    
    //Generate struct data
    vector<int> set;
    base.StringSplit(subset,",",&set);
    unsigned int i = 0;
    for (unsigned int j=0;j<set.size();j++) {
        data[i].values.push_back(set[j]); 
        if(i++>=countOf(data)) {i=0;}
    }
    for (i=0; i<countOf(data); i++) {
        data[i].piece = (i+1)+piece*10;
        data[i].sum = sum;
        data[i].solution = ptr;
    }


    if ((child=fork()) == 0) { // child, (child=fork()) == 0
        //Create pthread var
        pthread_t thread_solver[THREAD_NUM];
        
        //Create Threads Solvers: First Fork
        for (unsigned int j=0;j<countOf(thread_solver);j++) {
            pthread_create(&thread_solver[j], &attr, &Solver::solver_thread, &data[j]);
        }
        //Waiting Joinable Threads
        for (unsigned int j=0;j<countOf(thread_solver);j++) {
            pthread_join(thread_solver[j],NULL);
        }

        cout << "JOIN THREADS! - CHILD FORK \n";
        
        pthread_exit(NULL);
    } else {
        //Create pthread var
        pthread_t thread_solver[THREAD_NUM];
        
        //Create Threads Solvers: First Fork
        for (unsigned int j=0;j<countOf(thread_solver);j++) {
            pthread_create(&thread_solver[j], &attr, &Solver::solver_thread, &data[j+PROCESS_NUM]);
        }
        //Waiting Joinable Threads
        for (unsigned int j=0;j<countOf(thread_solver);j++) {
            pthread_join(thread_solver[j],NULL);
        }

        cout << "JOIN THREADS! - PARENT FORK \n";
    }
    
    waitpid(child,0,0);
    cout << "JOIN FORK! \n";
	
    pthread_attr_destroy(&attr);
	return(0);
}

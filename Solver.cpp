// Implementation of the Solver class.
#include "Solver.h"
    
Solver::Solver (vector<int> set_init, int sum_init, shared *solution_init, int piece_init) {
    set = set_init;
    sum = sum_init;
    solution = solution_init;
    piece = piece_init;
}

//inser value in shared memory with mutex control concurrence
void Solver::solution_insert(float key, string value) {
    int length;
    
    sem_wait(&solution->mutex); 
    solution->count++;
    //cout << "child:" << solution->count << ","; 
    solution->map[solution->count].key = key;
    length = value.copy(solution->map[solution->count].value,value.length(),0);
    solution->map[solution->count].value[length] = '\0';
    sem_post(&solution->mutex);   
}

pair<float,string> Solver::calcSum() { //Add all selected numbers
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
    //cout << hash << "=" << result << ", ";
    return make_pair(result,hash);
}    

string Solver::find_value(float value) {
    float frac; float i;
    for(int pos = 0; pos<=solution->count; ++pos) {
        if ((int)(value - solution->map[pos].key) == 0) { //verifica se é o valor esperado
            frac = FACTOR*modff(solution->map[pos].key, &i);
            if ((int)(frac - FACTOR*id) != 0) { //verifica se não é do próprio id
                //cout << endl << "compare: " << (int)(frac - FACTOR*id); 
                return solution->map[pos].value;    
            }
        }
    }
    return "";
}

void Solver::table_do(unsigned int ini, unsigned int b) {
    table.at(ini)=b;
    if (ini+1>=table.capacity()) { 
        pair<float, string> result = calcSum();
        result.first -= id;
        string other_piece = find_value(sum - result.first);
        if (other_piece.length() > 0) {
            //sleep(1);
            //string my_piece = find_value(sum - result,true);
            cout << endl << "SOLUTION(" << other_piece << "[" << (sum - result.first) << "] + [" << (result.first + id) << "]" << result.second << ")";
        }
    } else { //call recursion
        //cout << "ini=" << ini << ", b=" << b << endl;
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

void* Solver::solver_thread(void* params) {
    BasicSSP base;    

    struct thread_data *my_data;
    my_data = (struct thread_data *) params;
    
    //Initialize vector    
    vector<string> result;
    vector<int> set;
    string str = my_data->values;
    base.StringSplit(str,";",&result);
    base.StringSplit(result[0],",",&set);

    Solver subset_sum(set, atoi(result[1].c_str()), my_data->solution, my_data->piece);
    subset_sum.do_it();
    
    cout << "end piece: " << my_data->piece << endl;
    
    pthread_exit(NULL);
};

void Solver::solver_process() {
    cout << "GET TO SUBSOLVER !\n";

    BasicSSP base;

    //create mmap shared memory file
    struct shared *ptr; 
    struct shared sharedStruct;  
    string filename = "/mnt/nfs/DOC/file.o";
    bool fileExist=false;
    int fd, child;
    
    /* open file, initialize to 0, map into memory */ 
    cout << "open\n";
    fileExist = base.file_exists(filename);
    fd = open(filename.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO); 
    if (!fileExist) { write(fd,&sharedStruct, sizeof(struct shared));  }
    ptr = (struct shared *)mmap(NULL, sizeof(struct shared), PROT_READ | PROT_WRITE, MAP_SHARED,fd, 0); 
    close(fd); 
    cout << "close\n";
    
    /* initialize semaphore that is shared between processes */ 
    sem_init(&ptr->mutex,1, 1); 
    setbuf(stdout,NULL);
    cout << "mutex\n";

    //Init Joinable Attribute
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    //Create struct data
    struct thread_data data[4];
    
    if ((child=fork()) == 0) { // child, (child=fork()) == 0
        //Create pthread var
        pthread_t first_solver;
        pthread_t second_solver;
        
        //Create Threads Solvers: First Fork
        data[0].values = "1,3,6,7,12,2,4,5,8,9,10;50";
        data[0].piece = 1;
        data[0].solution = ptr;
        pthread_create(&first_solver, &attr, &Solver::solver_thread, &data[0]);
        
        data[1].values = "1,3,6,7,12,2,4,5,8,9,10;50";
        data[1].piece = 2;
        data[1].solution = ptr;
        pthread_create(&second_solver, &attr, &Solver::solver_thread, &data[1]);
        
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
        pthread_create(&first_solver, &attr, &Solver::solver_thread, &data[0]);
        
        data[1].values = "1,3,6,7,12,2,4,5,8,9,10;70";
        data[1].piece = 4;
        data[1].solution = ptr;
        pthread_create(&second_solver, &attr, &Solver::solver_thread, &data[1]);
        
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

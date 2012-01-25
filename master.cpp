#include <cstdio>
#include "mpi.h"
#include "master.h"

Master::Master(const string &dagfile) {
    this->dagfile = dagfile;
    this->dag.read(dagfile);
}

Master::~Master() {
}

void Master::run() {
    printf("Master starting...\n");
    
    int numprocs;
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    
    int numworkers = numprocs - 1;
    
    printf("Communicating with %d workers\n", numworkers);
    
    // All workers are idle at the start
    for (int i=1; i < numworkers; i++) {
        this->idle.push(i);
    }
    
    while (this->dag.has_ready_task()) {
        Task *t = this->dag.next_ready_task();
        printf("Running task %s\n", t->name.c_str());
    }
    
    /*
    MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
    */
    
}

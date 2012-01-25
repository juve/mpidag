#include <cstdio>
#include "mpi.h"
#include "worker.h"
#include "protocol.h"

Worker::Worker() {
}

Worker::~Worker() {
}

void Worker::run() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    printf("Worker %d starting...\n", rank);
    
    while (1) {
        string name;
        string command;
        
        recv_request(name, command);
        
        printf("Running task %s on worker %d\n", name.c_str(), rank);
        
        send_response(name, 0);
    }
}

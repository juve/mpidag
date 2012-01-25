#include "mpi.h"
#include "protocol.h"

#define MAX_MESSAGE 16384

void send_request(const string &name, const string &command, int worker) {
    char buf[MAX_MESSAGE];
    strcpy(buf, name.c_str());
    MPI_Send(buf, strlen(buf)+1, MPI_CHAR, worker, TAG_COMMAND, MPI_COMM_WORLD);
}

void send_shutdown(int worker) {
    MPI_Send(NULL, 0, MPI_CHAR, worker, TAG_SHUTDOWN, MPI_COMM_WORLD);
}

int recv_request(string &name, string &command) {
    char buf[MAX_MESSAGE];
    MPI_Status status;
    
    MPI_Recv(buf, MAX_MESSAGE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    if (status.MPI_TAG == TAG_SHUTDOWN) {
        return 1;
    }
    
    name.clear();
    name += buf;
    
    return 0;
}

void send_response(const string &name, int exitcode) {
    char buf[MAX_MESSAGE];
    strcpy(buf, name.c_str());
    MPI_Send(buf, strlen(buf)+1, MPI_CHAR, 0, TAG_RESULT, MPI_COMM_WORLD);
}

void recv_response(string &name, int &exitcode, int &worker) {
    char buf[MAX_MESSAGE];
    MPI_Status status;
    MPI_Recv(buf, MAX_MESSAGE, MPI_CHAR, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
    
    name.clear();
    name += buf;
    worker = status.MPI_SOURCE;
    exitcode = 0;
}
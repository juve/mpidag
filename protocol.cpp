#include "mpi.h"
#include "protocol.h"

#define MAX_MESSAGE 16384

void send_request(const string &name, const string &command, int worker) {
    char buf[MAX_MESSAGE];
    strcpy(buf, name.c_str());
    MPI_Send(buf, strlen(buf)+1, MPI_CHAR, worker, TAG_COMMAND, MPI_COMM_WORLD);
}

void recv_request(string &name, string &command) {
    char buf[MAX_MESSAGE];
    MPI_Status status;
    
    MPI_Recv(buf, MAX_MESSAGE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    name.clear();
    name += string(buf);
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
    name += string(buf);
    worker = status.MPI_SOURCE;
    exitcode = 0;
}
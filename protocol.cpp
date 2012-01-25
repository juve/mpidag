#include "mpi.h"
#include "protocol.h"

#define MAX_MESSAGE 16384

// XXX This protocol is really, really terrible. Make something better.

void send_request(const string &name, const string &command, int worker) {
    char buf[MAX_MESSAGE];
    strcpy(buf, name.c_str());
    strcpy(buf+name.size()+1, command.c_str());
    MPI_Send(buf, name.size()+command.size()+2, MPI_CHAR, worker, TAG_COMMAND, MPI_COMM_WORLD);
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
    
    name = buf;
    command = buf+strlen(buf)+1;
    
    return 0;
}

void send_response(const string &name, int exitcode) {
    char buf[MAX_MESSAGE];
    sprintf(buf, "%d", exitcode);
    strcpy(buf+strlen(buf)+1, name.c_str());
    MPI_Send(buf, strlen(buf)+name.size()+2, MPI_CHAR, 0, TAG_RESULT, MPI_COMM_WORLD);
}

void recv_response(string &name, int &exitcode, int &worker) {
    char buf[MAX_MESSAGE];
    MPI_Status status;
    MPI_Recv(buf, MAX_MESSAGE, MPI_CHAR, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
    
    sscanf(buf,"%d",&exitcode);
    name = buf+strlen(buf)+1;
    worker = status.MPI_SOURCE;
}
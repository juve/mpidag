#include "string.h"
#include "mpi.h"

#include "protocol.h"

#define MAX_MESSAGE 16384

static char buf[MAX_MESSAGE];

// XXX This protocol is really, really terrible. Make something better.

void send_stdio_paths(const std::string &outfile, const std::string &errfile) {
    strcpy(buf, outfile.c_str());
    strcpy(buf+outfile.size()+1, errfile.c_str());
    int size = outfile.size()+errfile.size()+2;
    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD); // Send message size first
    MPI_Bcast(buf, outfile.size()+errfile.size()+2, MPI_CHAR, 0, MPI_COMM_WORLD); // Then send message
}

void recv_stdio_paths(std::string &outfile, std::string &errfile) {
    int size;
    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD); // Get size first
    MPI_Bcast(buf, size, MPI_CHAR, 0, MPI_COMM_WORLD); // Then get message
    outfile = buf;
    errfile = buf+strlen(buf)+1;
}

void send_request(const std::string &name, const std::string &command, int worker) {
    strcpy(buf, name.c_str());
    strcpy(buf+name.size()+1, command.c_str());
    MPI_Send(buf, name.size()+command.size()+2, MPI_CHAR, worker, TAG_COMMAND, MPI_COMM_WORLD);
}

void send_shutdown(int worker) {
    MPI_Send(NULL, 0, MPI_CHAR, worker, TAG_SHUTDOWN, MPI_COMM_WORLD);
}

int recv_request(std::string &name, std::string &command) {
    MPI_Status status;
    MPI_Recv(buf, MAX_MESSAGE, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    if (status.MPI_TAG == TAG_SHUTDOWN) {
        return 1;
    }
    
    name = buf;
    command = buf+strlen(buf)+1;
    
    return 0;
}

void send_response(const std::string &name, int exitcode) {
    sprintf(buf, "%d", exitcode);
    strcpy(buf+strlen(buf)+1, name.c_str());
    MPI_Send(buf, strlen(buf)+name.size()+2, MPI_CHAR, 0, TAG_RESULT, MPI_COMM_WORLD);
}

void recv_response(std::string &name, int &exitcode, int &worker) {
    MPI_Status status;
    MPI_Recv(buf, MAX_MESSAGE, MPI_CHAR, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
    
    sscanf(buf,"%d",&exitcode);
    name = buf+strlen(buf)+1;
    worker = status.MPI_SOURCE;
}

void send_total_runtime(double total_runtime) {
    MPI_Reduce(&total_runtime, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
}

double collect_total_runtimes() {
    double ignore = 0.0;
    double total_runtime = 0.0;
    MPI_Reduce(&ignore, &total_runtime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    return total_runtime;
}

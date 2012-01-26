#include "strlib.h"
#include "unistd.h"
#include "stdio.h"
#include "sys/wait.h"
#include "mpi.h"
#include "fcntl.h"
#include "worker.h"
#include "protocol.h"

extern char **environ;

Worker::Worker(const string &outfile, const string &errfile) {
    this->outfile = outfile;
    this->errfile = errfile;
}

Worker::~Worker() {
}

int Worker::run() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    printf("Worker %d: Starting...\n", rank);
    
    // Truncate the stdout/stderr files
    truncate(outfile.c_str(), 0);
    truncate(errfile.c_str(), 0);
    
    while (1) {
        string name;
        string command;
        
        int shutdown = recv_request(name, command);
        
        if (shutdown) {
            break;
        }
        
        printf("Worker %d: Running task %s\n", rank, name.c_str());
        
        pid_t pid = fork();
        if (pid == 0) {
            // Process arguments
            vector<string> args;
            split_args(args, command);
            char **argv = (char **)malloc((args.size()+1) * sizeof(char *)); // N + 1 for null-termination
            for (unsigned i=0; i<args.size(); i++) {
                argv[i] = (char *)malloc(sizeof(char)*(args[i].size()+1));
                strcpy(argv[i], args[i].c_str());
            }
            argv[args.size()] = NULL; // Last one is null
            
            int out = open(outfile.c_str(), O_WRONLY|O_CREAT|O_APPEND, 0000644);
            int err = open(errfile.c_str(), O_WRONLY|O_CREAT|O_APPEND, 0000644);
            
            // Redirect stdout/stderr
            close(STDOUT_FILENO);
            dup2(out, STDOUT_FILENO);
            
            close(STDERR_FILENO);
            dup2(err, STDERR_FILENO);
            
            close(out);
            close(err);
            
            // Exec process
            execve(argv[0], argv, environ);
            perror("execve");
            exit(1);
        }
        
        // Wait for task to complete
        struct rusage usage;
        int exitcode = 1;
        if (wait4(pid, &exitcode, 0, &usage) == -1) {
            perror("Failed waiting for task");
        }
        
        send_response(name, exitcode);
    }
    
    // Signal master
    // TODO Change this to an MPI_Send
    MPI_Barrier(MPI_COMM_WORLD);
    
    printf("Worker %d: Exiting...\n", rank);
    
    return 0;
}

#include <cstdio>
#include "mpi.h"
#include "worker.h"
#include "protocol.h"
#include "strlib.h"
#include "unistd.h"
#include "sys/wait.h"

extern char **environ;

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
        
        int shutdown = recv_request(name, command);
        
        if (shutdown) {
            break;
        }
        
        printf("Running task %s on worker %d\n", name.c_str(), rank);
        
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
            
            // Exec process
            execve(argv[0], argv, environ);
            perror("execve");
            exit(1);
        }
        
        // Wait for task to complete
        struct rusage usage;
        int exitcode;
        if (wait4(pid, &exitcode, 0, &usage) == -1) {
            perror("Child failed");
        }
        
        // Free arguments
        /*
        for (unsigned i=0; i<args.size(); i++) {
            free(argv[i]);
        }
        free(argv);
        */
        
        send_response(name, exitcode);
    }
}

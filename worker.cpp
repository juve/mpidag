#include "strlib.h"
#include "errno.h"
#include "unistd.h"
#include "stdio.h"
#include "sys/wait.h"
#include "mpi.h"
#include "fcntl.h"
#include "worker.h"
#include "protocol.h"
#include "log.h"

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
    
    log_info("Worker %d: Starting...", rank);
    
    // Truncate the stdout/stderr files
    truncate(outfile.c_str(), 0);
    truncate(errfile.c_str(), 0);
    
    while (1) {
        string name;
        string command;
        
        int shutdown = recv_request(name, command);
        
        if (shutdown) {
            log_trace("Worker %d: Got shutdown message", rank);
            break;
        }
        
        log_debug("Worker %d: Running task %s", rank, name.c_str());
        
        pid_t pid = fork();
        if (pid == 0) {
            // Process arguments
            vector<string> args;
            split_args(args, command);
            // N + 1 for null-termination
            char **argv = (char **)malloc((args.size()+1) * sizeof(char *));
            for (unsigned i=0; i<args.size(); i++) {
                argv[i] = (char *)malloc(sizeof(char)*(args[i].size()+1));
                strcpy(argv[i], args[i].c_str());
            }
            argv[args.size()] = NULL; // Last one is null
            
            int out = open(outfile.c_str(), O_WRONLY|O_CREAT|O_APPEND, 0000644);
            if (out < 0) {
                fprintf(stderr, "%s: unable to open stdout: %s\n", 
                    name.c_str(), strerror(errno));
                exit(1);
            }
            
            int err = open(errfile.c_str(), O_WRONLY|O_CREAT|O_APPEND, 0000644);
            if (err < 0) {
                fprintf(stderr, "%s: unable to open stderr: %s\n", 
                    name.c_str(), strerror(errno));
                exit(1);
            }
            
            // Redirect stdout/stderr
            close(STDOUT_FILENO);
            dup2(out, STDOUT_FILENO);
            
            close(STDERR_FILENO);
            dup2(err, STDERR_FILENO);
            
            close(out);
            close(err);
            
            // Exec process
            execve(argv[0], argv, environ);
            fprintf(stderr, "%s: unable to execve command: %s\n", 
                name.c_str(), strerror(errno));
            exit(1);
        }
        
        // Wait for task to complete
        struct rusage usage;
        int exitcode = 1;
        if (wait4(pid, &exitcode, 0, &usage) == -1) {
            log_warn("Failed waiting for task: %s", strerror(errno));
        }
        
        log_debug("Worker %d: Task %s finished with exitcode %d", rank, name.c_str(), exitcode);
        
        send_response(name, exitcode);
    }
    
    // Signal master
    // TODO Change this to an MPI_Send
    log_trace("Worker %d: Sending shutdown message to master", rank);
    MPI_Barrier(MPI_COMM_WORLD);
    
    log_info("Worker %d: Exiting...", rank);
    
    return 0;
}

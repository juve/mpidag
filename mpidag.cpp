#include <cstdio>
#include <stdlib.h>
#include <list>
#include "mpi.h"
#include "master.h"
#include "worker.h"
#include "failure.h"
#include "log.h"

using namespace std;

static char *program;
static int rank;

void usage() {
    if (rank == 0) {
        fprintf(stderr,
            "Usage: %s [options] DAGFILE\n"
            "\n"
            "Options:\n"
            "   -h|--help           Print this message\n"
            "   -v|--verbose        Increase logging level\n"
            "   -q|--quiet          Decrease logging level\n"
            "   -L|--logfile PATH   Path to log file\n"
            "   -o|--stdout PATH    Path to stdout file for tasks\n"
            "   -e|--stderr PATH    Path to stderr file for tasks\n",
            program
        );
    }
}

void argerror(const string &message) {
    if (rank == 0) {
        fprintf(stderr, "%s\n", message.c_str());
        usage();
    }
}

int mpidag(int argc, char *argv[]) {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    program = argv[0];
    
    list<char *> flags;
    for (int i=1; i<argc; i++) {
        flags.push_back(argv[i]);
    }
    
    string outfile;
    string errfile;
    string logfile;
    list<string> args;
    int loglevel = LOG_INFO;
    
    while (flags.size() > 0) {
        string flag = flags.front();
        if (flag == "-h" || flag == "--help") {
            usage();
            return 0;
        } else if (flag == "-o" || flag == "--stdout") {
            flags.pop_front();
            if (flags.size() == 0) {
                argerror("-o/--stdout requires PATH");
                return 1;
            }
            outfile = flags.front();
        } else if (flag == "-e" || flag == "--stderr") {
            flags.pop_front();
            if (flags.size() == 0) {
                argerror("-e/--stderr requires PATH");
                return 1;
            }
            errfile = flags.front();
        } else if (flag == "-q" || flag == "--quiet") {
            loglevel -= 1;
        } else if (flag == "-v" || flag == "--verbose") {
            loglevel += 1;
        } else if (flag == "-L" || flag == "--logfile") {
            flags.pop_front();
            if (flags.size() == 0) {
                argerror("-L/--logfile requires PATH");
                return 1;
            }
            logfile = flags.front();
        } else if (flag[0] == '-') {
            fprintf(stderr, "Unrecognized argument: %s\n", flag.c_str());
            return 1;
        } else {
            args.push_back(flag);
        }
        flags.pop_front();
    }
    
    if (args.size() == 0) {
        usage();
        return 1;
    }
    
    if (args.size() > 1) {
        argerror("Invalid argument");
        return 1;
    }
    
    string dagfile = args.front();
    
    if (outfile.size() == 0) {
        outfile = dagfile;
        outfile += ".mpidag.out";
    }
    
    if (errfile.size() == 0) {
        errfile = dagfile;
        errfile += ".mpidag.err";
    }
    
    FILE *log = NULL;
    log_set_level(loglevel);
    if (logfile.size() > 0) {
        log = fopen(logfile.c_str(), "w");
        if (log == NULL) {
            fprintf(stderr, "Unable to open log file: %s: %s\n", logfile.c_str(), strerror(errno));
            return 1;
        }
        log_set_file(log);
    }
    
    try {
        if (rank == 0) {
            return Master(dagfile, outfile, errfile).run();
        } else {
    
            // Add rank to workers' out/err files
            char dotrank[25];
            sprintf(dotrank, ".%d", rank);
            outfile += dotrank;
            errfile += dotrank;
    
            return Worker(outfile, errfile).run();
        }
    } catch (...) {
        // Make sure we close the log
        if (log != NULL) {
            fclose(log);
        }
        throw;
    }
    
    if (log != NULL) {
        fclose(log);
    }
}

int main(int argc, char *argv[]) {
    try {
        MPI_Init(&argc, &argv);
        int rc = mpidag(argc, argv);
        MPI_Finalize();
        return rc;
    } catch (exception &error) {
        fprintf(stderr, "FATAL: %s\n", error.what());
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

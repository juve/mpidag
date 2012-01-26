#include <cstdio>
#include <stdlib.h>
#include <list>
#include "mpi.h"
#include "master.h"
#include "worker.h"
#include "failure.h"

using namespace std;

static char *program;
static int rank;

void usage() {
    if (rank == 0) {
        fprintf(stderr,
            "Usage: %s [options] DAGFILE\n"
            "\n"
            "Options:\n"
            "   -h|--help       Print this message\n"
            "   -o|--stdout     Path to stdout file for tasks\n"
            "   -e|--stderr     Path to stderr file for tasks\n",
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
    list<string> args;
    
    while (flags.size() > 0) {
        string flag = flags.front();
        if (flag == "-h" || flag == "--help") {
            usage();
            return 0;
        } else if (flag == "-o" || flag == "--stdout") {
            flags.pop_front();
            if (flags.size() == 0) {
                argerror("-o/--stdout requires value");
                return 1;
            }
            outfile = flags.front();
        } else if (flag == "-e" || flag == "--stderr") {
            flags.pop_front();
            if (flags.size() == 0) {
                argerror("-e/--stderr requires value");
                return 1;
            }
            errfile = flags.front();
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
        outfile += ".out";
    }
    
    if (errfile.size() == 0) {
        errfile = dagfile;
        errfile += ".err";
    }
    
    if (rank == 0) {
        return Master(dagfile, outfile, errfile).run();
    }
    
    // Add rank to workers' out/err files
    char dotrank[25];
    sprintf(dotrank, ".%d", rank);
    outfile += dotrank;
    errfile += dotrank;
    
    return Worker(outfile, errfile).run();
}

int main(int argc, char *argv[]) {
    try {
        MPI_Init(&argc, &argv);
        int rc = mpidag(argc, argv);
        MPI_Finalize();
        return rc;
    } catch (exception &error) {
        fprintf(stderr, "Error: %s\n", error.what());
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

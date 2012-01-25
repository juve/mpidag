#include <stdio.h>
#include "mpi.h"
#include "master.h"
#include "worker.h"
#include "failure.h"

using namespace std;

int mpidag(int argc, char *argv[]) {
    int myid;
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    
    if (argc != 2) {
        if (myid == 0) {
            fprintf(stderr, "Usage: %s DAGFILE\n", argv[0]);
        }
        return 1;
    }
    
    if (myid == 0) {
        Master(string(argv[1])).run();
    } else {
        Worker().run();
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    try {
        MPI_Init(&argc, &argv);
        int rc = mpidag(argc, argv);
        MPI_Finalize();
        return rc;
    } catch (exception &error) {
        cerr << "Error: " << error.what() << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

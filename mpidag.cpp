#include <stdio.h>
#include "mpi.h"
#include "master.h"
#include "worker.h"
#include "failure.h"

using namespace std;

int mpidag(int argc, char *argv[]) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    if (argc != 2) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s DAGFILE\n", argv[0]);
        }
        return 1;
    }
    
    int rc = 0;
    
    if (rank == 0) {
        rc = Master(string(argv[1])).run();
    } else {
        Worker().run();
    }
    
    return rc;
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

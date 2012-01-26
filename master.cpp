#include <cstdio>
#include "sys/time.h"
#include "mpi.h"
#include "master.h"
#include "failure.h"
#include "protocol.h"

Master::Master(const string &dagfile, const string &outfile, const string &errfile) {
    this->dagfile = dagfile;
    this->outfile = outfile;
    this->errfile = errfile;
    this->dag.read(dagfile);
}

Master::~Master() {
}

void Master::submit_task(Task *t, int worker) {
    send_request(t->name, t->command, worker);
}

void Master::wait_for_result() {
    string name;
    int exitcode;
    int worker;
    recv_response(name, exitcode, worker);
    
    // Mark worker idle
    this->mark_worker_idle(worker);
    
    // Mark task finished
    Task *t = this->dag.get_task(name);
    this->dag.mark_task_finished(t, exitcode);
    
    printf("Task %s finished with exitcode %d\n", name.c_str(), exitcode);
}

void Master::add_worker(int worker) {
    this->mark_worker_idle(worker);
}

bool Master::has_idle_worker() {
    return !this->idle.empty();
}

int Master::next_idle_worker() {
    if (!this->has_idle_worker()) {
        failure("No idle workers");
    }
    int worker = this->idle.front();
    this->idle.pop();
    return worker;
}

void Master::mark_worker_idle(int worker) {
    this->idle.push(worker);
}

void Master::merge_task_stdio(FILE *dest, const string &srcfile, const string &stream) {
    FILE *src = fopen(srcfile.c_str(), "r");
    if (src == NULL) {
        failures("Unable to open task %s file: %s", stream.c_str(), srcfile.c_str());
    }
    
    char buf[BUFSIZ];
    while (1) {
        int r = fread(buf, 1, BUFSIZ, src);
        if (r < 0) {
            failures("Error reading source file: %s", srcfile.c_str());
        }
        if (r == 0) {
            break;
        }
        int w = fwrite(buf, 1, r, dest);
        if (w < r) {
            failures("Error writing to dest file");
        }
    }
    
    fclose(src);
    
    if (unlink(srcfile.c_str())) {
        failures("Unable to delete task %s file: %s", stream.c_str(), srcfile.c_str());
    }
}

int Master::run() {
    struct timeval start;
    gettimeofday(&start, NULL);
    
    printf("Master starting...\n");
    
    int numprocs;
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    
    int numworkers = numprocs - 1;
    if (numworkers == 0) {
        failure("Need at least 1 worker");
    }
    
    printf("Managing %d workers\n", numworkers);
    
    for (int i=1; i<=numworkers; i++) {
        this->add_worker(i);
    }
    
    // While DAG has tasks to run
    while (!this->dag.is_finished()) {
        
        // Submit as many tasks as we can
        while (this->dag.has_ready_task() && this->has_idle_worker()) {
            int worker = this->next_idle_worker();
            Task *task = this->dag.next_ready_task();
            /* DEBUG
            printf("Submitting '%s' to worker %d\n", task->name.c_str(), worker);
            */
            this->submit_task(task, worker);
        }
        
        /* DEBUG Verbosity
        if (!this->dag.has_ready_task()) {
            printf("No ready tasks\n");
        }
        
        if (!this->has_idle_worker()) {
            printf("No idle workers\n");
        }
        */
        
        this->wait_for_result();
    }
    
    struct timeval finish;
    gettimeofday(&finish, NULL);
    
    // Tell workers to exit
    // TODO Change this to MPI_Bcast
    for (int i=1; i<=numworkers; i++) {
        send_shutdown(i);
    }
    
    // Wait until all workers exit
    // TODO Change this to an MPI_Recv loop
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Merge stdout/stderr from all tasks
    FILE *outf = fopen(this->outfile.c_str(), "w");
    if (outf == NULL) {
        failures("Unable to open task stdout\n");
    }
    FILE *errf = fopen(this->errfile.c_str(), "w");
    if (errf == NULL) {
        failures("Unable to open task stderr\n");
    }
    
    // Collect all stdout/stderr
    char dotrank[25];
    for (int i=1; i<=numworkers; i++) {
        sprintf(dotrank, ".%d", i);
        
        string toutfile = this->outfile;
        toutfile += dotrank;
        this->merge_task_stdio(outf, toutfile, "stdout");
        
        string terrfile = this->errfile;
        terrfile += dotrank;
        this->merge_task_stdio(errf, terrfile, "stderr");
    }
    
    fclose(errf);
    fclose(outf);
    
    double stime = start.tv_sec + (start.tv_usec/1000000.0);
    double ftime = finish.tv_sec + (finish.tv_usec/1000000.0);
    
    printf("Wall time: %lf seconds\n", (ftime-stime));
    
    if (this->dag.is_failed()) {
        printf("Workflow failed\n");
        return 1;
    } else {
        printf("Workflow finished successfully\n");
        return 0;
    }
}

#include <cstdio>
#include "sys/time.h"
#include "mpi.h"
#include "master.h"
#include "failure.h"
#include "protocol.h"
#include "log.h"

Master::Master(DAG *dag, const string &outfile, const string &errfile) {
    this->dagfile = dagfile;
    this->outfile = outfile;
    this->errfile = errfile;
    this->dag = dag;
}

Master::~Master() {
}

void Master::submit_task(Task *task, int worker) {
    log_debug("Submitting task %s to worker %d", task->name.c_str(), worker);
    send_request(task->name, task->command, worker);
}

void Master::wait_for_result() {
    log_trace("Waiting for task to finish");
    
    string name;
    int exitcode;
    int worker;
    recv_response(name, exitcode, worker);
    
    // Mark worker idle
    log_trace("Worker %d is idle", worker);
    this->mark_worker_idle(worker);
    
    // Mark task finished
    if (exitcode == 0) {
        log_debug("Task %s finished with exitcode %d", name.c_str(), exitcode);
    } else {
        log_error("Task %s failed with exitcode %d", name.c_str(), exitcode);
    }
    Task *t = this->dag->get_task(name);
    this->dag->mark_task_finished(t, exitcode);
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
    log_trace("Merging %s file: %s", stream.c_str(), srcfile.c_str());
    
    FILE *src = fopen(srcfile.c_str(), "r");
    if (src == NULL) {
        // The file may not exist if the worker didn't run any tasks, just print a warning
        if (errno == ENOENT) {
            log_warn("No %s file: %s", stream.c_str(), srcfile.c_str());
            return;
        } else {
            failures("Unable to open task %s file: %s", stream.c_str(), srcfile.c_str());
        }
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
    int numprocs;
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    
    int numworkers = numprocs - 1;
    if (numworkers == 0) {
        failure("Need at least 1 worker");
    }
    
    log_info("Master starting with %d workers", numworkers);
    
    // First, send out the paths to the outfile/errfile
    send_stdio_paths(this->outfile, this->errfile);
    
    // Queue up the workers
    for (int i=1; i<=numworkers; i++) {
        this->add_worker(i);
    }
    
    // Start time of workflow
    struct timeval start;
    gettimeofday(&start, NULL);

    // While DAG has tasks to run
    while (!this->dag->is_finished()) {
        
        // Submit as many tasks as we can
        while (this->dag->has_ready_task() && this->has_idle_worker()) {
            int worker = this->next_idle_worker();
            Task *task = this->dag->next_ready_task();
            this->submit_task(task, worker);
        }
        
        if (!this->dag->has_ready_task()) {
            log_debug("No ready tasks");
        }
        
        if (!this->has_idle_worker()) {
            log_debug("No idle workers");
        }
        
        this->wait_for_result();
    }
    
    log_info("Workflow finished");
    
    // Finish time of workflow
    struct timeval finish;
    gettimeofday(&finish, NULL);
    
    // Tell workers to exit
    // TODO Change this to MPI_Bcast
    log_trace("Sending workers shutdown messages");
    for (int i=1; i<=numworkers; i++) {
        send_shutdown(i);
    }
    
    // Wait until all workers exit
    // TODO Change this to an MPI_Recv loop
    log_trace("Waiting for workers to finish");
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Merge stdout/stderr from all tasks
    log_trace("Merging stdio from workers");
    FILE *outf = fopen(this->outfile.c_str(), "w");
    if (outf == NULL) {
        failures("Unable to open stdout file: %s\n", this->outfile.c_str());
    }
    FILE *errf = fopen(this->errfile.c_str(), "w");
    if (errf == NULL) {
        failures("Unable to open stderr file: %s\n", this->outfile.c_str());
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
    log_info("Wall time: %lf seconds", (ftime-stime));
    
    if (this->dag->max_failures_reached()) {
        log_error("Max failures reached");
    }
    
    if (this->dag->is_failed()) {
        log_error("Workflow failed");
        return 1;
    } else {
        log_info("Workflow suceeded");
        return 0;
    }
}

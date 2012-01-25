#include <cstdio>
#include "mpi.h"
#include "master.h"
#include "failure.h"
#include "protocol.h"

Master::Master(const string &dagfile) {
    this->dagfile = dagfile;
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

int Master::run() {
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
            printf("Submitting '%s' to worker %d\n", task->name.c_str(), worker);
            this->submit_task(task, worker);
        }
        
        if (!this->dag.has_ready_task()) {
            printf("No ready tasks\n");
        }
        
        if (!this->has_idle_worker()) {
            printf("No idle workers\n");
        }
        
        this->wait_for_result();
    }
    
    printf("Workflow finished\n");
    
    for (int i=1; i<=numworkers; i++) {
        send_shutdown(i);
    }
    
    if (this->dag.is_failed()) {
        return 1;
    } else {
        return 0;
    }
}

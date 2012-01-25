#include <string>
#include "stdio.h"
#include "dag.h"
#include "failure.h"

void test_dag() {
    DAG dag;
    
    dag.read("test/test.dag");
    
    Task *alpha = dag.get_task("Alpha");
    if (alpha == NULL) {
        failure("Didn't parse Alpha");
    }
    if (alpha->command.compare("/bin/echo Alpha") != 0) {
        failure("Command failed for Alpha: %s", alpha->command.c_str());
    }
    
    Task *beta = dag.get_task("Beta");
    if (beta == NULL) {
        failure("Didn't parse Beta");
    }
    if (beta->command.compare("/bin/echo Beta") != 0) {
        failure("Command failed for Beta: %s", beta->command.c_str());
    }
    
    if (alpha->children[0] != beta) {
        failure("No children");
    }
    
    if (beta->parents[0] != alpha) {
        failure("No parents");
    }
}

void diamond_dag() {
    DAG dag;
    dag.read("test/diamond.dag");
    
    if (!dag.has_ready_task()) {
        failure("Did not queue root tasks");
    }
    
    Task *a = dag.next_ready_task();
    if (a->name.compare("A") != 0) {
        failure("Queued non root task %s", a->name.c_str());
    }
    
    if (dag.has_ready_task()) {
        failure("Queued non-root tasks");
    }
    
    dag.mark_task_finished(a);
    
    if (!dag.has_ready_task()) {
        failure("Marking did not release tasks");
    }
    
    Task *bc = dag.next_ready_task();
    if (!dag.has_ready_task()) {
        failure("Marking did not release tasks");
    }
    
    Task *cb = dag.next_ready_task();
    if (dag.has_ready_task()) {
        failure("Marking released too many tasks");
    }
    
    if (bc->name.compare("B") != 0 && bc->name.compare("C") != 0) {
        failure("Wrong task released: %s", bc->name.c_str());
    }
    
    if (cb->name.compare("B") != 0 && cb->name.compare("C") != 0) {
        failure("Wrong task released: %s", cb->name.c_str());
    }
    
    dag.mark_task_finished(bc);
    if (dag.has_ready_task()) {
        failure("Marking released a task when it shouldn't");
    }
    
    dag.mark_task_finished(cb);
    if (!dag.has_ready_task()) {
        failure("Marking all parents did not release task D");
    }
    
    Task *d = dag.next_ready_task();
    if (d->name.compare("D") != 0) {
        failure("Not task D");
    }
    
    if (dag.has_ready_task()) {
        failure("No more tasks are available");
    }
    
    if (dag.is_finished()) {
        failure("DAG is not finished");
    }
    
    dag.mark_task_finished(d);
    
    if (!dag.is_finished()) {
        failure("DAG is finished");
    }
}

int main(int argc, char *argv[]) {
    test_dag();
    diamond_dag();
    return 0;
}

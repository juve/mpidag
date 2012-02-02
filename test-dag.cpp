#include <string>
#include "stdio.h"

#include "stdlib.h"
#include "dag.h"
#include "failure.h"

void test_dag() {
    DAG dag("test/test.dag");
    
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
    DAG dag("test/diamond.dag");
    
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
    
    dag.mark_task_finished(a, 0);
    
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
    
    dag.mark_task_finished(bc, 0);
    if (dag.has_ready_task()) {
        failure("Marking released a task when it shouldn't");
    }
    
    dag.mark_task_finished(cb, 0);
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
    
    dag.mark_task_finished(d, 0);
    
    if (!dag.is_finished()) {
        failure("DAG is finished");
    }
}

void diamond_dag_failure() {
    DAG dag("test/diamond.dag");
    
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
    
    dag.mark_task_finished(a, 1);
    
    if (dag.has_ready_task()) {
        failure("Released tasks even though parent failed");
    }
    
    if (!dag.is_finished()) {
        failure("DAG should have been finished after A failed");
    }
}

void diamond_dag_oldrescue() {
    DAG dag("test/diamond.dag", "test/diamond.rescue", "");
    
    if (!dag.has_ready_task()) {
        failure("Should have ready D task");
    }
    
    Task *d = dag.next_ready_task();
    if (d->name.compare("D") != 0) {
        failure("Ready task is not D");
    }
    
    dag.mark_task_finished(d, 1);
    
    if (dag.has_ready_task()) {
        failure("Ready tasks even though D failed");
    }
    
    if (!dag.is_finished()) {
        failure("DAG should have been finished after D failed");
    }
    
    if (!dag.is_failed()) {
        failure("DAG should be failed");
    }
}

void read_file(char *fname, char *buf) {
    FILE *f = fopen(fname, "r");
    int read = fread(buf, 1, 1024, f);
    buf[read] = '\0';
    fclose(f);
}

void diamond_dag_newrescue() {
	char temp[1024];
	sprintf(temp,"file_XXXXXX");
    mkstemp(temp);
    
    DAG dag("test/diamond.dag", "", temp);
    
    Task *a = dag.next_ready_task();
    dag.mark_task_finished(a, 0);
    
    Task *bc = dag.next_ready_task();
    dag.mark_task_finished(bc, 0);
    
    Task *cb = dag.next_ready_task();
    dag.mark_task_finished(cb, 0);
    
    Task *d = dag.next_ready_task();
    dag.mark_task_finished(d, 1);
    
    if (!dag.is_finished()) {
        failure("DAG should be finished");
    }
    
    if (!dag.is_failed()) {
        failure("DAG should be failed");
    }
    
    char buf[1024];
    read_file(temp, buf);
    if (strcmp(buf, "\nDONE A\nDONE B\nDONE C") != 0) {
        failure("Rescue file not updated properly: %s", temp);
    } else {
        unlink(temp);
    }
}

void diamond_dag_rescue() {
	char temp[1024];
	sprintf(temp, "file_XXXXXX");
    mkstemp(temp);
    
    DAG dag("test/diamond.dag", "test/diamond.rescue", temp);
    
    if (!dag.has_ready_task()) {
        failure("Should have ready D task");
    }
    
    Task *d = dag.next_ready_task();
    if (d->name.compare("D") != 0) {
        failure("Ready task is not D");
    }
    
    dag.mark_task_finished(d, 0);
    
    if (dag.has_ready_task()) {
        failure("Ready tasks even though D finished");
    }
    
    if (!dag.is_finished()) {
        failure("DAG should have been finished after D finished");
    }
    
    if (dag.is_failed()) {
        failure("DAG should not be failed");
    }
    
    char buf[1024];
    read_file(temp, buf);
    if (strcmp(buf, "\nDONE A\nDONE B\nDONE C\nDONE D") != 0) {
        failure("Rescue file not updated properly: %s: %s", temp, buf);
    } else {
        unlink(temp);
    }
}

void diamond_dag_max_failures() {
    DAG dag("test/diamond.dag", "", "", 1);

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
    
    dag.mark_task_finished(a, 0);

    Task *bc = dag.next_ready_task();

    dag.mark_task_finished(bc, 1);

    if (dag.has_ready_task()) {
        failure("DAG should not have a ready task because %s failed", bc->name.c_str());
    }
}

void diamond_dag_retries() {
    int tries = 3;

    DAG dag("test/diamond.dag", "", "", 0, tries);
    
    Task *a;
    
    for (int i=0; i<tries; i++) {
        if (!dag.has_ready_task()) {
            failure("A should have been ready");
        }

        a = dag.next_ready_task();
        if (a->name.compare("A") != 0) {
            failure("A should have been ready");
        }

        dag.mark_task_finished(a, 1);
    }
    
    if (dag.has_ready_task()) {
        failure("DAG should not have a ready task because A failed");
    }
}

void diamond_dag_retries2() {
    int tries = 3;

    DAG dag("test/diamond.dag", "", "", 0, tries);
    
    Task *a;
    
    for (int i=0; i<tries-1; i++) {
        if (!dag.has_ready_task()) {
            failure("A should have been ready");
        }
        
        a = dag.next_ready_task();
        if (a->name.compare("A") != 0) {
            failure("A should have been ready");
        }
        
        dag.mark_task_finished(a, 1);
    }
    
    if (!dag.has_ready_task()) {
        failure("A should have been ready");
    }
    
    a = dag.next_ready_task();
    if (a->name.compare("A") != 0) {
        failure("A should have been ready");
    }
    
    dag.mark_task_finished(a, 0);

    if (!dag.has_ready_task()) {
        failure("DAG should have a ready task because A finally succeeded");
    }

    Task *bc = dag.next_ready_task();
    if (bc->name.compare("B")!=0 && bc->name.compare("C")!=0) {
        failure("B or C should have been ready");
    }
}

int main(int argc, char *argv[]) {
    test_dag();
    diamond_dag();
    diamond_dag_failure();
    diamond_dag_max_failures();
    diamond_dag_retries();
    diamond_dag_retries2();
    diamond_dag_oldrescue();
    diamond_dag_newrescue();
    diamond_dag_rescue();
    return 0;
}

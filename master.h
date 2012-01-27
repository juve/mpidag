#ifndef MASTER_H
#define MASTER_H

#include <queue>
#include "dag.h"

class Master {
    string dagfile;
    string outfile;
    string errfile;
    DAG *dag;
    queue<int> idle;
    
    void submit_task(Task *t, int worker);
    void wait_for_result();
    void add_worker(int worker);
    bool has_idle_worker();
    void mark_worker_idle(int worker);
    int next_idle_worker();
    void merge_task_stdio(FILE *dest, const string &src, const string &stream);
public:
    Master(DAG *dag, const string &outfile, const string &errfile);
    ~Master();
    int run();
};

#endif /* MASTER_H */
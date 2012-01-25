#ifndef MASTER_H
#define MASTER_H

#include <queue>
#include "dag.h"

class Master {
    string dagfile;
    DAG dag;
    queue<int> idle;
    
    void submit_task(Task *t, int worker);
    void wait_for_result();
public:
    Master(const string &dagfile);
    ~Master();
    int run();
    void add_worker(int worker);
    bool has_idle_worker();
    void mark_worker_idle(int worker);
    int next_idle_worker();
};

#endif /* MASTER_H */
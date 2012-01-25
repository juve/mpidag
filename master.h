#ifndef MASTER_H
#define MASTER_H

#include <queue>
#include "dag.h"

class Master {
    string dagfile;
    DAG dag;
    queue<int> idle;
public:
    Master(const string &dagfile);
    ~Master();
    void run();    
};

#endif /* MASTER_H */
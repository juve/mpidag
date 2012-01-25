#ifndef DAG_H
#define DAG_H

#include <string>
#include <map>
#include <queue>
#include <vector>
#include <set>

using namespace std;

class Task {
public:
    string name;
    string command;
    vector<Task *> children;
    vector<Task *> parents;
    
    bool success;
    
    Task(const string &name, const string &command);
    ~Task();
};

class DAG {
    queue<Task *> ready;
    map<string, Task *> tasks;
    set<Task *> queue;
public:
    DAG();
    ~DAG();
    bool has_task(const string &name) const;
    Task *get_task(const string &name) const;
    void add_task(Task *task);
    void add_edge(const string &parent, const string &child);
    void mark_task_finished(Task *t, int exitcode);
    void queue_ready_task(Task *t);
    bool has_ready_task();
    Task *next_ready_task();
    bool is_finished();
    bool is_failed();
    void read(const string &filename);
    void write(const string &filename, bool rescue=true) const;
};

#endif /* DAG_H */
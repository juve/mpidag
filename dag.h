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
    
    bool done;
    
    Task(const string &name, const string &command);
    ~Task();
    
    bool is_ready();
    bool is_done();
};

class DAG {
    queue<Task *> ready;
    map<string, Task *> tasks;
    set<Task *> queue;
    FILE *rescue;
    int failures;
    int max_failures;
    
    // DAG files
    void read_dag(const string &filename);
    void add_task(Task *task);
    void add_edge(const string &parent, const string &child);
    
    // Rescue files
    void read_rescue(const string &filename);
    bool has_rescue();
    void open_rescue(const string &filename);
    void close_rescue();
    void write_rescue(Task *task);
    
    void queue_ready_task(Task *t);
    void init();
public:
    DAG(const string &dagfile);
    DAG(const string &dagfile, const string &oldrescue);
    DAG(const string &dagfile, const string &oldrescue, const string &newrescue, int max_failures = 0);
    ~DAG();
    bool max_failures_reached();
    bool has_task(const string &name) const;
    Task *get_task(const string &name) const;
    void mark_task_finished(Task *t, int exitcode);
    bool has_ready_task();
    Task *next_ready_task();
    bool is_finished();
    bool is_failed();
};

#endif /* DAG_H */
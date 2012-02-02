#ifndef DAG_H
#define DAG_H

#include <string>
#include <map>
#include <queue>
#include <vector>
#include <set>

class Task {
public:
    std::string name;
    std::string command;
    std::vector<Task *> children;
    std::vector<Task *> parents;
    
    bool success;
    int failures;
    
    Task(const std::string &name, const std::string &command);
    ~Task();
    
    bool is_ready();
};

class DAG {
    std::queue<Task *> ready;
    std::map<std::string, Task *> tasks;
    std::set<Task *> queue;
    FILE *rescue;
    int failures;
    int max_failures;
    int tries;
    
    // DAG files
    void read_dag(const std::string &filename);
    void add_task(Task *task);
    void add_edge(const std::string &parent, const std::string &child);
    
    // Rescue files
    void read_rescue(const std::string &filename);
    bool has_rescue();
    void open_rescue(const std::string &filename);
    void close_rescue();
    void write_rescue(Task *task);
    
    void queue_ready_task(Task *t);
    void init();
public:
    DAG(const std::string &dagfile);
    DAG(const std::string &dagfile, const std::string &oldrescue);
    DAG(const std::string &dagfile, const std::string &oldrescue, const std::string &newrescue, int max_failures = 0, int tries = 1);
    ~DAG();
    bool max_failures_reached();
    bool has_task(const std::string &name) const;
    Task *get_task(const std::string &name) const;
    void mark_task_finished(Task *t, int exitcode);
    bool has_ready_task();
    Task *next_ready_task();
    bool is_finished();
    bool is_failed();
};

#endif /* DAG_H */

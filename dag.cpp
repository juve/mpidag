#include <cstdio>
#include <cstring>
#include <map>
#include <vector>
#include "dag.h"
#include "failure.h"
#include "strlib.h"

#define MAX_LINE 16384

Task::Task(const string &name, const string &command, const vector<string> &args) {
    this->name = string(name);
    this->command = string(command);
    for (unsigned i=0; i<args.size(); i++) {
        this->args.push_back(args[i]);
    }
    this->finished = false;
}

Task::~Task() {
}

DAG::DAG() {
}

DAG::~DAG() {
    map<string, Task *>::iterator i;
    for (i = this->tasks.begin(); i != this->tasks.end(); i++) {
        delete (*i).second;
    }
}

bool DAG::has_task(const string &name) const {
    return this->tasks.find(name) != this->tasks.end();
}

Task *DAG::get_task(const string &name) const {
    if (!this->has_task(name)) {
        return NULL;
    }
    return (*(this->tasks.find(name))).second;
}

void DAG::add_task(Task *task) {
    if (this->has_task(task->name)) {
        failure("Duplicate task: %s\n", task->name.c_str());
    }
    this->tasks[task->name] = task;
}

void DAG::add_edge(const string &parent, const string &child) {
    if (!this->has_task(parent)) {
        failure("No such task: %s\n", parent.c_str());
    }
    if (!this->has_task(child)) {
        failure("No such task: %s\n", child.c_str());
    }
    
    Task *p = get_task(parent);
    Task *c = get_task(child);
    
    p->children.push_back(c);
    c->parents.push_back(p);
}

void DAG::read(const string &filename) {
    const char *DELIM = " \t\n\r";
    
    FILE *dagfile = fopen(filename.c_str(), "r");
    if (dagfile == NULL) {
        failures("Unable to open DAG file: %s", filename.c_str());
    }
    
    vector<Task *> tasks;
    
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, dagfile) != NULL) {
        string rec(line);
        trim(rec);
        
        // Blank lines
        if (rec.length() == 0) {
            continue;
        }
        
        // Comments
        if (rec[0] == '#') {
            continue;
        }
        
        if (rec.find("TASK", 0, 4) == 0) {
            vector<string> v;
            
            split(v, rec, DELIM, 3);
            
            if (v.size() < 3) {
                failure("Invalid TASK record: %s\n", line);
            }
            
            string name = v[1];
            string cmd = v[2];
            vector<string> args;
            if (v.size() > 3) {
                split_args(args, v[3]);
            }
            
            // Check for duplicate tasks
            if (this->has_task(name)) {
                failure("Duplicate task: %s", name.c_str());
            }
            
            Task *t = new Task(name, cmd, args);
            
            this->add_task(t);
            
            tasks.push_back(t);
        } else if (rec.find("EDGE", 0, 4) == 0) {
            
            vector<string> v;
            
            split(v, rec, DELIM, 2);
            
            if (v.size() < 3) {
                failure("Invalid EDGE record: %s\n", line);
            }
            
            string parent = v[1];
            string child = v[2];
            
            this->add_edge(parent, child);
        } else {
            failure("Invalid record type: %s", line);
        }
    }
    
    
    fclose(dagfile);
    
    // Now queue all root tasks
    for (unsigned i = 0; i < tasks.size(); i++) {
        Task *t = tasks[i];
        if (t->parents.size() == 0) {
            this->queue_ready_task(t);
        }
    }
}

void DAG::write(const string &filename, bool rescue) const {
    failure("Not implemented");
}

void DAG::queue_ready_task(Task *t) {
    this->ready.push(t);
}

void DAG::mark_task_finished(Task *t) {
    // Mark task
    t->finished = true;
    
    // Release ready children
    for (unsigned i=0; i<t->children.size(); i++) {
        Task *c = t->children[i];
        bool ready = true;
        for (unsigned j=0; j<c->parents.size(); j++) {
            Task *p = c->parents[j];
            if (!p->finished) {
                ready = false;
            }
        }
        if (ready) {
            this->queue_ready_task(c);
        }
    }
}

bool DAG::has_ready_task() {
    return !this->ready.empty();
}

Task *DAG::next_ready_task() {
    if (!this->has_ready_task()) {
        failure("No ready tasks");
    }
    Task *t = this->ready.front();
    this->ready.pop();
    return t;
}

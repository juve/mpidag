#include <map>
#include <vector>
#include "string.h"
#include "stdio.h"
#include "unistd.h"

#include "strlib.h"
#include "dag.h"
#include "failure.h"
#include "log.h"

#define MAX_LINE 16384

Task::Task(const std::string &name, const std::string &command) {
    this->name = name;
    this->command = command;
    this->success = false;
    this->failures = 0;
}

Task::~Task() {
}

bool Task::is_ready() {
    // A task is ready when all its parents are done
    if (this->parents.empty()) {
        return true;
    }
    
    for (unsigned j=0; j<this->parents.size(); j++) {
        Task *p = this->parents[j];
        if (!p->success) {
            return false;
        }
    }
    
    return true;
}

DAG::DAG(const std::string &dagfile) {
    this->max_failures = 0;
    this->tries = 1;
    this->rescue = NULL;
    this->read_dag(dagfile);
    this->init();
}

DAG::DAG(const std::string &dagfile, const std::string &oldrescue) {
    this->max_failures = 0;
    this->tries = 1;
    this->rescue = NULL;
    this->read_dag(dagfile);
    if (!oldrescue.empty()) {
        this->read_rescue(oldrescue);
    }
    this->init();
}

DAG::DAG(const std::string &dagfile, const std::string &oldrescue, const std::string &newrescue, int max_failures, int tries) {
    if (max_failures < 0) {
        failure("max_failures must be >= 0");
    }
    if (tries < 1) {
        failure("tries must be >= 1");
    }
    this->max_failures = max_failures;
    this->tries = tries;
    this->rescue = NULL;
    this->read_dag(dagfile);
    if (!oldrescue.empty()) {
        this->read_rescue(oldrescue);
    }
    if (!newrescue.empty()) {
        this->open_rescue(newrescue);
    }
    this->init();
}

DAG::~DAG() {
    // Close rescue file
    this->close_rescue();
    
    // Delete all tasks
    std::map<std::string, Task *>::iterator i;
    for (i = this->tasks.begin(); i != this->tasks.end(); i++) {
        delete (*i).second;
    }
}

void DAG::init() {
    this->failures = 0;
    
    // Queue all tasks that are ready, but not done
    std::map<std::string, Task *>::iterator i;
    for (i=this->tasks.begin(); i!=this->tasks.end(); i++) {
        Task *t = (*i).second;
        if (t->is_ready() && !t->success) {
            this->queue_ready_task(t);
        }
    }
}

bool DAG::has_task(const std::string &name) const {
    return this->tasks.find(name) != this->tasks.end();
}

Task *DAG::get_task(const std::string &name) const {
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

void DAG::add_edge(const std::string &parent, const std::string &child) {
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

void DAG::read_dag(const std::string &filename) {
    const char *DELIM = " \t\n\r";
    
    FILE *dagfile = fopen(filename.c_str(), "r");
    if (dagfile == NULL) {
        failures("Unable to open DAG file: %s", filename.c_str());
    }
    
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, dagfile) != NULL) {
        std::string rec(line);
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
            std::vector<std::string> v;
            
            split(v, rec, DELIM, 2);
            
            if (v.size() < 3) {
                failure("Invalid TASK record: %s\n", line);
            }
            
            std::string name = v[1];
            std::string cmd = v[2];
                        
            // Check for duplicate tasks
            if (this->has_task(name)) {
                failure("Duplicate task: %s", name.c_str());
            }
            
            Task *t = new Task(name, cmd);
            
            this->add_task(t);
        } else if (rec.find("EDGE", 0, 4) == 0) {
            
            std::vector<std::string> v;
            
            split(v, rec, DELIM, 2);
            
            if (v.size() < 3) {
                failure("Invalid EDGE record: %s\n", line);
            }
            
            std::string parent = v[1];
            std::string child = v[2];
            
            this->add_edge(parent, child);
        } else {
            failure("Invalid DAG record: %s", line);
        }
    }
    
    
    fclose(dagfile);
}

void DAG::read_rescue(const std::string &filename) {
    
    // Check if rescue file exists
    if (access(filename.c_str(), R_OK)) {
        if (errno == ENOENT) {
            // File doesn't exist
            return;
        }
        failures("Unable to read rescue file: %s", filename.c_str());
    }
    
    FILE *rescuefile = fopen(filename.c_str(), "r");
    if (rescuefile == NULL) {
        failures("Unable to open rescue file: %s", filename.c_str());
    }
    
    const char *DELIM = " \t\n\r";
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, rescuefile) != NULL) {
        std::string rec(line);
        trim(rec);
        
        // Blank lines
        if (rec.length() == 0) {
            continue;
        }
        
        // Comments
        if (rec[0] == '#') {
            continue;
        }
        
        if (rec.find("DONE", 0, 4) == 0) {
            std::vector<std::string> v;
            
            split(v, rec, DELIM, 1);
            
            if (v.size() < 2) {
                failure("Invalid DONE record: %s\n", line);
            }
            
            std::string name = v[1];
            
            if (!this->has_task(name)) {
                failure("Unknown task %s in rescue file", name.c_str());
            }
            
            Task *task = this->get_task(name);
            task->success = true;
        } else {
            failure("Invalid rescue record: %s", line);
        }
    }
    
    fclose(rescuefile);
}

void DAG::queue_ready_task(Task *t) {
    this->ready.push(t);
    this->queue.insert(t);
}

void DAG::open_rescue(const std::string &filename) {
    this->rescue = fopen(filename.c_str(), "a");
    if (this->rescue == NULL) {
        failure("Unable to open rescue file: %s", filename.c_str());
    }
    
    // Mark done tasks as done in the new rescue file
    std::map<std::string, Task *>::iterator i;
    for (i=this->tasks.begin(); i!=this->tasks.end(); i++) {
        Task *t = (*i).second;
        if (t->success) {
            this->write_rescue(t);
        }
    }
}

bool DAG::has_rescue() {
    return this->rescue != NULL;
}

void DAG::close_rescue() {
    if (this->has_rescue()) {
        fclose(this->rescue);
        this->rescue = NULL;
    }
}

void DAG::write_rescue(Task *task) {
    if (this->has_rescue()) {
        if (fprintf(this->rescue, "\nDONE %s", task->name.c_str()) < 0) {
            log_error("Error writing to rescue file: %s", strerror(errno));
        }
        if (fflush(this->rescue)) {
            log_error("Error flushing rescue file: %s", strerror(errno));
        }
    }
}

void DAG::mark_task_finished(Task *t, int exitcode) {
    
    if (exitcode == 0) {
        // Task succeeded
        t->success = true;
        this->write_rescue(t);
    } else {
        // Task failed
        t->failures += 1;

        //If job can be retried, then re-submit it
        if (t->failures < this->tries) {
            this->queue_ready_task(t);
            return;
        }
        
        // Otherwise count the failure
        this->failures += 1;
    }

    // Remove from the queue
    this->queue.erase(t);
    
    if (max_failures_reached()) {
        // Clear ready queue
        while (this->has_ready_task()) {
            Task *t = this->next_ready_task();
            this->queue.erase(t);
        }
    } else {
        // Release ready children
        for (unsigned i=0; i<t->children.size(); i++) {
            Task *c = t->children[i];
            if (c->is_ready()) {
                this->queue_ready_task(c);
            }
        }
    }
    
    // If we are finished, close rescue
    if (this->is_finished()) {
        this->close_rescue();
    }
}

bool DAG::max_failures_reached() {
    return this->failures >= this->max_failures && this->max_failures != 0;
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

bool DAG::is_finished() {
    return this->queue.empty();
}

bool DAG::is_failed() {
    bool finished = this->is_finished();
    if (!finished) {
        failure("Not finished");
    }
    
    bool success = true;
    std::map<std::string, Task *>::iterator i;
    for (i=this->tasks.begin(); i!=this->tasks.end(); i++) {
        Task *t = (*i).second;
        if (!t->success) {
            success = false;
            break;
        }
    }
    
    return !success;
}

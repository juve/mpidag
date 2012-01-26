#ifndef WORKER_H
#define WORKER_H

#include <string>

using namespace std;

class Worker {
    string outfile;
    string errfile;
public:
    Worker(const string &outfile, const string &errfile);
    ~Worker();
    int run();
};

#endif /* WORKER_H */
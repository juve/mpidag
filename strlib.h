#ifndef STRLIB_H
#define STRLIB_H

#include <vector>
#include <string>

using namespace std;

void trim(string &str, const string &delim = " \t\r\n");

void split(vector<string> &v, const string &line, const string &delim = " \t\r\n", unsigned maxsplits = 0);

void split_args(vector<string> &v, const string &line);


#endif /* STRLIB_H */

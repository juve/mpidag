#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>

using namespace std;

#define TAG_COMMAND 1
#define TAG_RESULT 2

void send_request(const string &name, const string &command, int worker);
void recv_request(string &name, string &command);
void send_response(const string &name, int exitcode);
void recv_response(string &name, int &exitcode, int &worker);

#endif /* PROTOCOL_H */
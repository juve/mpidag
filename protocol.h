#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>

#define TAG_COMMAND 1
#define TAG_RESULT 2
#define TAG_SHUTDOWN 3

void send_stdio_paths(const std::string &outfile, const std::string &errfile);
void recv_stdio_paths(std::string &outfile, std::string &errfile);
void send_request(const std::string &name, const std::string &command, int worker);
void send_shutdown(int worker);
int recv_request(std::string &name, std::string &command);
void send_response(const std::string &name, int exitcode);
void recv_response(std::string &name, int &exitcode, int &worker);
double collect_total_runtimes();
void send_total_runtime(double total_runtime);

#endif /* PROTOCOL_H */

#ifndef HTTP_HPP
#define HTTP_HPP

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include "http_response.h"

extern "C"
{
#include "visitor.h"
#include "http.h"
}

struct http_context
{
    std::string remote_addr;
    int remote_port;
    std::string body;
    std::string raw_method;
    enum http_method method;
    std::string path;
    std::string query_string;
    std::map<std::string, std::string> headers;
    const char *raw_request;
};
void log_http_request(http_context &context, int status, int length);
void parse_request(http_context &http_context, const char *buf);
#endif

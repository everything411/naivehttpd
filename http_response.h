#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string>

std::string HTTP_response_status(int status_code);
std::string HTTP_response_date();
std::string HTTP_response_content_info(std::string content_type, int content_len);
std::string HTTP_response_header(int status_code, std::string content_type, int content_len);
std::string HTTP_response_header2(int status_code, int content_length);
#endif

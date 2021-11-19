#include "http.hpp"
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

http_response *build_http_response(string &resp)
{
    int resp_length = resp.length();
    http_response *p = (http_response *)malloc(sizeof(http_response));
    p->length = resp_length;
    p->content = malloc(resp_length + 1 + 4);
    memcpy(p->content, resp.c_str(), resp_length);
    memcpy((char *)p->content + resp_length, "\r\n\r\n\0", 5);
    return p;
}

extern "C" void destroy_http_response(http_response *p)
{
    if (p)
    {
        if (p->content)
        {
            free(p->content);
        }
        free(p);
    }
}

static bool endsWith(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

extern "C" http_response *handle_request(const char *http_request, char *remote_addr, int remote_port)
{
    http_context http_context;
    http_context.remote_addr = remote_addr;
    http_context.remote_port = remote_port;
    http_context.raw_request = http_request;
    parse_request(http_context, http_request);

    // auto &request_key_map = http_context.headers;
    auto &method = http_context.method;
    auto &url = http_context.path;
    // auto &query_str = http_context.query_string;
    string header, content;
    string content_type;
    int content_len;
    string res;
    int status;
    if (url == "/418")
    {
        header = HTTP_response_header2(418, 13);
        content = "\r\nI'm a teapot\n";
        res = header + content;
        status = 418;
    }
    else if (endsWith(url, ".php")) // run cgi
    {
        if (method == UNKNOWN)
        {
            // 400 Bad request
            RawContent *ctnt_p = get_raw_by_path("/400.html");
            content = ctnt_p->raw_buf;
            content_type = ctnt_p->raw_type;
            content_len = ctnt_p->raw_len;
            header = HTTP_response_header(400, content_type, content_len);
            del_raw(ctnt_p);
            status = 400;
        }
        else
        {
            cgi_request request;
            request.request_method = http_context.raw_method.c_str();
            request.document_root = "./webroot";
            request.query_string = http_context.query_string.c_str();
            request.remote_addr = http_context.remote_addr.c_str();
            request.remote_port = http_context.remote_port;
            request.script_name = http_context.path.c_str();
            request.content_length = http_context.body.length();
            request.content = http_context.body.c_str();
            vector<string> vs;
            for (auto &i : http_context.headers)
            {
                stringstream ss;
                ss << "HTTP_";
                auto &key = i.first;
                auto &value = i.second;
                for (auto c : key)
                {
                    if (c == '-')
                    {
                        ss << '_';
                    }
                    else
                    {
                        ss << (char)toupper(c);
                    }
                }
                ss << '=';
                ss << value;
                vs.push_back(ss.str());
            }
            const char **header_env = (const char **)calloc(vs.size() + 1, sizeof(char *));
            for (int i = 0; i < (int)vs.size(); i++)
            {
                header_env[i] = vs[i].c_str();
            }
            auto p = http_context.headers.find("Content-Type");
            if (p != http_context.headers.end())
            {
                request.content_type = p->second.c_str();
            }
            else
            {
                request.content_type = "";
            }

            auto result = do_exec_cgi(request, header_env);
            free(header_env);
            content = result->response;
            // puts(result->response);
            status = 200;
            char *status_ptr = strstr(result->response, "Status:");
            if (status_ptr != NULL)
            {
                // puts(status_ptr);
                sscanf(status_ptr + 7, "%d", &status);
            }
            char *header_end_ptr = strstr(result->response, "\r\n\r\n");
            if (header_end_ptr != NULL)
            {
                content_len = result->length - (header_end_ptr - result->response + 4);
            }
            else
            {
                content_len = 0;
            }
            header = HTTP_response_header2(status, content_len);
            res = header + content;
            destroy_cgi_result(result);
        }
    }
    else // serve static web pages
    {
        if (method == GET)
        {
            RawContent *ctnt_p = get_raw_by_path(url.c_str());
            content = ctnt_p->raw_buf;
            content_type = ctnt_p->raw_type;
            content_len = ctnt_p->raw_len;
            if (ctnt_p->raw_exist)
            {
                header = HTTP_response_header(200, content_type, content_len);
                status = 200;
            }
            else
            {
                header = HTTP_response_header(404, content_type, content_len);
                status = 404;
            }
            del_raw(ctnt_p);
        }
        else if (method == HEAD)
        {
            RawContent *ctnt_p = get_raw_by_path(url.c_str());
            content_type = ctnt_p->raw_type;
            content_len = ctnt_p->raw_len;
            if (ctnt_p->raw_exist)
            {
                header = HTTP_response_header(200, content_type, content_len);
                status = 200;
            }
            else
            {
                header = HTTP_response_header(404, content_type, content_len);
                status = 404;
            }
            del_raw(ctnt_p);
        }
        else
        {
            header = HTTP_response_header(405, "text/plain", 23);
            content = "405 Method Not Allowed\n";
            status = 405;
        }
        res = header + content;
    }
    auto response = build_http_response(res);
    log_http_request(http_context, status, content.length());
    return response;
}

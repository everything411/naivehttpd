#ifndef HTTP_H
#define HTTP_H

struct http_response
{
    int length;
    void *content;
};
enum http_method
{
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS,
    TRACE,
    UNKNOWN
};
struct cgi_request
{
    const char *query_string;
    const char *request_method;
    int content_length;
    const char *content_type;
    const void *content;
    const char *script_name;
    const char *document_root;
    const char *remote_addr;
    int remote_port;
};
struct cgi_result
{
    int length;
    char *response;
};
void init_log();
struct http_response *handle_request(const char *http_request, char *remote_addr, int remote_port);
void destroy_cgi_result(struct cgi_result *p);
void destroy_http_response(struct http_response *p);
struct cgi_result *do_exec_cgi(struct cgi_request request, const char **extra_env);

#endif

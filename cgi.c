#include "http.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;

void destroy_cgi_result(struct cgi_result *p)
{
    if (p)
    {
        if (p->response)
        {
            free(p->response);
        }
        free(p);
    }
}

struct cgi_result *do_exec_cgi(struct cgi_request request, const char **extra_env)
{
    char *php_cgi_path = "/usr/bin/php-cgi";
    char *php_cgi_argv[] = {php_cgi_path, NULL};
    int fd[2];
    int fd2[2];
    pid_t pid;
    if (pipe(fd) < 0 || pipe(fd2) < 0)
    {
        perror("pipe");
        exit(1);
    }
    if ((pid = fork()) < 0)
    {
        perror("fork");
        exit(1);
    }
    if (pid)
    {
        close(fd[0]);
        close(fd2[1]);
        write(fd[1], request.content, request.content_length);
        struct cgi_result *p = malloc(sizeof(struct cgi_result));
        p->response = malloc(102400);
        p->length = 0;
        int n = 0;
        for (;;)
        {
            n = read(fd2[0], p->response + p->length, 4096);
            p->length += n;
            if (n <= 0)
            {
                p->response[p->length] = 0;
                break;
            }
        }
        waitpid(pid, NULL, 0);
        return p;
    }
    else
    {
        close(fd[1]);
        close(fd2[0]);
        dup2(fd[0], STDIN_FILENO);
        dup2(fd2[1], STDOUT_FILENO);

        char query_string[128];
        char request_method[32];
        char content_type[128];
        char content_length[32];
        char script_name[128];
        char document_root[128];
        char script_filename[256];
        char remote_addr[32];
        char remote_port[32];

        snprintf(query_string, sizeof(query_string), "QUERY_STRING=%s", request.query_string);
        snprintf(request_method, sizeof(request_method), "REQUEST_METHOD=%s", request.request_method);
        snprintf(content_type, sizeof(content_type), "CONTENT_TYPE=%s", request.content_type);
        snprintf(content_length, sizeof(content_length), "CONTENT_LENGTH=%d", request.content_length);
        snprintf(query_string, sizeof(query_string), "QUERY_STRING=%s", request.query_string);
        snprintf(script_name, sizeof(script_name), "SCRIPT_NAME=%s", request.script_name);
        snprintf(document_root, sizeof(document_root), "DOCUMENT_ROOT=%s", request.document_root);
        snprintf(script_filename, sizeof(script_filename), "SCRIPT_FILENAME=%s/%s", request.document_root,
                 request.script_name);
        snprintf(remote_addr, sizeof(remote_addr), "REMOTE_ADDR=%s", request.remote_addr);
        snprintf(remote_port, sizeof(remote_port), "REMOTE_PORT=%d", request.remote_port);
        // snprintf(query_string, sizeof(query_string), "QUERY_STRING=%s", request.query_string);
        char *new_environ[] = {query_string,
                               request_method,
                               content_type,
                               content_length,
                               script_filename,
                               script_name,
                               document_root,
                               remote_addr,
                               remote_port,
                               "GATEWAY_INTERFACE=CGI/1.1",
                               "SERVER_PROTOCOL=HTTP/1.1",
                               "REQUEST_SCHEME=http",
                               "SERVER_SOFTWARE=naivehttpd/0.1",
                               "SERVER_ADDR=127.0.0.1",
                               "SERVER_NAME=default_server",
                               "SERVER_PORT=80",
                               "REDIRECT_STATUS=200",
                               NULL};
        int c = 0;
        while (environ[c])
        {
            c++;
        }
        int c2 = 0;
        while (extra_env[c2])
        {
            c2++;
        }
        char **php_cgi_environ = malloc((c + c2) * sizeof(char *) + sizeof(new_environ));
        memcpy(php_cgi_environ, environ, c * sizeof(char *));
        memcpy(php_cgi_environ + c, extra_env, c2 * sizeof(char *));
        memcpy(php_cgi_environ + c + c2, new_environ, sizeof(new_environ));
        execve(php_cgi_path, php_cgi_argv, php_cgi_environ);
        printf("Status: 500 Internal Server Error\r\n");
        printf("Content-type: text/plain\r\n\r\nexecve error\n");
        exit(EXIT_FAILURE);
    }
}

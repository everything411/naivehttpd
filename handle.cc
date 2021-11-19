#include "http.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

using namespace std;

void parse_request(http_context &http_context, const char *buf)
{
    size_t i = 0, j = 0;
    char method[8] = {};

    char *url = new char[1024];
    char *query = new char[1024];
    size_t buflen = strlen(buf);
    while (i < buflen && j < 7 && !isspace(buf[i]))
    {
        method[j++] = buf[i++];
    }
    method[j] = '\0';
    http_context.raw_method = method;
    if (!strcmp(method, "GET"))
    {
        http_context.method = GET;
    }
    else if (!strcmp(method, "POST"))
    {
        http_context.method = POST;
    }
    else if (!strcmp(method, "PUT"))
    {
        http_context.method = PUT;
    }
    else if (!strcmp(method, "DELETE"))
    {
        http_context.method = DELETE;
    }
    else if (!strcmp(method, "PATCH"))
    {
        http_context.method = PATCH;
    }
    else if (!strcmp(method, "HEAD"))
    {
        http_context.method = HEAD;
    }
    else if (!strcmp(method, "OPTIONS"))
    {
        http_context.method = OPTIONS;
    }
    else if (!strcmp(method, "TRACE"))
    {
        http_context.method = TRACE;
    }
    else
    {
        http_context.method = UNKNOWN;
    }

    while (i < buflen && isspace(buf[i]))
        i++;

    j = 0;
    int k = 0;
    while (i < buflen && j < 1023 && !isspace(buf[i]))
    {
        url[j++] = buf[i++];
        if (buf[i] == '?')
        {
            k = 0;
            i++;
            while (i < strlen(buf) && k < 1023 && !isspace(buf[i]))
            {
                query[k++] = buf[i++];
            }
        }
    }

    url[j] = '\0';
    query[k] = '\0';

    http_context.path = url;
    http_context.query_string = query;

    while (i < buflen && buf[i] != '\n')
    {
        i++;
    }
    i++;
    while (i < buflen)
    {
        char *key = new char[64];
        char *value = new char[1024];
        j = 0;

        memset(key, '\0', 64);
        memset(value, '\0', 1024);
        while (i < buflen && buf[i] != ':')
        {
            key[j++] = buf[i++];
        }
        key[j] = '\0';
        j = 0;
        while (i < buflen && (buf[i] == ':' || isspace(buf[i])))
            i++;

        while (i < buflen && buf[i] != '\n')
        {
            value[j++] = buf[i++];
        }
        if (value[j - 1] == '\r')
        {
            value[j - 1] = 0;
        }
        value[j] = '\0';
        http_context.headers.insert(pair<string, string>(key, value));

        i = i + 1;
        delete[] key;
        delete[] value;
        if (buf[i + 1] == '\n')
            break;
    }
    delete[] url;
    delete[] query;
    http_context.body = &buf[i + 2];
}

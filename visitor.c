#include "visitor.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// #define _DEBUG

const char raw_buf_404[] = ""
                           "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"><title>404 Not Found"
                           "</title><h1>Not Found</h1><p>The requested URL was not found on the server. I"
                           "f you entered the URL manually please check your spelling and try again.</p>";

const char raw_type_html[] = "text/html";

// path of webroot, no last slash '/'
const char webroot_path[] = "./webroot";

const char default_document_name[] = "index.html";

int del_raw(RawContent *content)
{
    if (content == NULL)
    {
        return -1;
    }
    if (content->raw_buf != NULL)
    {
        free(content->raw_buf);
        content->raw_buf = NULL;
    }
    if (content->raw_type != NULL)
    {
        free(content->raw_type);
        content->raw_type = NULL;
    }
    free(content);
    return 1;
}

int is_folder(const char *path, int len)
{
    return path[len - 1] == '/';
}

int is_cgi(const char *path, int len)
{
    return len >= 4 && strcmp(path + len - 4, ".php") == 0;
}

int assign_content_type(RawContent *content, const char *type)
{
    if (content->raw_type != NULL)
    {
        free(content->raw_type);
        content->raw_type = NULL;
    }
    content->raw_type = (char *)malloc((strlen(type) + 1) * sizeof(char));
    strcpy(content->raw_type, type);
    return 1;
}

int get_content_type(const char *path, RawContent *content)
{
    char *pos = strrchr(path, '/');
    if (pos != NULL)
    {
        char *file_sufx = strrchr(pos, '.');
#ifdef _DEBUG
        printf("[DEBUG] FileSuffix: %s\n", file_sufx);
#endif
        if (file_sufx != NULL)
        {
            if (strcmp(file_sufx, ".html") == 0)
            {
                assign_content_type(content, "text/html");
            }
            else if (strcmp(file_sufx, ".txt") == 0)
            {
                assign_content_type(content, "text/plain");
            }
            else if (strcmp(file_sufx, ".css") == 0)
            {
                assign_content_type(content, "text/css");
            }
            else if (strcmp(file_sufx, ".js") == 0)
            {
                assign_content_type(content, "application/x-javascript");
            }
            else if (strcmp(file_sufx, ".png") == 0)
            {
                assign_content_type(content, "image/png");
            }
            else if (strcmp(file_sufx, ".jpg") == 0)
            {
                assign_content_type(content, "image/jpeg");
            }
            else if (strcmp(file_sufx, ".jpeg") == 0)
            {
                assign_content_type(content, "image/jpeg");
            }
            else if (strcmp(file_sufx, ".gif") == 0)
            {
                assign_content_type(content, "image/gif");
            }
            else
            {
                assign_content_type(content, "application/octet-stream");
            }
        }
        else
        {
#ifdef _DEBUG
            printf("[DEBUG] NoSuffix\n");
#endif
            assign_content_type(content, "application/octet-stream");
        }
        return 1;
    }
    else
    {
        return -1;
    }
}

int get_raw_by_fp(FILE *const fp, const char *path, RawContent *content)
{
    // get file size
    struct stat stat_buf;
    stat(path, &stat_buf);
    int len_f = stat_buf.st_size;
    // malloc space
    content->raw_buf = (char *)malloc((len_f + 1) * sizeof(char));
    content->raw_buf[len_f] = '\0';
    content->raw_len = len_f;
    // get content
    int n_read = fread(content->raw_buf, sizeof(char), len_f, fp);
#ifdef _DEBUG
    printf("[DEBUG] n_read len_f: %d %d\n", n_read, len_f);
    printf("[DEBUG] get_raw_by_fp():\n%s\n", content->raw_buf);
    assert(n_read == len_f);
#endif
    if (n_read != len_f)
        return -1;
    return 1;
}

/*
get_raw_by_path()
args:
  path: file path requested
retn:
  content: type RawContent*, a struct made up as follows:
    char* raw_buf   : buffer of raw content
    int raw_len     : length of raw_buf
    char* raw_type  : content-type of resource
    int raw_exist   : 1 for found, 0 for not found
*/
RawContent *get_raw_by_path(const char *path)
{
    int len_path = strlen(path);
    int webroot_path_len = strlen(webroot_path);
    int default_document_name_length = strlen(default_document_name);

#ifdef _DEBUG
    printf("[DEBUG] path: %s\n", path);

    // ensure valid path length
    assert(len_path > 0);
    assert(path[0] == '/');
#endif

    // translate into relative path
    // "${webroot_path}/${path}[/${default_document_name}]"
    char *new_path =
        (char *)malloc((webroot_path_len + len_path + default_document_name_length + 1 + 1 + 1) * sizeof(char));
    strcpy(new_path, webroot_path);
    strcat(new_path, "/");
    strcat(new_path, path);
    len_path = strlen(new_path);

#ifdef _DEBUG
    printf("[DEBUG] new_path: %s\n", new_path);
#endif

    // declare
    RawContent *content = (RawContent *)malloc(sizeof(RawContent));
    memset(content, 0, sizeof(RawContent));

    // parse path and get content-type
    if (is_folder(new_path, len_path))
    {
#ifdef _DEBUG
        printf("[DEBUG] PathType: folder\n");
#endif
        strcat(new_path, "/");
        strcat(new_path, default_document_name);
        assign_content_type(content, raw_type_html);
    }
    else if (is_cgi(new_path, len_path))
    {
#ifdef _DEBUG
        printf("[DEBUG] PathType: cgi\n");
#endif
        // get_cgi_result(new_path);
    }
    else
    {
#ifdef _DEBUG
        printf("[DEBUG] PathType: file\n");
#endif
        get_content_type(new_path, content);
#ifdef _DEBUG
        printf("[DEBUG] ContentType: %s\n", content->raw_type);
#endif
    }

#ifdef _DEBUG
    printf("[DEBUG] ReqrPath: %s\n", new_path);
#endif

    content->raw_exist = 1;

    // try to open file
    FILE *fp = fopen(new_path, "rb");
    if (fp == NULL)
    {
        // file not found
        content->raw_exist = 0;
        assign_content_type(content, raw_type_html);
        char *default_404_path = (char *)malloc((strlen(webroot_path) + 10) * sizeof(char));
        strcpy(default_404_path, webroot_path);
        strcat(default_404_path, "/404.html");
#ifdef _DEBUG
        printf("[DEBUG] 404path: %s\n", default_404_path);
#endif
        fp = fopen(default_404_path, "rb");
        if (fp == NULL)
        {
            // 404 file not exist
#ifdef _DEBUG
            printf("[DEBUG] FileRet: 404 default\n");
#endif
            // malloc space
            int len_buf_404 = strlen(raw_buf_404);
            content->raw_buf = (char *)malloc((len_buf_404 + 1) * sizeof(char));
            content->raw_len = len_buf_404;
            strcpy(content->raw_buf, raw_buf_404);
        }
        else
        {
#ifdef _DEBUG
            printf("[DEBUG] FileRet: 404 manual\n");
#endif
            get_raw_by_fp(fp, default_404_path, content);
            fclose(fp);
        }
        free(default_404_path);
        default_404_path = NULL;
    }
    else
    {
#ifdef _DEBUG
        printf("[DEBUG] FileRet: file\n");
#endif
        get_raw_by_fp(fp, new_path, content);
#ifdef _DEBUG
        printf("[DEBUG] get_raw_by_path():\n%s\n", content->raw_buf);
#endif
        fclose(fp);
    }
    free(new_path);
    new_path = NULL;
    return content;
}

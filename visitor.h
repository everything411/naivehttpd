#ifndef VISITOR_H_
#define VISITOR_H_

#include <stdio.h>

typedef struct rawcontent{
    char* raw_buf;
    int raw_len;
    char* raw_type;
    int raw_exist;
} RawContent;

RawContent* get_raw_by_path(const char*);
int del_raw(RawContent*);

int is_folder(const char*, int);
int is_cgi(const char*, int);

int assign_content_type(RawContent*, const char*);
int get_content_type(const char*, RawContent*);

int get_raw_by_fp(FILE* const, const char*, RawContent*);

#endif // VISITOR_H_

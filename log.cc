#include "http.hpp"
#include <cstdio>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
pid_t gettid(void)
{
    return syscall(SYS_gettid);
}
#endif
using namespace std;
const char *month_name[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char *day_name[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
FILE *logfp = stdout;
char *current_time_to_date_str(char *s, int n)
{
    time_t rawtime;
    struct tm tm;
    time(&rawtime);
    gmtime_r(&rawtime, &tm);
    snprintf(s, n, "%s, %02d %s %04d %02d:%02d:%02d GMT", day_name[tm.tm_wday], tm.tm_mday, month_name[tm.tm_mon],
             tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return s;
}
extern "C" void init_log()
{
    logfp = fopen("webroot/log/access.log", "a");
    if (logfp == NULL)
    {
        perror("open log file error, use stdout");
        logfp = stdout;
    }
    else
    {
        puts("write log to webroot/log/access.log");
    }
}
void do_log(const char *remote_ip, const char *request_firstline, int status, int length, const char *referer,
            const char *user_agent)
{
    char date[64];
    current_time_to_date_str(date, 64);
    fprintf(logfp, "(%d) %s - - [%s] \"%s\" %d %d \"%s\", \"%s\"\n", gettid(), remote_ip, date, request_firstline,
            status, length, referer, user_agent);
}
void log_http_request(http_context &context, int status, int length)
{
    const char *referer, *user_agent;
    int i = 0;
    referer = user_agent = "(null)";
    auto &headers = context.headers;
    auto referer_header = headers.find("Referer");

    if (referer_header != headers.end())
    {
        referer = referer_header->second.c_str();
    }
    auto user_agent_header = headers.find("User-Agent");
    if (user_agent_header != headers.end())
    {
        user_agent = user_agent_header->second.c_str();
    }
    while (context.raw_request[i] && context.raw_request[i] != '\n')
    {
        i++;
    }
    char *str = (char *)malloc(i + 1);
    memcpy(str, context.raw_request, i);
    str[i] = 0;
    if (str[i - 1] == '\r')
    {
        str[i - 1] = 0;
    }
    do_log(context.remote_addr.c_str(), str, status, length, referer, user_agent);
    free(str);
}

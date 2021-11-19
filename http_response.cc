#include "http_response.h"
#include <sstream>
#include <string>
using namespace std;
const char *descripions[600];

string HTTP_response_status(int status_code)
{
    if (!*descripions)
    {
        for (int i = 0; i < 600; i++)
        {
            descripions[i] = "";
        }
        descripions[100] = "Continue";
        descripions[101] = "Switching Protocol";
        descripions[200] = "OK";
        descripions[201] = "Created";
        descripions[202] = "Accepted";
        descripions[203] = "Non-Authoritative Information";
        descripions[204] = "No Content";
        descripions[205] = "Reset Content";
        descripions[206] = "Partial Content";
        descripions[300] = "Multiple Choice";
        descripions[301] = "Moved Permanently";
        descripions[302] = "Found";
        descripions[303] = "See Other";
        descripions[304] = "Not Modified";
        descripions[307] = "Temporary Redirect";
        descripions[308] = "Permanent Redirect";
        descripions[400] = "Bad Request";
        descripions[401] = "Unauthorized";
        descripions[403] = "Forbidden";
        descripions[404] = "Not Found";
        descripions[405] = "Method Not Allowed";
        descripions[406] = "Not Acceptable";
        descripions[407] = "Proxy Authentication Required";
        descripions[408] = "Request Timeout";
        descripions[409] = "Conflict";
        descripions[410] = "Gone";
        descripions[411] = "Length Required";
        descripions[412] = "Precondition Failed";
        descripions[413] = "Payload Too Large";
        descripions[414] = "URI Too Long";
        descripions[415] = "Unsupported Media Type";
        descripions[416] = "Range Not Satisfiable";
        descripions[417] = "Expectation Failed";
        descripions[418] = "I'm a teapot";
        descripions[426] = "Upgrade Required";
        descripions[428] = "Precondition Required";
        descripions[429] = "Too Many Requests";
        descripions[431] = "Request Header Fields Too Large";
        descripions[500] = "Internal Server Error";
        descripions[501] = "Not Implemented";
        descripions[502] = "Bad Gateway";
        descripions[503] = "Service Unavailable";
        descripions[504] = "Gateway Timeout";
        descripions[505] = "HTTP Version Not Supported";
    }
    string descrip;
    switch (status_code)
    {
    case 200:
        descrip = "OK";
        break;
    case 400:
        descrip = "Bad Request";
        break;
    case 403:
        descrip = "Forbidden";
        break;
    case 404:
        descrip = "Not Found";
        break;
    default:
        descrip = "";
    }
    stringstream sstr;
    sstr << "HTTP/1.1 " << status_code << " " << descripions[status_code] << "\r\n";
    string res = sstr.str();
    return res;
}

string HTTP_response_date()
{
    static const char *day_name[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char *month_name[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    time_t rawtime;
    struct tm tm;
    time(&rawtime);
    gmtime_r(&rawtime, &tm);
    char *timestr = (char *)malloc(50);
    sprintf(timestr, "%s, %02d %s %04d %02d:%02d:%02d GMT", day_name[tm.tm_wday], tm.tm_mday, month_name[tm.tm_mon],
            tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    stringstream sstr;
    sstr << "Date: " << timestr << "\r\n";
    string res = sstr.str();
    free(timestr);
    return res;
}

string HTTP_response_content_info(string content_type, int content_len)
{
    stringstream sstr;
    sstr << "Content-Type: " << content_type << "\r\n";
    sstr << "Content-Length: " << content_len << "\r\n";
    sstr << "Connection: keep-alive"
         << "\r\n";
    string res = sstr.str();
    return res;
}

string HTTP_response_header(int status_code, string content_type, int content_len)
{
    string res = "";
    res += HTTP_response_status(status_code);
    res += HTTP_response_date();
    res += HTTP_response_content_info(content_type, content_len);
    res += "Server: naivehttpd/0.1\r\n";
    res += "\r\n";
    return res;
}
string HTTP_response_header2(int status_code, int content_length)
{
    char str[128];
    snprintf(str, 128, "Content-Length: %d\r\n", content_length);
    string res = "";
    res += HTTP_response_status(status_code);
    res += HTTP_response_date();
    res += "Connection: keep-alive\r\n";
    res += "Server: naivehttpd/0.1\r\n";
    res += str;
    return res;
}

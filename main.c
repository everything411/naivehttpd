#define _GNU_SOURCE
#include "http.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define MAX_EVENTS 10
#define WORKER_COUNT 8
#define MAX_CONNECTION 1024
struct connection_context
{
    int bytes_sent;
    struct http_response *response;
    char ip_str[16];
    int port;
};
// fd as index
struct connection_context connection_contexts[MAX_CONNECTION + 64];
struct thread_info
{
    pthread_t thread_id; /* ID returned by pthread_create() */
    int thread_num;      /* Application-defined thread # */
};
int worker_epollfd[WORKER_COUNT];

char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
    switch (sa->sa_family)
    {
    case AF_INET:
        inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, maxlen);
        break;
    case AF_INET6:
        inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, maxlen);
        break;
    default:
        strncpy(s, "Unknown AF", maxlen);
        return NULL;
    }
    return s;
}
void signal_handler()
{
}
void *worker(void *arg)
{
    struct thread_info *tinfo = arg;
    int worker_id = tinfo->thread_num;
    int epollfd = worker_epollfd[worker_id];
    struct epoll_event events[MAX_EVENTS];
    int nfds;
    char *recv_buffer = malloc(8192);
    for (;;)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            if (errno == EINTR)
            {
                close(epollfd);
                free(recv_buffer);
                pthread_exit(NULL);
            }
            else
            {
                perror("epoll_pwait");
                exit(EXIT_FAILURE);
            }
        }
        for (int n = 0; n < nfds; ++n)
        {
            int fd = events[n].data.fd;
            if (events[n].events & EPOLLIN)
            {
                // printf("EPOLLIN\n");
                int recv_ret;
                recv_ret = recv(fd, recv_buffer, 8192, MSG_DONTWAIT);
                if (recv_ret == 0 || (recv_ret == -1 && errno != EAGAIN))
                {
                    close(fd);
                }
                else
                {
                    recv_buffer[recv_ret] = 0;
                    struct http_response *p =
                        handle_request(recv_buffer, connection_contexts[fd].ip_str, connection_contexts[fd].port);
                    int ret = send(fd, p->content, p->length, MSG_DONTWAIT);
                    if ((ret != p->length) && errno == EAGAIN)
                    {
                        connection_contexts[fd].response = p;
                        connection_contexts[fd].bytes_sent = ret;
                    }
                    else
                    {
                        destroy_http_response(p);
                        connection_contexts[fd].response = NULL;
                        connection_contexts[fd].bytes_sent = 0;
                    }
                }
            }
            else if (events[n].events & EPOLLOUT)
            {
                // printf("EPOLLOUT\n");
                struct http_response *p = connection_contexts[fd].response;
                if (!p)
                {
                    // printf("%d enter continue\n", fd);
                    continue;
                }
                int bytes_sent = connection_contexts[fd].bytes_sent;
                int ret = send(fd, p->content + bytes_sent, p->length - bytes_sent, MSG_DONTWAIT);
                if ((ret != p->length - bytes_sent) && errno == EAGAIN)
                {
                    connection_contexts[fd].bytes_sent += ret;
                }
                else
                {
                    destroy_http_response(p);
                    connection_contexts[fd].response = NULL;
                    connection_contexts[fd].bytes_sent = 0;
                }
            }
        }
    }
}
int main(int argc, char *argv[])
{
    init_log();
    printf("%d workers, use round robin\n", WORKER_COUNT);
    struct epoll_event ev, events[MAX_EVENTS];
    int listen_sock, conn_sock, nfds, epollfd;
    struct sockaddr_in servaddr;
    signal(SIGINT, signal_handler);
    /* socket(), bind() and listen() */
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(8080);
    int flag = 1;
    if (-1 == setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (bind(listen_sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    listen(listen_sock, MAX_CONNECTION);

    /* create master epoll */
    epollfd = epoll_create(32);
    if (epollfd == -1)
    {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    /* create worker epoll */
    for (int i = 0; i < WORKER_COUNT; i++)
    {
        worker_epollfd[i] = epoll_create(32);
        if (worker_epollfd[i] == -1)
        {
            perror("epoll_create");
            exit(EXIT_FAILURE);
        }
    }

    /* create worker threads */
    int s, num_threads;

    num_threads = WORKER_COUNT;
    struct thread_info *tinfo = calloc(num_threads, sizeof(*tinfo));
    if (tinfo == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    for (int tnum = 0; tnum < num_threads; tnum++)
    {
        tinfo[tnum].thread_num = tnum;
        /* The pthread_create() call stores the thread ID into
           corresponding element of tinfo[] */
        s = pthread_create(&tinfo[tnum].thread_id, NULL, &worker, &tinfo[tnum]);
        if (s != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    char ip_str[32];
    unsigned int chosen_worker = 0;
    for (;;)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            /* EINTR */
            if (errno == EINTR)
            {
                printf("master received SIGINT, stopping workers\n");
                for (int i = 0; i < WORKER_COUNT; ++i)
                {
                    pthread_kill(tinfo[i].thread_id, SIGINT);
                }
                /* block until all threads complete */
                for (int i = 0; i < WORKER_COUNT; ++i)
                {
                    pthread_join(tinfo[i].thread_id, NULL);
                }
                close(listen_sock);
                close(epollfd);
                exit(EXIT_SUCCESS);
            }
            else
            {
                perror("epoll_pwait");
                exit(EXIT_FAILURE);
            }
        }

        for (int n = 0; n < nfds; ++n)
        {
            struct sockaddr local;
            socklen_t addrlen = sizeof(local);
            // use accept4 for SOCK_NONBLOCK
            conn_sock = accept4(listen_sock, &local, &addrlen, SOCK_NONBLOCK);
            // conn_sock = accept(listen_sock, &local, &addrlen);
            if (conn_sock == -1)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
            ev.data.fd = conn_sock;
            // round robin scheduling
            chosen_worker = (chosen_worker + 1) % WORKER_COUNT;
            get_ip_str(&local, ip_str, sizeof(ip_str));
            // printf("[%d] accept %s, count=%d\n", tid, ip_str, c++);
            strcpy(connection_contexts[conn_sock].ip_str, ip_str);
            connection_contexts[conn_sock].port = ((struct sockaddr_in *)&local)->sin_port;
            if (epoll_ctl(worker_epollfd[chosen_worker], EPOLL_CTL_ADD, conn_sock, &ev) == -1)
            {
                perror("epoll_ctl: conn_sock");
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
}

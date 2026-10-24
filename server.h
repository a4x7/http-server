#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>

#define LOCAL_HOST "127.0.0.1"

typedef struct{
    const char *s_domain;
    const char *s_port;
    const char *s_ipver;
    char s_ipstr[INET6_ADDRSTRLEN];
    int s_socket;
    struct addrinfo s_ai;
} Server;

typedef enum{
    BLOCKING,
    NON_BLOCKING,
} CONNECTION_TYPE;

typedef struct{
    int c_socket;
    struct sockaddr_storage c_sa;
    CONNECTION_TYPE c_type; 
    const char *c_ipver;
    char c_ipstr[INET6_ADDRSTRLEN];
} Connection;

typedef enum{
    STREAM_SOCKET,
    DGRAM_SOCKET
} SOCKET_TYPE;

typedef enum{
    IPV4,
    IPV6,
    UNSPECIFIED
} IP_VERSION;

typedef enum{
    NONE,
    PASSIVE
} S_FLAGS;

const Server serverConstruct(const char *domain, const char *port, IP_VERSION family, SOCKET_TYPE socket_type, int backlog, S_FLAGS flags, int *sockopts, int *optlevels, void **optvals,size_t *optsizes, size_t optarrsize);

const Connection acceptConnection(Server S, CONNECTION_TYPE type, int *sockopts, int *optlevels, void **optvals, size_t *optsizes, size_t optarrsize);

#endif

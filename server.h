#ifndef SERVER_H
#define SERVER_H

#include "heapString.h"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>

#define LOCAL_HOST "127.0.0.1"

typedef enum{
    STREAM_SOCKET,
    DGRAM_SOCKET
} SOCKET_TYPE;

typedef enum{
    IPV4,
    IPV6,
    UNSPECIFIED
} IP_VERSION;

typedef struct{
    const char *s_domain;
    in_port_t s_port;
    IP_VERSION ipver;
    SOCKET_TYPE so_type;
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
    SOCKET_TYPE so_type;
    IP_VERSION ipver;
    char c_ipstr[INET6_ADDRSTRLEN];
} Connection;

typedef enum{
    NONE = 0,
    PASSIVE = 1,
} S_FLAGS;

typedef enum{
    SHUT_READ,
    SHUT_WRITE,
    SHUT_READ_WRITE,
} SHUT_FLAG;

const Server serverConstruct(const char *domain, unsigned short port, IP_VERSION family, SOCKET_TYPE socket_type, S_FLAGS flags, int *sockopts, int *optlevels, void **optvals,size_t *optsizes, size_t optarrsize);

const Connection connectToServer(const char *domain, unsigned short port, IP_VERSION family, SOCKET_TYPE socket_type, S_FLAGS flags, int *sockopts, int *optlevels, void **optvals,size_t *optsizes, size_t optarrsize);

void serverStartListening(Server s, int backlog);

const Connection acceptStreamConnection(const Server S, CONNECTION_TYPE type, int *sockopts, int *optlevels, void **optvals, size_t *optsizes, size_t optarrsize);

ssize_t sendToConnection(Connection C, const String *buf);

ssize_t receiveStream(Connection C, String *buf);

ssize_t receiveDatagram(const Server S, Connection *C, String *buf);

int shutServer(const Server S, SHUT_FLAG flag);

int shutConnection(const Connection C, SHUT_FLAG flag);

#endif

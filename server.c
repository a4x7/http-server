#include "server.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>

const Server serverConstruct(const char *domain, const char *port, IP_VERSION family, SOCKET_TYPE socket_type, int backlog, S_FLAGS flags, int *sockopts, int *optlevels, void **optvals, size_t *optsizes, size_t optarrsize){
    int status;
    Server S;
    struct addrinfo *res, *ptr;
    void *addrServer;

    memset(&S.s_ai, 0, sizeof(S.s_ai));
    S.s_domain = domain;
    S.s_port = port;
    switch(family){
        case IPV4:
            S.s_ai.ai_family = AF_INET;
            break;
        case IPV6:
            S.s_ai.ai_family = AF_INET6;
            break;
        case UNSPECIFIED:
            S.s_ai.ai_family = AF_UNSPEC;
            break;
        default:
            perror("Server: invalid protocol type\n");
            exit(1);
    }
    switch(socket_type){
        case STREAM_SOCKET:
            S.s_ai.ai_socktype = SOCK_STREAM;
            break;
        case DGRAM_SOCKET:
            S.s_ai.ai_socktype = SOCK_DGRAM;
            break;
        default:
            perror("Server: invalid socket type\n");
            exit(1);
    }
    switch(flags){
        case NONE:
            break;
        case PASSIVE:
            S.s_ai.ai_flags |= AI_PASSIVE;
            break;
        default:
            perror("Server: invalid flags\n");
            exit(1);
    }

    if((status = getaddrinfo(S.s_domain, S.s_port, &S.s_ai, &res)) != 0){
        fprintf(stderr, "Server: %s\n", gai_strerror(status));
        exit(1);
    }

    ptr = res;
    while(ptr != NULL){
        if((S.s_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1){
            perror("Server: socket error\n");
            ptr = ptr->ai_next;
            continue;
        };
        if(sockopts != NULL){
            for(size_t i = 0; i < optarrsize; i++){
                if((setsockopt(S.s_socket, optlevels[i], sockopts[i], optvals[i], optsizes[i])) == -1){
                    perror("Server: cannot set socket option\n");
                    freeaddrinfo(res);
                    shutdown(S.s_socket, SHUT_RDWR);
                    exit(1);
                }
            }
        }
        if(bind(S.s_socket, ptr->ai_addr, ptr->ai_addrlen) == -1){
            perror("Server: bind error\n");
            ptr = ptr->ai_next;
            continue;
        };
        break;
    }
    if(ptr == NULL){
        perror("Server: failed to bind\n");
        freeaddrinfo(res);
        shutdown(S.s_socket, SHUT_RDWR);
        exit(1);
    }

    if(ptr->ai_family == AF_INET){ // IPv4
        addrServer = &(((struct sockaddr_in*)ptr->ai_addr)->sin_addr);
        S.s_ipver = "IPv4";
    } else if(ptr->ai_family == AF_INET6){ // IPv6
        addrServer = &(((struct sockaddr_in6*)ptr->ai_addr)->sin6_addr);
        S.s_ipver = "IPv6";
    } else{
        perror("Server: protocol not supported\n");
        freeaddrinfo(res);
        shutdown(S.s_socket, SHUT_RDWR);
        exit(1);
    }
    if(inet_ntop(ptr->ai_family, addrServer, S.s_ipstr, INET6_ADDRSTRLEN) == NULL){
        perror("Server: inet_ntop failed\n");
        freeaddrinfo(res);
        shutdown(S.s_socket, SHUT_RDWR);
        exit(1);
    };

    freeaddrinfo(res);

    if(listen(S.s_socket, backlog) == -1){
        perror("Server: failed to start listening\n");
        shutdown(S.s_socket, SHUT_RDWR);
        exit(1);
    }

    printf("Server: ready\n");

    return S;
}

const Connection acceptConnection(Server S, CONNECTION_TYPE type, int *sockopts, int *optlevels, void **optvals, size_t* optsizes, size_t optarrsize){
    Connection new_connection;
    memset(&new_connection, 0, sizeof(new_connection));
    new_connection.c_type = type;
    socklen_t sa_len = sizeof new_connection.c_sa;
    printf("Waiting for connection...\n");
    if((new_connection.c_socket = accept(S.s_socket, (struct sockaddr*)&new_connection.c_sa, &sa_len)) == -1){
        perror("Server: failed to accept connection\n");
        shutdown(S.s_socket, SHUT_RDWR);
        exit(1);
    }
    if(new_connection.c_type == NON_BLOCKING){
        if(fcntl(new_connection.c_socket, F_SETFL, fcntl(new_connection.c_socket, F_GETFL) | O_NONBLOCK) == -1){
            perror("Server: cannot set socket to non-blocking\n");
            shutdown(S.s_socket, SHUT_RDWR);
            shutdown(new_connection.c_socket, SHUT_RDWR);
            exit(1);
        }
    }
    void *addrptr;
    if(new_connection.c_sa.ss_family == AF_INET){
        addrptr = &((struct sockaddr_in*)&new_connection.c_sa)->sin_addr;
    } else if(new_connection.c_sa.ss_family == AF_INET6){
        addrptr = &((struct sockaddr_in6*)&new_connection.c_sa)->sin6_addr;
    } else{
        perror("Server: protocol not supported\n");
        shutdown(S.s_socket, SHUT_RDWR);
        shutdown(new_connection.c_socket, SHUT_RDWR);
        exit(1);
    }
    if(inet_ntop(new_connection.c_sa.ss_family, addrptr, new_connection.c_ipstr, INET6_ADDRSTRLEN) == NULL){
        perror("Server: inet_ntop failed\n");
        shutdown(S.s_socket, SHUT_RDWR);
        shutdown(new_connection.c_socket, SHUT_RDWR);
        exit(1);
    }
    if(sockopts != NULL){
        for(size_t i = 0; i < optarrsize; i++){
            if((setsockopt(S.s_socket, optlevels[i], sockopts[i], optvals[i], optsizes[i])) == -1){
                perror("Server: cannot set socket option\n");
                shutdown(S.s_socket, SHUT_RDWR);
                shutdown(new_connection.c_socket, SHUT_RDWR);
                exit(1);
            }
        }
    }
    return new_connection;
}

#include "server.h"
#include "heapString.h"
#include <arpa/inet.h>
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <bits/sockaddr.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const Server serverConstruct(const char *domain, in_port_t port, IP_VERSION family, SOCKET_TYPE socket_type, S_FLAGS flags, int *sockopts, int *optlevels, void **optvals, size_t *optsizes, size_t optarrsize){
    int status;
    Server S;
    struct addrinfo *res, *ptr;
    void *addrServer;

    memset(&S.s_ai, 0, sizeof(S.s_ai));
    S.s_domain = domain;
    S.s_port = port;
    S.ipver = family;
    S.so_type = socket_type;
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
    if(flags & PASSIVE)
        S.s_ai.ai_flags |= AI_PASSIVE;

    char sport[6];
    snprintf(sport, sizeof(sport), "%d", port);
    if((status = getaddrinfo(S.s_domain, sport, &S.s_ai, &res)) != 0){
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
                    fprintf(stderr, "Server: %s\n", strerror(errno));
                    freeaddrinfo(res);
                    shutdown(S.s_socket, SHUT_RDWR);
                    exit(1);
                }
            }
        }
        if(bind(S.s_socket, ptr->ai_addr, ptr->ai_addrlen) == -1){
            fprintf(stderr, "Server: %s\n", strerror(errno));
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
    } else if(ptr->ai_family == AF_INET6){ // IPv6
        addrServer = &(((struct sockaddr_in6*)ptr->ai_addr)->sin6_addr);
    } else{
        perror("Server: protocol not supported\n");
        freeaddrinfo(res);
        shutdown(S.s_socket, SHUT_RDWR);
        exit(1);
    }
    if(inet_ntop(ptr->ai_family, addrServer, S.s_ipstr, INET6_ADDRSTRLEN) == NULL){
        fprintf(stderr, "Server: %s\n", strerror(errno));
        freeaddrinfo(res);
        shutdown(S.s_socket, SHUT_RDWR);
        exit(1);
    };

    freeaddrinfo(res);

    printf("Server: ready\n");

    return S;
}

const Connection connectToServer(const char *domain, in_port_t port, IP_VERSION family, SOCKET_TYPE socket_type, S_FLAGS flags, int *sockopts, int *optlevels, void **optvals, size_t *optsizes, size_t optarrsize){
    int status;
    Connection C;
    struct addrinfo ai, *res, *ptr;
    void *addrConnection;

    memset(&C, 0 ,sizeof(Connection));
    memset(&ai, 0, sizeof(struct addrinfo));
    C.c_type = BLOCKING;
    C.ipver = family;
    C.so_type = socket_type;
    switch(family){
        case IPV4:
            ai.ai_family = AF_INET;
            break;
        case IPV6:
            ai.ai_family = AF_INET6;
            break;
        case UNSPECIFIED:
            ai.ai_family = AF_UNSPEC;
            break;
        default:
            perror("Client: invalid protocol type\n");
            exit(1);
    }
    switch(socket_type){
        case STREAM_SOCKET:
            ai.ai_socktype = SOCK_STREAM;
            break;
        case DGRAM_SOCKET:
            ai.ai_socktype = SOCK_DGRAM;
            break;
        default:
            perror("Client: invalid socket type\n");
            exit(1);
    }
    if(flags & PASSIVE)
        ai.ai_flags |= AI_PASSIVE;

    char sport[6];
    snprintf(sport, sizeof(sport), "%d", port);
    if((status = getaddrinfo(domain, sport, &ai, &res)) != 0){
        fprintf(stderr, "CLient: %s\n", gai_strerror(status));
        exit(1);
    }

    ptr = res;
    while(ptr != NULL){
        if((C.c_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1){
            fprintf(stderr, "Client: %s\n", strerror(errno));
            ptr = ptr->ai_next;
            continue;
        };
        if(sockopts != NULL){
            for(size_t i = 0; i < optarrsize; i++){
                if((setsockopt(C.c_socket, optlevels[i], sockopts[i], optvals[i], optsizes[i])) == -1){
                    fprintf(stderr, "Client: %s\n", strerror(errno));
                    freeaddrinfo(res);
                    shutdown(C.c_socket, SHUT_RDWR);
                    exit(1);
                }
            }
        }
        break;
    }
    if(ptr == NULL){
        perror("Client: failed to connect\n");
        freeaddrinfo(res);
        shutdown(C.c_socket, SHUT_RDWR);
        exit(1);
    }

    if(ptr->ai_family == AF_INET){ // IPv4
        addrConnection = &(((struct sockaddr_in*)ptr->ai_addr)->sin_addr);
    } else if(ptr->ai_family == AF_INET6){ // IPv6
        addrConnection = &(((struct sockaddr_in6*)ptr->ai_addr)->sin6_addr);
    } else{
        perror("Client: protocol not supported\n");
        freeaddrinfo(res);
        shutdown(C.c_socket, SHUT_RDWR);
        exit(1);
    }
    if(inet_ntop(ptr->ai_family, addrConnection, C.c_ipstr, INET6_ADDRSTRLEN) == NULL){
        fprintf(stderr, "Client: %s\n", strerror(errno));
        freeaddrinfo(res);
        shutdown(C.c_socket, SHUT_RDWR);
        exit(1);
    };

    freeaddrinfo(res);

    if(connect(C.c_socket, ptr->ai_addr, ptr->ai_addrlen) == -1){
        fprintf(stderr, "Client: %s\n", strerror(errno));
        shutdown(C.c_socket, SHUT_RDWR);
        exit(1);
    }

    printf("Client: connected to %s\n", C.c_ipstr);

    return C;
}

void serverStartListening(Server S, int backlog){
    if(listen(S.s_socket, backlog) == -1){
        fprintf(stderr, "Server: %s\n", strerror(errno));
        shutdown(S.s_socket, SHUT_RDWR);
        exit(1);
    }
}

const Connection acceptStreamConnection(const Server S, CONNECTION_TYPE type, int *sockopts, int *optlevels, void **optvals, size_t* optsizes, size_t optarrsize){
    Connection new_connection;
    memset(&new_connection, 0, sizeof(new_connection));
    new_connection.c_type = type;
    socklen_t sa_len = sizeof new_connection.c_sa;
    printf("Server: waiting for connection...\n");
    if((new_connection.c_socket = accept(S.s_socket, (struct sockaddr*)&new_connection.c_sa, &sa_len)) == -1){
        fprintf(stderr, "Server: %s\n", strerror(errno));
        shutdown(S.s_socket, SHUT_RDWR);
        exit(1);
    }
    if(new_connection.c_type == NON_BLOCKING){
        if(fcntl(new_connection.c_socket, F_SETFL, fcntl(new_connection.c_socket, F_GETFL) | O_NONBLOCK) == -1){
            fprintf(stderr, "Server: %s\n", strerror(errno));
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
        fprintf(stderr, "Server: %s\n", strerror(errno));
        shutdown(S.s_socket, SHUT_RDWR);
        shutdown(new_connection.c_socket, SHUT_RDWR);
        exit(1);
    }
    new_connection.so_type = STREAM_SOCKET;
    if(sockopts != NULL){
        for(size_t i = 0; i < optarrsize; i++){
            if((setsockopt(new_connection.c_socket, optlevels[i], sockopts[i], optvals[i], optsizes[i])) == -1){
                fprintf(stderr, "Server: %s\n", strerror(errno));
                shutdown(S.s_socket, SHUT_RDWR);
                shutdown(new_connection.c_socket, SHUT_RDWR);
                exit(1);
            }
        }
    }
    printf("Server: received request from %s\n", new_connection.c_ipstr);
    return new_connection;
}

int shutServer(const Server S, SHUT_FLAG flag){
    if(flag & SHUT_READ)
        return shutdown(S.s_socket, SHUT_RD);
    if(flag & SHUT_WRITE)
        return shutdown(S.s_socket, SHUT_WR);
    if(flag & SHUT_READ_WRITE)
        return shutdown(S.s_socket, SHUT_RDWR);
    return -1;
}

int shutConnection(const Connection C, SHUT_FLAG flag){
    if(flag == SHUT_READ)
        return shutdown(C.c_socket, SHUT_RD);
    if(flag == SHUT_WRITE)
        return shutdown(C.c_socket, SHUT_WR);
    if(flag == SHUT_READ_WRITE)
        return shutdown(C.c_socket, SHUT_RDWR);
    return -1;
}

ssize_t sendToConnection(Connection C, const String *buf){
    if(buf == NULL || buf->n == 0)
        return 0;
    ssize_t size = buf->n;
    if(C.so_type == STREAM_SOCKET){
        ssize_t total_sent = 0;
        while(total_sent < size){
            ssize_t sent = send(C.c_socket, string_read(buf)+total_sent, size-total_sent, 0);
            if(sent == -1){
                fprintf(stderr, "Server: %s\n", strerror(errno));
                return -1;
            }
            total_sent += sent;
        }
        return total_sent;
    } else if(C.so_type == DGRAM_SOCKET){
        ssize_t total_sent = sendto(C.c_socket, string_read(buf), size, 0, (struct sockaddr*)&C.c_sa, sizeof C.c_sa);
        if(total_sent == -1){
            fprintf(stderr, "Server: %s\n", strerror(errno));
            return -1;
        }
        return total_sent;
    }
    return -1;
}

ssize_t receiveStream(Connection C, String *strBuf){
    if(strBuf == NULL)
        return 0;
    char buf[4096];
    ssize_t total_received = 0;
    while(1){
        ssize_t received = recv(C.c_socket, buf, 4096, 0);
        if(received == -1){
            if(C.c_type == NON_BLOCKING && (errno == EWOULDBLOCK || errno == EAGAIN))
                break;
            fprintf(stderr, "Server: %s\n", strerror(errno));
            return -1;
        }
        if(received == 0)
            break;
        total_received += received;
        string_raw_cat(strBuf, buf, received);
        if(buf[received] == '\0')
            break;
    }
    return total_received;
}

ssize_t receiveDatagram(const Server S, Connection *from, String *strBuf){
    if(strBuf == NULL || from == NULL)
        return 0;
    memset(from, 0, sizeof(Connection));
    char buf[65527];
    ssize_t total_received = 0;
    socklen_t sa_len = sizeof from->c_sa;
    total_received = recvfrom(S.s_socket, buf, 65527, 0, (struct sockaddr*)&from->c_sa, &sa_len);
    if(total_received == -1){
        fprintf(stderr, "Server: %s\n", strerror(errno));
        return -1;
    }
    string_raw_cat(strBuf, buf, total_received);
    from->c_socket = S.s_socket;
    from->so_type = DGRAM_SOCKET;
    void *addr;
    if(from->c_sa.ss_family == AF_INET){
        addr = &(((struct sockaddr_in*)&from->c_sa)->sin_addr);
        from->ipver = IPV4;
    } else if(from->c_sa.ss_family == AF_INET6){
        addr = &(((struct sockaddr_in6*)&from->c_sa)->sin6_addr);
        from->ipver = IPV6;
    }
    inet_ntop(from->c_sa.ss_family, addr, from->c_ipstr, INET6_ADDRSTRLEN);
    printf("Server: received request from %s\n", from->c_ipstr);
    return total_received;
}

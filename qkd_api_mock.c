#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netdb.h>
#include <errno.h>
#include <strings.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/random.h>
#include "qkd_api.h"
#include <assert.h>
#include "qkd_debug.h"

/* TODO: Remove superfluous header files */

/* Current limitations in this mock:
 * - More than one concurrent connection not yet supported
 * - Pre-defined key handles (section 6.1.3 and 6.1.4) not yet supported
 * - IPv6 is not yet supported (only IPv4)
 * - Timeout on blocking connect is not yet supported (timeout is ignored)
 */

#define BUFSIZE 1024

#define QKD_PORT 8080

QKD_qos_t current_qos;

typedef struct qkd_session_t {
    char *destination;
    QKD_key_handle_t key_handle;
    QKD_qos_t qos;
    int listen_sock;
} QKD_SESSION;

/** 
 * Create a listening socket.
 * Create a listening socket to receive incoming connections from the remote QKD peer. Both QKD
 * peers create a listening socket.
 */
static int create_listen_socket()
{
    /* Create the socket. */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    QKD_fatal_with_errno_if(sock == -1, "socket failed");

    /* The client and server may run on the same host. In that case we want to allow both of them
    to create a listening socket for the same port. */
    int on = 1;
    int result = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char*)&on, sizeof(on));
    QKD_fatal_with_errno_if(result != 0, "setsockopt SO_REUSEPORT failed");

    /* We want to be able to bind again while a previous socket for the same port is still in the
    lingering state. */
    result = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
    QKD_fatal_with_errno_if(result != 0, "setsockopt SO_REUSEADDR failed");

    /* Bind the socket to the QKD port and the wildcard address. */
    struct sockaddr_in listen_address; 
    bzero(&listen_address, sizeof(listen_address));
    listen_address.sin_family = AF_INET; 
    listen_address.sin_addr.s_addr = htonl(INADDR_ANY); 
    listen_address.sin_port = htons(QKD_PORT);
    result = bind(sock, (const struct sockaddr *) &listen_address, sizeof(listen_address));
    QKD_fatal_with_errno_if(result != 0, "bind failed");

    return sock;
}

QKD_SESSION *qkd_session_new(char *destination, QKD_qos_t qos)
{
    QKD_SESSION *session = malloc(sizeof(QKD_SESSION));
    QKD_fatal_if(session == NULL, "malloc failed");
    session->destination = strdup(destination);
    QKD_key_handle_set_random(&session->key_handle);  /* TODO: support chosen key_handle */
    session->qos = qos;
    session->listen_sock = create_listen_socket();
    return session;
}

void qkd_session_delete(QKD_SESSION *session)
{
    assert(session != NULL);
    int result = close(session->listen_sock);
    QKD_fatal_with_errno_if(result == -1, "close failed");
    free(session);
}

static QKD_SESSION *qkd_session = NULL;  /* TODO: For now this is the one and only session */

QKD_RC QKD_open(char *destination, QKD_qos_t qos, QKD_key_handle_t *key_handle)
{
    assert(key_handle != NULL);
    assert(qkd_session == NULL);   /* TODO: For now we only support one session */
    qkd_session = qkd_session_new(destination, qos);
    *key_handle = qkd_session->key_handle;
    return QKD_RC_SUCCESS;
}

QKD_RC QKD_connect_nonblock(const QKD_key_handle_t *key_handle)
{
    /* TODO: Not yet implemented. */
    return QKD_RC_NOT_SUPPORTED;
}

QKD_RC QKD_connect_blocking(const QKD_key_handle_t *key_handle, uint32_t timeout)
{
    return QKD_RC_SUCCESS;
}

QKD_RC QKD_get_key(const QKD_key_handle_t *key_handle, char* key_buffer) {
    /* TODO: Implement this; fixed shared secret for now */
    assert(key_buffer != NULL);
    for (size_t i = 0; i < current_qos.requested_length; i++) {
        key_buffer[i] = i;
    }
    return QKD_RC_SUCCESS;
}

QKD_RC QKD_PLACEHOLDER_FOR_OLD_CODE(const QKD_key_handle_t *key_handle, char* key_buffer) {
    // const char* hostname = "localhost";
    // const char* portname = "8080";
    // char buf[BUFSIZE];
    // struct addrinfo hints;
    // memset(&hints, 0, sizeof(hints));
    // hints.ai_family = AF_UNSPEC;
    // hints.ai_socktype = SOCK_STREAM;
    // hints.ai_protocol = 0;
    // hints.ai_flags = AI_ADDRCONFIG;
    // struct addrinfo* res = 0;
    // int err = getaddrinfo(hostname, portname, &hints, &res);
    // if (err != 0) {
    //     error("ERROR with getaddrinfo");
    // }

    // printf("create socket...\n");
    // int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    // if (sd == -1) {
    //     error("ERROR opening socket");
    // }

    // int optval = 1;
    // setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    // int reuseaddr = 1;
    // if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) == -1) {
    //     error("ERROR with setsockopt");
    // }

    // if (bind(sd, res->ai_addr, res->ai_addrlen) == -1) {
    //     /************
    //      *  CLIENT  *
    //      ************/
    //     if (connect(sd, res->ai_addr, res->ai_addrlen) < 0) {
    //         error("ERROR connecting");
    //     }

    //     /* Send key_handle first */
    //     int n = write(sd, key_handle, QKD_KEY_HANDLE_SIZE);
    //     if (n < 0) {
    //         error("ERROR writing to socket");
    //     }

    //     /* Check whether key_handle was accepted */
    //     n = read(sd, buf, sizeof(char));
    //     if (n < 0) {
    //         error("ERROR reading from socket");
    //     }

    //     if (buf[0] != 0) {
    //         /* key_handle is different from what was expected */
    //         printf("--> WRONG key_handle\n");
    //         freeaddrinfo(res);
    //         close(sd);
    //         return QKD_RC_GET_KEY_FAILED;
    //     }

    //     /* key_handle is correct */
    //     n = read(sd, key_buffer, current_qos.requested_length);
    //     if (n < 0) {
    //         error("ERROR reading from socket");
    //     }
    //     freeaddrinfo(res);
    //     close(sd);
    //     return QKD_RC_SUCCESS;
    // } else {
    //     /************
    //      *  SERVER  *
    //      ************/
    //     if (listen(sd, SOMAXCONN)) {
    //         error("FAILED to listen for connections");
    //     }
    //     int session_fd = accept(sd, 0, 0);

    //     /* Check whether key_handle is correct */
    //     int n = read(session_fd, buf, BUFSIZE);
    //     if (n < 0) {
    //         error("ERROR reading from socket");
    //     }

    //     if (memcmp(key_handle, buf, QKD_KEY_HANDLE_SIZE) != 0) {
    //         /* key_handle is different from what was expected */
    //         n = write(session_fd, &(char){ 1 }, sizeof(char));
    //         if (n < 0) {
    //             error("ERROR writing to socket");
    //         }
    //         freeaddrinfo(res);
    //         close(sd);
    //         close(session_fd);
    //         return QKD_RC_GET_KEY_FAILED;
    //     }

    //     /* key_handle is correct */
    //     n = write(session_fd, &(char){ 0 }, sizeof(char));
    //     if (n < 0) {
    //         error("ERROR writing to socket");
    //     }

    //     /* Key generation */
    //     for (size_t i = 0; i < current_qos.requested_length; i++) {
    //         key_buffer[i] = (char) (rand() % 256);
    //     }

    //     n = write(session_fd, key_buffer, current_qos.requested_length);
    //     if (n < 0) {
    //         error("ERROR writing to socket");
    //     }
    //     close(session_fd);
    //     freeaddrinfo(res);
    //     close(sd);
    //     return QKD_RC_SUCCESS;
    // }
    return 0;
}

QKD_RC QKD_close(const QKD_key_handle_t *key_handle)
{
    /* TODO */
    return QKD_RC_SUCCESS;
}

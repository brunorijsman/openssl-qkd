/**
 * qkd_api_mock.c
 * 
 * A mock implementation of the ETSI QKD API. Instead of really using QKD to exchange a key, the
 * server picks a key at random and sends it to the client over an insecure classical channel. This
 * is for testing only.
 * 
 * (c) 2019 Bruno Rijsman, All Rights Reserved.
 * See LICENSE for licensing information.
 */

#include "qkd_api.h"
#include "qkd_debug.h"
#include <unistd.h>
#include <assert.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h> 
#include <arpa/inet.h>


/* TODO: Server can have more than one simultanious client */

/* TODO: Add support for non-blocking connect */

#define BUFSIZE 1024

#define QKD_PORT 8080
#define QKD_PORT_STR "8080"

static int listen_sock = -1;

QKD_qos_t current_qos;

typedef struct qkd_session_t {
    bool am_client;
    char *destination;
    QKD_key_handle_t key_handle;
    QKD_qos_t qos;
} QKD_SESSION;

/** 
 * Listen for incoming connections.
 *
 * Create a listen socket to receive incoming connections from the clients.
 * Returns listen socket on success, or -1 on failure.
 */
static int listen_for_incoming_connections()
{
    QKD_enter();

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

    /* Listen for incoming connections. */
    result = listen(sock, SOMAXCONN);
    QKD_fatal_with_errno_if(result != 0, "listen failed");

    QKD_exit();
    return sock;
}

/** 
 * Connect to server.
 *
 * Create a TCP connection to the server.
 * Returns connection socket on success, or -1 on failure.
 */
static int connect_to_server(char *destination)
{
    assert(destination != NULL);

    /* Resolve the destination to an address. */
    /* TODO: for now, ignode the destination and just hard-code localhost */
    const char *host_str = "localhost";
    const char *port_str = QKD_PORT_STR;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_ADDRCONFIG;
    struct addrinfo* res = 0;
    int result = getaddrinfo(host_str, port_str, &hints, &res);
    QKD_fatal_with_errno_if(result != 0, "getaddrinfo failed");

    /* Create the socket. */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    QKD_fatal_with_errno_if(sock == -1, "socket failed");

    /* Create the outgoing TCP connection. */
    result = connect(sock, res->ai_addr, res->ai_addrlen);
    QKD_fatal_with_errno_if(result != 0, "connect failed");
    return sock;
}

static int accept_connection_from_client()
{
    assert(listen_sock != -1);
    return accept(listen_sock, NULL, NULL);
}

static int send_key_handle(int sock, const QKD_key_handle_t *key_handle)
{
    assert(key_handle != NULL);
    int result = write(sock, key_handle->bytes, QKD_KEY_HANDLE_SIZE);
    QKD_fatal_with_errno_if(result != QKD_KEY_HANDLE_SIZE, "write failed");
    return QKD_RC_SUCCESS;
}

static int receive_key_handle(int sock, QKD_key_handle_t *key_handle)
{
    assert(key_handle != NULL);
    int result = read(sock, key_handle->bytes, QKD_KEY_HANDLE_SIZE);
    QKD_fatal_with_errno_if(result != QKD_KEY_HANDLE_SIZE, "read failed");
    return QKD_RC_SUCCESS;
}

/** 
 * Allocate and initialize a new QKD session.
 *
 * Returns pointer to new session, or NULL on failure.
 */
QKD_SESSION *qkd_session_new(bool am_client, char *destination, QKD_qos_t qos)
{
    QKD_enter();
    QKD_SESSION *session = malloc(sizeof(QKD_SESSION));
    QKD_fatal_if(session == NULL, "malloc failed");
    session->am_client = am_client;
    if (destination) {
        session->destination = strdup(destination);
    } else {
        session->destination = NULL;
    }
    QKD_key_handle_set_random(&session->key_handle);
    session->qos = qos;
    QKD_exit();
    return session;
}

void qkd_session_delete(QKD_SESSION *session)
{
    QKD_enter();
    assert(session != NULL);
    free(session);
    QKD_exit();
}

static QKD_SESSION *qkd_session = NULL;  /* TODO: For now this is the one and only session */

QKD_RC QKD_init(void)
{
    QKD_enter();
    listen_sock = listen_for_incoming_connections();
    QKD_fatal_if(listen_sock == -1, "listen_for_incoming_connections failed");
    QKD_exit();
    return QKD_RC_SUCCESS;
}

QKD_RC QKD_open(char *destination, QKD_qos_t qos, QKD_key_handle_t *key_handle)
{
    QKD_enter();
    assert(key_handle != NULL);

    /* Do we have a destination? In our implementation, the destination is optional, even though
     * the ETSI QKD API document doesn't say anything about the destination being optional. If the
     * destination is NULL it means we will accept incoming QKD connections from any client, and
     * if the destination is not NULL it means that we are a client and that destination contains
     * the address of the server. */
    bool am_client = (destination != NULL);
    if (am_client) {

        /* We are client, remote side is server */


        /* TODO: Get a key over the connection */

    } else {

        /* We are server, remote side is client */

        /* TODO: For now (maybe forever) we don't support predefined key handles on the server.
         * Hence we insist that the provded key handle is a null key handle (which is not the same * thing as a null pointer.) */
        bool is_null_handle = QKD_key_handle_is_null(key_handle);
        QKD_fatal_if(!is_null_handle, "Key handle must be null");

    }

    /* Create a new QKD session */
    /* TODO: for now we only allow one concurrent session. */
    assert(qkd_session == NULL);
    qkd_session = qkd_session_new(am_client, destination, qos);

    /* Return the key handle for the session */
    *key_handle = qkd_session->key_handle;
    QKD_exit();
    return QKD_RC_SUCCESS;
}

QKD_RC QKD_connect_nonblock(const QKD_key_handle_t *key_handle)
{
    QKD_enter();
    /* TODO: Not yet implemented. */
    QKD_exit();
    return QKD_RC_NOT_SUPPORTED;
}

QKD_RC QKD_connect_blocking(const QKD_key_handle_t *key_handle, uint32_t timeout)
{
    QKD_enter();

    /* TODO: For now there is only one concurrent session. */
    assert(qkd_session != NULL);
    if (qkd_session->am_client) {

        /* Client */

        /* Initiate a TCP connection to the server. */
        QKD_report("Initiate TCP connection to server");
        assert(qkd_session->destination != NULL);
        int connection_sock = connect_to_server(qkd_session->destination);
        QKD_fatal_if(connection_sock == -1, "connect_to_server failed");
        QKD_report("TCP connected to server");

        /* Send our (the client's) key handle to the server. */
        QKD_RC qkd_result = send_key_handle(connection_sock, key_handle);
        QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "send_key_handle failed");
        QKD_report("Sent key handle to server");

        /* TODO: store sock */

    } else {

        /* Server */

        /* Accept an incoming TCP connection from the client. */
        QKD_report("Accept an incoming TCP connection from the client");
        int connection_sock = accept_connection_from_client();
        QKD_fatal_with_errno_if(connection_sock == -1, "accept failed");
        QKD_report("TCP connected to client");

        /* TODO: store sock */

        /* Receive the client's key handle. */
        QKD_key_handle_t client_key_handle;
        QKD_RC qkd_result = receive_key_handle(connection_sock, &client_key_handle);
        QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "receive_key_handle failed");
        QKD_report("Received key handle from client");

        /* The client's key handle must be the same as ours. */
        bool same = QKD_key_handle_compare(&client_key_handle, key_handle) == 0;
        QKD_fatal_if(!same, "Client's key handle is different from server's key handle");
        QKD_report("Client's key handle is same as server's key handle");
    }

    QKD_exit();
    return QKD_RC_SUCCESS;
}

QKD_RC QKD_get_key(const QKD_key_handle_t *key_handle, char* key_buffer) {
    QKD_enter();
    /* TODO: Implement this; fixed shared secret for now */
    assert(key_buffer != NULL);
    for (size_t i = 0; i < current_qos.requested_length; i++) {
        key_buffer[i] = i;
    }
    QKD_exit();
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

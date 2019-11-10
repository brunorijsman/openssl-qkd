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
#include <assert.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h> 
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

/* TODO: Server can have more than one simultanious client */
/* TODO: Add support for non-blocking connect */

#define QKD_PORT 8080
#define QKD_PORT_STR "8080"

static int listen_sock = -1;

QKD_qos_t current_qos;

typedef struct qkd_session_t {
    bool am_client;
    char *destination;
    QKD_key_handle_t key_handle;
    QKD_qos_t qos;
    int connection_sock;
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
    int bytes_written = write(sock, key_handle->bytes, QKD_KEY_HANDLE_SIZE);
    QKD_fatal_with_errno_if(bytes_written != QKD_KEY_HANDLE_SIZE, "write failed");
    return QKD_RC_SUCCESS;
}

static int receive_key_handle(int sock, QKD_key_handle_t *key_handle)
{
    assert(key_handle != NULL);
    int bytes_read = read(sock, key_handle->bytes, QKD_KEY_HANDLE_SIZE);
    QKD_fatal_with_errno_if(bytes_read != QKD_KEY_HANDLE_SIZE, "read failed");
    return QKD_RC_SUCCESS;
}

static int send_shared_secret(int sock, const char *shared_secret, size_t shared_secret_size)
{
    int bytes_written = write(sock, shared_secret, shared_secret_size);
    QKD_fatal_with_errno_if(bytes_written != shared_secret_size, "write failed");
    return QKD_RC_SUCCESS;
}

/* Caller is responsible for allocating memory. */
static int receive_shared_secret(int sock, char *shared_secret, size_t shared_secret_size)
{
    assert(shared_secret != NULL);
    int bytes_read = read(sock, shared_secret, shared_secret_size);
    QKD_fatal_with_errno_if(bytes_read != shared_secret_size, "read failed");
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
    session->connection_sock = -1;
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

    /* TODO: For now (maybe forever) we don't support predefined key handles on the server.
     * Hence we insist that the provded key handle is a null key handle (which is not the same
     * thing as a null pointer.) */
    if (!am_client) {
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
    int connection_sock = -1;
    if (qkd_session->am_client) {

        /* Client */

        /* Initiate a TCP connection to the server. */
        QKD_report("Initiate TCP connection to server");
        assert(qkd_session->destination != NULL);
        connection_sock = connect_to_server(qkd_session->destination);
        QKD_fatal_if(connection_sock == -1, "connect_to_server failed");
        QKD_report("TCP connected to server");

        /* Send our (the client's) key handle to the server. */
        QKD_RC qkd_result = send_key_handle(connection_sock, key_handle);
        QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "send_key_handle failed");
        QKD_report("Sent key handle to server");

    } else {

        /* Server */

        /* Accept an incoming TCP connection from the client. */
        QKD_report("Accept an incoming TCP connection from the client");
        connection_sock = accept_connection_from_client();
        QKD_fatal_with_errno_if(connection_sock == -1, "accept failed");
        QKD_report("TCP connected to client");

        /* Receive the client's key handle. */
        QKD_key_handle_t client_key_handle;
        QKD_RC qkd_result = receive_key_handle(connection_sock, &client_key_handle);
        QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "receive_key_handle failed");
        QKD_report("Received key handle from client");

        /* The client's key handle must be the same as ours. This is just a sanity check does
         * not provide any level of security since the key handle was sent in the clear, namely
         * in the public key of the Diffie-Hellman exchange. (Anyway this mock implementation is
         * not intended to be secure in the first place.) */ 
        bool same = QKD_key_handle_compare(&client_key_handle, key_handle) == 0;
        QKD_fatal_if(!same, "Client's key handle is different from server's key handle");
        QKD_report("Client's key handle is same as server's key handle");
    }

    /* Store connection socket in QKD session */
    qkd_session->connection_sock = connection_sock;

    QKD_exit();
    return QKD_RC_SUCCESS;
}

QKD_RC QKD_get_key(const QKD_key_handle_t *key_handle, char* shared_secret)
{
    QKD_enter();

    /* TODO: For now there is only one concurrent session. */
    assert(qkd_session != NULL);
    assert(qkd_session->connection_sock != -1);    /* TODO: Keep track of connected in session. */

    int shared_secret_size = qkd_session->qos.requested_length;
    QKD_report("Shared secret size is %d", shared_secret_size);

    if (qkd_session->am_client) {

        /* Client */

        /* TODO: Explain WHY the client has to choose it */

        /* Choose a random shared secret. */
        assert(shared_secret != NULL);
        srand(time(NULL));
        for (int i = 0; i < shared_secret_size; ++i) {
            shared_secret[i] = rand();
        }
        QKD_report("Shared secret = %s", QKD_shared_secret_str(shared_secret, shared_secret_size));

        /* Send the shared secret to the server. */
        QKD_RC qkd_result = send_shared_secret(qkd_session->connection_sock, shared_secret,
                                               shared_secret_size);
        QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "send_shared_secret failed");
        QKD_report("Sent shared secret to server");


    } else {

        /* Server */

        /* Receive the shared secret chosen by the client. */
        QKD_report("Waiting for shared_secret from client");
        QKD_RC qkd_result = receive_shared_secret(qkd_session->connection_sock, shared_secret,
                                                  shared_secret_size);
        QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "receive_shared_secret failed");
        QKD_report("Received shared secret from client");
        QKD_report("Shared secret = %s", QKD_shared_secret_str(shared_secret, shared_secret_size));

    }

    QKD_exit();
    return QKD_RC_SUCCESS;
}

QKD_RC QKD_PLACEHOLDER_FOR_OLD_CODE(const QKD_key_handle_t *key_handle, char* key_buffer) 
{
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

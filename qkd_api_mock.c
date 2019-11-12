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
#include <string.h> 
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

/* TODO: Server can have more than one simultanious client */
/* TODO: Add support for non-blocking connect */

/**
 * TCP port number used for the "mock" replacement of the QKD protocol.
 */
#define QKD_PORT 8999
#define QKD_PORT_STR "8999"

static int listen_sock = -1;

QKD_qos_t current_qos;

typedef struct qkd_session_t {
    bool am_client;
    char *destination;
    QKD_key_handle_t key_handle;
    QKD_qos_t qos;
    int connection_sock;
} QKD_SESSION;

static QKD_SESSION *qkd_session = NULL;  /* TODO: For now this is the one and only session */

/** 
 * Listen for incoming connections.
 *
 * Create a listen socket to receive incoming connections from the clients.
 * 
 * Returns listen socket on success, or -1 on failure.
 */
static int listen_for_incoming_connections()
{
    QKD_enter();

    /* Create the socket. */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        QKD_error_with_errno("socket failed");
        QKD_return_error("%d", -1);
    }

    /* The client and server may run on the same host. In that case we want to allow both of them
    to create a listening socket for the same port. */
    int on = 1;
    int result = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char*)&on, sizeof(on));
    if (result != 0) {
        QKD_error_with_errno("setsockopt SO_REUSEPORT failed");
        QKD_return_error("%d", -1);
    }

    /* We want to be able to bind again while a previous socket for the same port is still in the
    lingering state. */
    result = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
    if (result != 0) {
        QKD_error_with_errno("setsockopt SO_REUSEADDR failed");
        QKD_return_error("%d", -1);
    }

    /* Bind the socket to the QKD port and the wildcard address. */
    struct sockaddr_in listen_address; 
    bzero(&listen_address, sizeof(listen_address));
    listen_address.sin_family = AF_INET; 
    listen_address.sin_addr.s_addr = htonl(INADDR_ANY); 
    listen_address.sin_port = htons(QKD_PORT);
    result = bind(sock, (const struct sockaddr *) &listen_address, sizeof(listen_address));
    if (result != 0) {
        QKD_error_with_errno("bind failed");
        QKD_return_error("%d", -1);
    }

    /* Listen for incoming connections. */
    result = listen(sock, SOMAXCONN);
    if (result != 0) {
        QKD_error_with_errno("listen failed");
        QKD_return_error("%d", -1);
    }

    QKD_return_success("%d", sock);
}

/** 
 * Connect to server.
 *
 * Create a TCP connection to the server.
 * 
 * Returns connection socket on success, or -1 on failure.
 */
static int connect_to_server(char *destination)
{
    QKD_enter();
    assert(destination != NULL);

    /* Resolve the destination to an address. */
    /* TODO: for now, ignore the destination and just hard-code localhost */
    const char *host_str = "localhost";
    const char *port_str = QKD_PORT_STR;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_ADDRCONFIG;
    struct addrinfo *res = NULL;
    int result = getaddrinfo(host_str, port_str, &hints, &res);
    if (result != 0) {
        QKD_error_with_errno("getaddrinfo failed");
        QKD_return_error("%d", -1);
    }

    /* Create the socket. */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        QKD_error_with_errno("socket failed");
        freeaddrinfo(res);
        QKD_return_error("%d", -1);
    }

    /* Connect the TCP connection. */
    result = connect(sock, res->ai_addr, res->ai_addrlen);
    if (result != 0) {
        QKD_error_with_errno("connect failed");
        freeaddrinfo(res);
        close(sock);
        QKD_return_error("%d", -1);
    }

    freeaddrinfo(res);
    QKD_return_success("%d", sock);
}

/**
 * Accept an incoming TCP connection from the client (blocking).
 * 
 * Returns the connection socket on success, -1 on failure.
 */
static int accept_connection_from_client()
{
    QKD_enter();
    assert(listen_sock != -1);

    int sock = accept(listen_sock, NULL, NULL);
    if (sock == -1) {
        QKD_error_with_errno("accept failed");
        QKD_return_error("%d", -1);
    }

    QKD_return_success("%d", sock);
}

/**
 * Send a key handle over a TCP connection (blocking).
 * 
 * Returns QKD_result_t.
 */
static QKD_result_t send_key_handle(int sock, const QKD_key_handle_t *key_handle)
{
    QKD_enter();
    assert(key_handle != NULL);

    int bytes_written = write(sock, key_handle->bytes, QKD_KEY_HANDLE_SIZE);
    if (bytes_written != QKD_KEY_HANDLE_SIZE) {
        QKD_error_with_errno("write failed");
        QKD_return_error_qkd(QKD_RESULT_SEND_FAILED);

    }

    QKD_return_success_qkd();
}

/**
 * Receive a key handle over a TCP connection (blocking).
 * 
 * Returns QKD_result_t.
 */
static QKD_result_t receive_key_handle(int sock, QKD_key_handle_t *key_handle)
{
    QKD_enter();
    assert(key_handle != NULL);

    int bytes_read = read(sock, key_handle->bytes, QKD_KEY_HANDLE_SIZE);
    if (bytes_read != QKD_KEY_HANDLE_SIZE) {
        QKD_error_with_errno("read failed");
        QKD_return_error_qkd(QKD_RESULT_RECEIVE_FAILED);
    }

    QKD_return_success_qkd();
}

/**
 * Send a shared secret over a TCP connection (blocking).
 * 
 * The mock implementation of the API does not provide any security at all because the shared secret
 * is sent in the clear.
 * 
 * Returns QKD_result_t.
 */
static QKD_result_t send_shared_secret(int sock, const char *shared_secret,
                                       size_t shared_secret_size)
{
    QKD_enter();
    assert(shared_secret != NULL);

    int bytes_written = write(sock, shared_secret, shared_secret_size);
    if (bytes_written != shared_secret_size) {
        QKD_error_with_errno("write failed");
        QKD_return_error_qkd(QKD_RESULT_SEND_FAILED);

    }

    QKD_return_success_qkd();
}

/**
 * Receive a shared secret over a TCP connection (blocking).
 * 
 * The caller is responsible for allocating the memory pointed to by shared secret (which implies
 * that the caller must now a-priori how large the shared secret will be.)
 * 
 * Returns QKD_result_t.
 */
static QKD_result_t receive_shared_secret(int sock, char *shared_secret, size_t shared_secret_size)
{
    QKD_enter();
    assert(shared_secret != NULL);

    int bytes_read = read(sock, shared_secret, shared_secret_size);
    if (bytes_read != shared_secret_size) {
        QKD_error_with_errno("read failed");
        QKD_return_error_qkd(QKD_RESULT_RECEIVE_FAILED);
    }

    QKD_return_success_qkd();
}

/** 
 * Allocate and initialize a new QKD session.
 *
 * Destination is a string containing the host name or IP address (as a string) of the remote side
 * of the QKD connection. For servers, destination may be NULL, which means that the server is
 * willing to accept an incoming QKD connection from any client.
 * 
 * Returns pointer to new session, or NULL on failure.
 */
QKD_SESSION *qkd_session_new(bool am_client, char *destination, QKD_qos_t qos)
{
    QKD_enter();

    /* Allocate the session. */
    QKD_SESSION *session = malloc(sizeof(QKD_SESSION));
    if (session == NULL) {
        QKD_error("malloc failed");
        QKD_return_error("%p", NULL);
    }

    /* Initialize the session. */
    session->am_client = am_client;
    if (destination) {
        session->destination = strdup(destination);
        if (session->destination == NULL) {
            QKD_error("malloc failed");
            free(session);
            QKD_return_error("%p", NULL);
        }
    } else {
        session->destination = NULL;
    }
    QKD_key_handle_set_random(&session->key_handle);
    session->qos = qos;
    session->connection_sock = -1;

    QKD_return_success("%p", session);
}

/**
 * Delete a QKD session.
 */
void qkd_session_delete(QKD_SESSION *session)
{
    QKD_enter();
    assert(session != NULL);
    free(session);
    QKD_return_success_void();
}

/**
 * Initialize the API.
 * 
 * Returns QKD_result_t.
 */
QKD_result_t QKD_init(bool am_server)
{
    QKD_enter();
    if (am_server) {
        listen_sock = listen_for_incoming_connections();
        if (-1 == listen_sock) {
            QKD_error_with_errno("listen failed");
            QKD_return_error_qkd(QKD_RESULT_CONNECTION_FAILED);
        }
    }
    QKD_return_success_qkd();
}

/**
 * Mock implementation of QKD_open, which is defined in the ETSI QKD API specification as follows:
 * "Receive an association (key_handle) to a set of future keys at both ends of the QKD link through
 * this distributed Key Management Layer and establish a set of parameters that define the expected
 * levels of key service. This function shall return immediately and not block."
 * 
 * On the server side, the provided key handle may contain the QKD_key_handle_null value (which is
 * different from the key_handle pointer being NULL), in which case this function will allocate a
 * new handle and return it in the key_handle paramter.
 * 
 * Returns QKD_result_t.
 */
QKD_result_t QKD_open(char *destination, QKD_qos_t qos, QKD_key_handle_t *key_handle)
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
        assert(QKD_key_handle_is_null(key_handle));
    }

    /* Create a new QKD session */
    /* TODO: for now we only allow one concurrent session. */
    assert(qkd_session == NULL);
    qkd_session = qkd_session_new(am_client, destination, qos);
    if (qkd_session == NULL) {
        QKD_return_error_qkd(QKD_RESULT_OUT_OF_MEMORY);
    }

    /* Return the key handle for the session (in the key_handle parameter) */
    *key_handle = qkd_session->key_handle;
    QKD_return_success_qkd();
}

/**
 * Mock implementation of QKD_connect_nonblock, which is defined in the ETSI QKD API specification
 * as follows: "Verifies that the QKD link is available and the key_handle association is
 * synchronized at both ends of the link. This function shall not block and returns immediately
 * indicating that both sides of the link have rendezvoused or an error has occured."
 * 
 * Returns QKD_result_t.
 */
QKD_result_t QKD_connect_nonblock(const QKD_key_handle_t *key_handle)
{
    QKD_enter();
    /* TODO: Not yet implemented. */
    QKD_return_error_qkd(QKD_RESULT_NOT_SUPPORTED);
}

/**
 * Mock implementation of QKD_connect_blocking, which is defined in the ETSI QKD API specification
 * as follows: "Verifies that the QKD link is available and the key_handle association is
 * synchronized at both ends of the link. This function shall block until both sides of the link
 * have rendezvoused, an error is detected, or the specified TIMEOUT delay has been exceeded."
 * 
 * Returns QKD_result_t.
 */
QKD_result_t QKD_connect_blocking(const QKD_key_handle_t *key_handle, uint32_t timeout)
{
    QKD_enter();
    /* TODO: Implement the timeout */

    /* TODO: For now there is only one concurrent session. */
    assert(qkd_session != NULL);
    int connection_sock = -1;
    if (qkd_session->am_client) {

        /* Client */

        /* Initiate a TCP connection to the server. */
        QKD_debug("Initiate TCP connection to server");
        assert(qkd_session->destination != NULL);
        connection_sock = connect_to_server(qkd_session->destination);
        if (-1 == connection_sock) {
            QKD_error("connect_to_server failed");
            QKD_return_error_qkd(QKD_RESULT_CONNECTION_FAILED);
        }
        QKD_debug("TCP connected to server");

        /* Send our (the client's) key handle to the server. */
        QKD_result_t qkd_result = send_key_handle(connection_sock, key_handle);
        if (QKD_RESULT_SUCCESS != qkd_result) {
            QKD_error("send_key_handle failed");
            QKD_return_error_qkd(qkd_result);
        }
        QKD_debug("Sent key handle to server");

    } else {

        /* Server */

        /* Accept an incoming TCP connection from the client. */
        QKD_debug("Accept an incoming TCP connection from the client");
        connection_sock = accept_connection_from_client();
        if (-1 == connection_sock) {
            QKD_error("accept_connection_from_client failed");
            QKD_return_error_qkd(QKD_RESULT_CONNECTION_FAILED);
        }
        QKD_debug("TCP connected to client");

        /* Receive the client's key handle. */
        QKD_key_handle_t client_key_handle;
        QKD_result_t qkd_result = receive_key_handle(connection_sock, &client_key_handle);
        if (QKD_RESULT_SUCCESS != qkd_result) {
            QKD_error("receive_key_handle failed");
            QKD_return_error_qkd(qkd_result);
        }
        QKD_debug("Received key handle from client");

        /* The client's key handle must be the same as ours. This is just a sanity check does
         * not provide any level of security since the key handle was sent in the clear, namely
         * in the public key of the Diffie-Hellman exchange. (Anyway this mock implementation is
         * not intended to be secure in the first place.) */ 
        bool same = QKD_key_handle_compare(&client_key_handle, key_handle) == 0;
        assert(same);
        QKD_debug("Client's key handle is same as server's key handle");
    }

    /* Store connection socket in QKD session */
    qkd_session->connection_sock = connection_sock;

    QKD_return_success_qkd();
}

/**
 * Mock implementation of QKD_get_key, which is defined in the ETSI QKD API specification as
 * follows: "Obtain the required amount of key material requested for this key_handle. Each call
 * shall return the fixed amount of requested key or an error message indicating why it failed.
 * This function may be called as often as desired, but the key manager only needs to respond at the
 * bit rate requested through the QOS parameters, or at the best rate the system can manage. The key
 * manager is responsible for reserving and synchronizing the keys at the two ends of the QKD link
 * through communication with its peer. This function may be blocking (wait for the key or an error)
 * or non-blocking and always return with the status parameter indicating success or failure,
 * depending on the request made via the QKD_OPEN function. The TIMEOUT value for this function is
 * specified in the QKD_OPEN() function."
 * 
 * The key (i.e. the shared secret) is returned in the shared_secret parameter. The caller is
 * responsible for allocating memory for shared secret.
 * 
 * Returns QKD_result_t.
 */
QKD_result_t QKD_get_key(const QKD_key_handle_t *key_handle, char* shared_secret)
{
    QKD_enter();

    /* TODO: For now there is only one concurrent session. */
    assert(qkd_session != NULL);
    assert(qkd_session->connection_sock != -1);    /* TODO: Keep track of connected in session. */

    int shared_secret_size = qkd_session->qos.requested_length;
    QKD_debug("Shared secret size is %d", shared_secret_size);

    if (qkd_session->am_client) {

        /* Client */

        /* There is a subtle hack here. It is necessary that the client chooses the shared secret
         * and sends it to the server. If we do it the other way around, i.e. if the server chooses
         * the shared secret and send it to the client, we will het a deadlock. This is because in
         * the OpenSSL implementation of the client side is as follows
         * (1) The client receives a Server Hello Message, which contains the Diffie-Hellman
         *     parameters and public key from the server.
         * (2) The client chooses its own Diffie-Hellman private key, using the received
         *     Diffie-Hellman parameters.
         * (3) The client calls generate_key to compute the shared secret based on (a) the received
         *     Diffie-Hellman paramters, (b) the received server's public key, and (c) it's own,
         *     i.e. the client's private key.
         * (4) Then, and only then, does the client send the Client Key Exchange message to the
         *     server, which contains the client's public key.
         * (5) The server can only call generate_key after it has received the Client Key Exchange
         *     message.
         * Now, if the server chooses the key, then the client will do a blocking receive in step
         * (3) to receive the shared secret from the server, but that message will never come
         * because it can only be sent in step (5).
         * This implementation is a hack because it makes assumptions about in which order
         * QKD_get_key will called on the server and the client. */

        /* Choose a random shared secret. */
        assert(shared_secret != NULL);
        srand(time(NULL));
        for (int i = 0; i < shared_secret_size; ++i) {
            shared_secret[i] = rand();
        }
        QKD_debug("Shared secret = %s", QKD_shared_secret_str(shared_secret, shared_secret_size));

        /* Send the shared secret to the server. */
        QKD_result_t qkd_result = send_shared_secret(qkd_session->connection_sock, shared_secret,
                                               shared_secret_size);
        if (QKD_RESULT_SUCCESS != qkd_result) {
            QKD_error("send_shared_secret failed: %s", QKD_result_str(qkd_result));
            QKD_return_error_qkd(qkd_result);
        }
        QKD_debug("Sent shared secret to server");

    } else {

        /* Server */

        /* Receive the shared secret chosen by the client. */
        QKD_debug("Waiting for shared_secret from client");
        QKD_result_t qkd_result = receive_shared_secret(qkd_session->connection_sock, shared_secret,
                                                  shared_secret_size);
        if (QKD_RESULT_SUCCESS != qkd_result) {
            QKD_error("receive_shared_secret failed: %s", QKD_result_str(qkd_result));
            QKD_return_error_qkd(qkd_result);
        }
        QKD_debug("Received shared secret from client");
        QKD_debug("Shared secret = %s", QKD_shared_secret_str(shared_secret, shared_secret_size));

    }

    QKD_return_success_qkd();
}

/**
 * Mock implementation of QKD_close, which is defined in the ETSI QKD API specification as follows:
 * "This terminates the association established for this key_handle and no further keys will be
 * allocated for this key_handle. Due to timing differences at the other end of the link, the peer
 * operation will happen at some other time and any unused keys shall be held until that occurs and
 * then discarded."
 * 
 * Returns QKD_result_t.
 */
QKD_result_t QKD_close(const QKD_key_handle_t *key_handle)
{
    QKD_enter();
    assert(key_handle);

    /* TODO: For now there is only one concurrent session. */
    assert(qkd_session != NULL);
    assert(qkd_session->connection_sock != -1);
    close(qkd_session->connection_sock);
    qkd_session->connection_sock = -1;
    free(qkd_session);
    qkd_session = NULL;

    QKD_return_success_qkd();
}

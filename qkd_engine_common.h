/**
 * qkd_engine_common.h
 * 
 * Code that is common to both OpenSSL engines: the server engine (qkd_server.c) and the client
 * engine (qkd_client.c).
 * 
 * (c) 2019 Bruno Rijsman, All Rights Reserved.
 * See LICENSE for licensing information.
 */

#ifndef ETSI_KQD_COMMON_H
#define ETSI_KQD_COMMON_H

#include "qkd_api.h"
#include <openssl/dh.h>
#include <openssl/engine.h>

/* TODO: Follow QKD_ naming convention */

int shared_secret_nr_bytes(DH *dh);

int QKD_engine_bind(ENGINE *engine, const char *engine_id, const char *engine_name,
                    int (*generate_key) (DH *),
                    int (*compute_key) (unsigned char *key, const BIGNUM *pub_key, DH *dh),
                    ENGINE_GEN_INT_FUNC_PTR engine_init);

int QKD_bignum_to_key_handle(const BIGNUM *bn, QKD_key_handle_t *key_handle);

int QKD_key_handle_to_bignum(const QKD_key_handle_t *key_handle, BIGNUM *bn);

char *QKD_shared_secret_str(unsigned char *shared_secret, size_t shared_secret_size);

/* We can control a "fixed" key (instead of an actual QKD-negotiated key) to allow end-to-end
testing before the interaction with the QKD-API has actually been implemented. */
extern bool QKD_return_fixed_key_for_testing;
extern const unsigned long QKD_fixed_public_key;
extern const unsigned long QKD_fixed_private_key;

/* When we run on SimulaQron, we do certain things differently than "in real life" (see below) */
extern bool QKD_running_on_simulaqron;

#endif
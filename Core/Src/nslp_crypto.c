/*
 * nslp_crypto.c
 *
 *  Created on: Aug 12, 2025
 *      Author: leonu
 */

#include "nslp_crypto.h"
#include "mbedtls/gcm.h"
#include <string.h>
#include <stdlib.h>

// Serialize DataPacket into a buffer:
// [ type (2 bytes) | size (1 byte) | payload (size bytes) ]
static size_t serialize_packet(const DataPacket *pkt, uint8_t *buffer) {
    buffer[0] = (pkt->type >> 8) & 0xFF;
    buffer[1] = pkt->type & 0xFF;
    buffer[2] = pkt->size;
    memcpy(&buffer[3], pkt->payload, pkt->size);
    return 3 + pkt->size;
}

// Deserialize DataPacket from a buffer, payload_buffer must be allocated to pkt->size
static int deserialize_packet(const uint8_t *buffer, size_t buffer_len, DataPacket *pkt, uint8_t *payload_buffer) {
    if (buffer_len < 3) return -1;
    pkt->type = ((uint16_t)buffer[0] << 8) | buffer[1];
    pkt->size = buffer[2];
    if (buffer_len < 3 + pkt->size) return -2;
    memcpy(payload_buffer, &buffer[3], pkt->size);
    pkt->payload = payload_buffer;
    return 0;
}

int nslp_encrypt_packet(const DataPacket *pkt,
                        const uint8_t key[NSLP_CRYPTO_KEY_LEN],
                        const uint8_t iv[NSLP_CRYPTO_IV_LEN],
                        uint8_t *output,
                        size_t *output_len)
{
    int ret;
    uint8_t plaintext[256]; // max size (adjust if needed)
    size_t plaintext_len = serialize_packet(pkt, plaintext);
    uint8_t tag[NSLP_CRYPTO_TAG_LEN];
    mbedtls_gcm_context gcm;

    mbedtls_gcm_init(&gcm);
    ret = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, NSLP_CRYPTO_KEY_LEN * 8);
    if (ret != 0) goto cleanup;

    ret = mbedtls_gcm_crypt_and_tag(&gcm,
            MBEDTLS_GCM_ENCRYPT,
            plaintext_len,
            iv,
            NSLP_CRYPTO_IV_LEN,
            NULL, 0, // additional data (AAD) can be added here
            plaintext,
            output,
            NSLP_CRYPTO_TAG_LEN,
            tag);
    if (ret != 0) goto cleanup;

    // Append tag after ciphertext
    memcpy(output + plaintext_len, tag, NSLP_CRYPTO_TAG_LEN);
    *output_len = plaintext_len + NSLP_CRYPTO_TAG_LEN;

cleanup:
    mbedtls_gcm_free(&gcm);
    return ret;
}

int nslp_decrypt_packet(const uint8_t *input,
                        size_t input_len,
                        const uint8_t key[NSLP_CRYPTO_KEY_LEN],
                        const uint8_t iv[NSLP_CRYPTO_IV_LEN],
                        DataPacket *pkt,
                        uint8_t *payload_buffer)
{
    if (input_len < NSLP_CRYPTO_TAG_LEN + 3) return -1; // too small

    int ret;
    size_t ciphertext_len = input_len - NSLP_CRYPTO_TAG_LEN;
    const uint8_t *ciphertext = input;
    const uint8_t *tag = input + ciphertext_len;

    uint8_t plaintext[256]; // max packet size, adjust if needed

    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);

    ret = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, NSLP_CRYPTO_KEY_LEN * 8);
    if (ret != 0) goto cleanup;

    ret = mbedtls_gcm_auth_decrypt(&gcm,
            ciphertext_len,
            iv,
            NSLP_CRYPTO_IV_LEN,
            NULL, 0,
            tag,
            NSLP_CRYPTO_TAG_LEN,
            ciphertext,
            plaintext);
    if (ret != 0) goto cleanup;

    ret = deserialize_packet(plaintext, ciphertext_len, pkt, payload_buffer);

cleanup:
    mbedtls_gcm_free(&gcm);
    return ret;
}




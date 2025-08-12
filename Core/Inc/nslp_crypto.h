/*
 * nslp_crypto.h
 *
 *  Created on: Aug 12, 2025
 *      Author: leonu
 */

#ifndef INC_NSLP_CRYPTO_H_
#define INC_NSLP_CRYPTO_H_


#include <stdint.h>
#include "DataPacket.h"  // Your packet struct header

#define NSLP_CRYPTO_TAG_LEN 16
#define NSLP_CRYPTO_IV_LEN 12
#define NSLP_CRYPTO_KEY_LEN 32  // AES-256

// Encrypt a DataPacket into output buffer.
// output must be at least (sizeof packet + tag)
// Returns 0 on success, or negative on error.
int nslp_encrypt_packet(const DataPacket *pkt,
                        const uint8_t key[NSLP_CRYPTO_KEY_LEN],
                        const uint8_t iv[NSLP_CRYPTO_IV_LEN],
                        uint8_t *output,
                        size_t *output_len);

// Decrypt an encrypted buffer into a DataPacket.
// Assumes packet->payload points to an allocated buffer of size pkt->size
// Returns 0 on success, negative on error.
int nslp_decrypt_packet(const uint8_t *input,
                        size_t input_len,
                        const uint8_t key[NSLP_CRYPTO_KEY_LEN],
                        const uint8_t iv[NSLP_CRYPTO_IV_LEN],
                        DataPacket *pkt,
                        uint8_t *payload_buffer);




#endif /* INC_NSLP_CRYPTO_H_ */

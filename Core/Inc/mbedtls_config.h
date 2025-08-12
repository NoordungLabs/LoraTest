// Core/Inc/mbedtls_config.h

#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#define MBEDTLS_AES_C
#define MBEDTLS_GCM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_CTR_DRBG_C

// We'll provide our own entropy source
#define MBEDTLS_NO_PLATFORM_ENTROPY

// Reduce memory usage if desired
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS

#endif // MBEDTLS_CONFIG_H


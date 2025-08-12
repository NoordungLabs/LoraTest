#include "stm32g4xx_hal.h"
#include <string.h>

extern RNG_HandleTypeDef hrng;

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen);


int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen) {
    (void) data; // unused
    for (size_t i = 0; i < len; i += 4) {
        uint32_t rnd;
        if (HAL_RNG_GenerateRandomNumber(&hrng, &rnd) != HAL_OK) {
            return -1;
        }
        size_t copy_len = (len - i >= 4) ? 4 : (len - i);
        memcpy(output + i, &rnd, copy_len);
    }
    *olen = len;
    return 0;
}

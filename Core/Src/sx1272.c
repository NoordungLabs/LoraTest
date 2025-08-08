#include "sx1272.h"

// === Low-level SPI helpers ===
static void SX1272_WriteReg(uint8_t addr, uint8_t value)
{
    uint8_t buf[2] = { (uint8_t)(addr | 0x80), value };
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_SET);
}

static uint8_t SX1272_ReadReg(uint8_t addr)
{
    uint8_t tx = addr & 0x7F, rx = 0;
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, &rx, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_SET);
    return rx;
}

static void SX1272_WriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_RESET);
    uint8_t header = addr | 0x80;
    HAL_SPI_Transmit(&hspi1, &header, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, buffer, size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_SET);
}

static void SX1272_ReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_RESET);
    uint8_t header = addr & 0x7F;
    HAL_SPI_Transmit(&hspi1, &header, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, buffer, size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_SET);
}

// === High-level functions ===
void SX1272_Init(void)
{
    // Set NSS pin high
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_SET);

    // Put in LoRa + Sleep mode to configure
    SX1272_WriteReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
    HAL_Delay(10);

    // Set frequency: example 868 MHz
    uint64_t frf = ((uint64_t)868000000 << 19) / 32000000;
    SX1272_WriteReg(REG_FRF_MSB, (uint8_t)(frf >> 16));
    SX1272_WriteReg(REG_FRF_MID, (uint8_t)(frf >> 8));
    SX1272_WriteReg(REG_FRF_LSB, (uint8_t)(frf >> 0));

    // Set base addresses
    SX1272_WriteReg(REG_FIFO_TX_BASE, 0x00);
    SX1272_WriteReg(REG_FIFO_RX_BASE, 0x00);

    // Modem Config (BW125, CR4/5, Explicit header mode)
    SX1272_WriteReg(REG_MODEM_CONFIG1, 0x72);
    SX1272_WriteReg(REG_MODEM_CONFIG2, (0x07 << 4) | 0x04); // SF7, CRC on

    // Max payload length
    SX1272_WriteReg(REG_PAYLOAD_LENGTH, 0x40);

    // Set Standby mode
    SX1272_WriteReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);

    // Map DIO0 to TxDone / RxDone
    SX1272_WriteReg(REG_DIO_MAPPING1, 0x00);
}

void SX1272_Transmit(uint8_t *data, uint8_t len)
{
    // Go to standby
    SX1272_WriteReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);

    // Clear IRQs
    SX1272_WriteReg(REG_IRQ_FLAGS, 0xFF);

    // FIFO pointer to Tx base
    SX1272_WriteReg(REG_FIFO_ADDR_PTR, 0x00);

    // Write payload
    SX1272_WriteBuffer(REG_FIFO, data, len);

    // Set payload length
    SX1272_WriteReg(REG_PAYLOAD_LENGTH, len);

    // Go to TX mode
    SX1272_WriteReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);
}

void SX1272_Receive(void)
{
    // Clear IRQs
    SX1272_WriteReg(REG_IRQ_FLAGS, 0xFF);

    // FIFO pointer to Rx base
    SX1272_WriteReg(REG_FIFO_ADDR_PTR, 0x00);

    // Go to continuous RX mode
    SX1272_WriteReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
}

uint8_t SX1272_RxBuffer[256];
uint8_t SX1272_RxLength = 0;

void SX1272_HandleDIO0(void)
{
    uint8_t irqFlags = SX1272_ReadReg(REG_IRQ_FLAGS);

    if (irqFlags & IRQ_TX_DONE_MASK)
    {
        SX1272_WriteReg(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
        // TX done event
    }
    if (irqFlags & IRQ_RX_DONE_MASK)
    {
        if (irqFlags & IRQ_CRC_ERROR_MASK)
        {
            SX1272_WriteReg(REG_IRQ_FLAGS, IRQ_CRC_ERROR_MASK);
        }
        else
        {
            uint8_t currentAddr = SX1272_ReadReg(REG_FIFO_RX_CURRENT);
            SX1272_RxLength = SX1272_ReadReg(REG_RX_NB_BYTES);

            SX1272_WriteReg(REG_FIFO_ADDR_PTR, currentAddr);
            SX1272_ReadBuffer(REG_FIFO, SX1272_RxBuffer, SX1272_RxLength);

            SX1272_WriteReg(REG_IRQ_FLAGS, IRQ_RX_DONE_MASK);

            // Optional: null-terminate if it's text
            if (SX1272_RxLength < sizeof(SX1272_RxBuffer))
                SX1272_RxBuffer[SX1272_RxLength] = '\0';
        }
    }
}

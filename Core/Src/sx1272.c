/*
 * sx1272.c
 *
 *  Created on: Aug 5, 2025
 *      Author: Leon
 */

#include "sx1272.h"
#include <string.h>


static SPI_HandleTypeDef *hspi;

// Private functions
static void SX1272_SetOpMode(uint8_t mode);

void SX1272_Init(SPI_HandleTypeDef *spi)
{
    hspi = spi;

    // Initialize GPIOs
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // NSS pin
    GPIO_InitStruct.Pin = SX1272_NSS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SX1272_NSS_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_SET);

    // Reset pin
    GPIO_InitStruct.Pin = SX1272_RESET_PIN;
    HAL_GPIO_Init(SX1272_RESET_PORT, &GPIO_InitStruct);

    // DIO0 pin (interrupt)
    GPIO_InitStruct.Pin = SX1272_DIO0_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SX1272_DIO0_PORT, &GPIO_InitStruct);

    // Reset the module
    SX1272_Reset();

    // Check version
    uint8_t version = SX1272_Read(REG_VERSION);
    if(version != 0x12) {
        while(1); // Error - device not found
    }

    // Put in LoRa mode
    SX1272_SetOpMode(MODE_LONG_RANGE_MODE | MODE_SLEEP);
    HAL_Delay(10);

    // Set frequency to 915 MHz
    SX1272_SetFrequency(915000000);

    // Set base addresses
    SX1272_Write(REG_FIFO_TX_BASE_ADDR, 0);
    SX1272_Write(REG_FIFO_RX_BASE_ADDR, 0);

    // Set LNA boost
    SX1272_SetLnaBoost(1);

    // Set TX power to 17 dBm
    SX1272_SetTxPower(17);

    // Set spreading factor to 12
    SX1272_SetSpreadingFactor(12);

    // Set bandwidth to 125 kHz
    SX1272_SetBandwidth(125000);

    // Set coding rate to 4/8
    SX1272_SetCodingRate4(8);

    // Set preamble length to 8
    SX1272_SetPreambleLength(8);

    // Enable CRC
    SX1272_EnableCRC();

    // Set sync word to 0x12
    SX1272_SetSyncWord(0x12);

    // Set DIO0 for TxDone and RxDone
    SX1272_Write(REG_DIO_MAPPING1, 0x00); // DIO0 => 00: TxDone, 01: RxDone

    // Put in standby mode
    SX1272_SetOpMode(MODE_LONG_RANGE_MODE | MODE_STDBY);
}

void SX1272_Reset(void)
{
    HAL_GPIO_WritePin(SX1272_RESET_PORT, SX1272_RESET_PIN, GPIO_PIN_RESET);
    HAL_Delay(1); // Wait at least 100us
    HAL_GPIO_WritePin(SX1272_RESET_PORT, SX1272_RESET_PIN, GPIO_PIN_SET);
    HAL_Delay(5); // Wait for reset to complete
}

void SX1272_Write(uint8_t addr, uint8_t data)
{
    uint8_t buf[2];
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_RESET);
    buf[0] = addr | 0x80;
    buf[1] = data;
    HAL_SPI_Transmit(hspi, buf, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_SET);
}

uint8_t SX1272_Read(uint8_t addr)
{
    uint8_t data;
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hspi, &addr, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(hspi, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_SET);
    return data;
}

void SX1272_WriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_RESET);
    addr |= 0x80;
    HAL_SPI_Transmit(hspi, &addr, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(hspi, buffer, size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_SET);
}

void SX1272_ReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hspi, &addr, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(hspi, buffer, size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SX1272_NSS_PORT, SX1272_NSS_PIN, GPIO_PIN_SET);
}

static void SX1272_SetOpMode(uint8_t mode)
{
    SX1272_Write(REG_OP_MODE, mode);
}

void SX1272_SetFrequency(uint32_t freq)
{
    uint64_t frf = ((uint64_t)freq << 19) / 32000000;
    SX1272_Write(REG_FRF_MSB, (frf >> 16) & 0xFF);
    SX1272_Write(REG_FRF_MID, (frf >> 8) & 0xFF);
    SX1272_Write(REG_FRF_LSB, frf & 0xFF);
}

void SX1272_SetTxPower(int8_t level)
{
    if(level > 17) {
        level = 17;
    } else if(level < 2) {
        level = 2;
    }

    SX1272_Write(REG_PA_CONFIG, PA_BOOST | (level - 2));
}

void SX1272_SetSpreadingFactor(uint8_t sf)
{
    if(sf < 6) {
        sf = 6;
    } else if(sf > 12) {
        sf = 12;
    }

    if(sf == 6) {
        SX1272_Write(REG_DETECTION_OPTIMIZE, 0xC5);
        SX1272_Write(REG_DETECTION_THRESHOLD, 0x0C);
    } else {
        SX1272_Write(REG_DETECTION_OPTIMIZE, 0xC3);
        SX1272_Write(REG_DETECTION_THRESHOLD, 0x0A);
    }

    uint8_t config2 = SX1272_Read(REG_MODEM_CONFIG2);
    config2 = (config2 & 0x0F) | ((sf << 4) & 0xF0);
    SX1272_Write(REG_MODEM_CONFIG2, config2);
}

void SX1272_SetBandwidth(uint32_t bw)
{
    uint8_t bw_val;

    if(bw <= 7800) {
        bw_val = 0;
    } else if(bw <= 10400) {
        bw_val = 1;
    } else if(bw <= 15600) {
        bw_val = 2;
    } else if(bw <= 20800) {
        bw_val = 3;
    } else if(bw <= 31250) {
        bw_val = 4;
    } else if(bw <= 41700) {
        bw_val = 5;
    } else if(bw <= 62500) {
        bw_val = 6;
    } else if(bw <= 125000) {
        bw_val = 7;
    } else if(bw <= 250000) {
        bw_val = 8;
    } else {
        bw_val = 9;
    }

    uint8_t config1 = SX1272_Read(REG_MODEM_CONFIG1);
    config1 = (config1 & 0x0F) | (bw_val << 4);
    SX1272_Write(REG_MODEM_CONFIG1, config1);
}

void SX1272_SetCodingRate4(uint8_t denominator)
{
    if(denominator < 5) {
        denominator = 5;
    } else if(denominator > 8) {
        denominator = 8;
    }

    uint8_t cr = denominator - 4;
    uint8_t config1 = SX1272_Read(REG_MODEM_CONFIG1);
    config1 = (config1 & 0xF1) | (cr << 1);
    SX1272_Write(REG_MODEM_CONFIG1, config1);
}

void SX1272_SetSyncWord(uint8_t sw)
{
    SX1272_Write(REG_SYNC_WORD, sw);
}

void SX1272_SetPreambleLength(uint16_t length)
{
    SX1272_Write(REG_PREAMBLE_MSB, (length >> 8) & 0xFF);
    SX1272_Write(REG_PREAMBLE_LSB, length & 0xFF);
}

void SX1272_EnableCRC(void)
{
    uint8_t config2 = SX1272_Read(REG_MODEM_CONFIG2);
    config2 |= 0x04;
    SX1272_Write(REG_MODEM_CONFIG2, config2);
}

void SX1272_SetLnaBoost(uint8_t boost)
{
    if(boost) {
        SX1272_Write(REG_LNA, SX1272_Read(REG_LNA) | 0x03);
    } else {
        SX1272_Write(REG_LNA, SX1272_Read(REG_LNA) & ~0x03);
    }
}

int16_t SX1272_GetPacketRssi(void)
{
    return (SX1272_Read(REG_PKT_RSSI_VALUE) - (164));
}

int8_t SX1272_GetPacketSnr(void)
{
    return ((int8_t)SX1272_Read(REG_PKT_SNR_VALUE)) * 0.25;
}

void SX1272_SendPacket(uint8_t *buffer, uint8_t size)
{
    // Set standby mode
    SX1272_SetOpMode(MODE_LONG_RANGE_MODE | MODE_STDBY);

    // Set FIFO address pointer
    SX1272_Write(REG_FIFO_ADDR_PTR, 0);

    // Write payload to FIFO
    SX1272_WriteBuffer(REG_FIFO, buffer, size);

    // Set payload length
    SX1272_Write(REG_PAYLOAD_LENGTH, size);

    // Set TX mode
    SX1272_SetOpMode(MODE_LONG_RANGE_MODE | MODE_TX);

    // Wait for TX done
}

uint8_t SX1272_ReceivePacket(uint8_t *buffer, uint8_t size) {
    uint8_t irqFlags = SX1272_Read(REG_IRQ_FLAGS);

    if(!(irqFlags & IRQ_RX_DONE_MASK)) {
        return 0;
    }

    // Clear IRQ flags (specific flags only)
    SX1272_Write(REG_IRQ_FLAGS, IRQ_RX_DONE_MASK | IRQ_PAYLOAD_CRC_ERROR_MASK);

    if(irqFlags & IRQ_PAYLOAD_CRC_ERROR_MASK) {
        return 0; // CRC error
    }

    uint8_t length = SX1272_Read(REG_RX_NB_BYTES);
    if(length == 0 || length > size) {
        SX1272_StartReceiving(); // Return to RX
        return 0;
    }

    uint8_t currentAddr = SX1272_Read(REG_FIFO_RX_CURRENT_ADDR);
    SX1272_Write(REG_FIFO_ADDR_PTR, currentAddr);
    SX1272_ReadBuffer(REG_FIFO, buffer, length);

    // Restart RX
    SX1272_StartReceiving();

    return length;
}

void SX1272_StartReceiving(void)
{
    // Set FIFO address pointer
    SX1272_Write(REG_FIFO_ADDR_PTR, 0);

    // Set RX mode
    SX1272_SetOpMode(MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
}

uint8_t SX1272_CheckReceived(void)
{
    return (SX1272_Read(REG_IRQ_FLAGS) & IRQ_RX_DONE_MASK) ? 1 : 0;
}



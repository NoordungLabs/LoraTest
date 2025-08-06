/*
 * sx1272.h
 *
 *  Created on: Aug 5, 2025
 *      Author: Leon
 */

#ifndef INC_SX1272_H_
#define INC_SX1272_H_

#include "stm32g4xx_hal.h"

// Pin definitions
#define SX1272_NSS_PIN     GPIO_PIN_4
#define SX1272_NSS_PORT    GPIOA
#define SX1272_RESET_PIN   GPIO_PIN_3
#define SX1272_RESET_PORT  GPIOA
#define SX1272_DIO0_PIN    GPIO_PIN_0
#define SX1272_DIO0_PORT   GPIOB

// LoRa Registers
#define REG_FIFO                 0x00
#define REG_OP_MODE              0x01
#define REG_FRF_MSB              0x06
#define REG_FRF_MID              0x07
#define REG_FRF_LSB              0x08
#define REG_PA_CONFIG            0x09
#define REG_LNA                  0x0C
#define REG_FIFO_ADDR_PTR        0x0D
#define REG_FIFO_TX_BASE_ADDR    0x0E
#define REG_FIFO_RX_BASE_ADDR    0x0F
#define REG_FIFO_RX_CURRENT_ADDR 0x10
#define REG_IRQ_FLAGS            0x12
#define REG_RX_NB_BYTES          0x13
#define REG_PKT_SNR_VALUE        0x19
#define REG_PKT_RSSI_VALUE       0x1A
#define REG_MODEM_CONFIG1        0x1D
#define REG_MODEM_CONFIG2        0x1E
#define REG_PREAMBLE_MSB         0x20
#define REG_PREAMBLE_LSB         0x21
#define REG_PAYLOAD_LENGTH       0x22
#define REG_MODEM_CONFIG3        0x26
#define REG_RSSI_WIDEBAND        0x2C
#define REG_DETECTION_OPTIMIZE   0x31
#define REG_DETECTION_THRESHOLD  0x37
#define REG_SYNC_WORD            0x39
#define REG_DIO_MAPPING1         0x40
#define REG_VERSION              0x42

// Modes
#define MODE_LONG_RANGE_MODE     0x80
#define MODE_SLEEP               0x00
#define MODE_STDBY               0x01
#define MODE_TX                  0x03
#define MODE_RX_CONTINUOUS       0x05
#define MODE_RX_SINGLE           0x06

// PA config
#define PA_BOOST                 0x80

// IRQ masks
#define IRQ_TX_DONE_MASK         0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK 0x20
#define IRQ_RX_DONE_MASK         0x40

// Functions
void SX1272_Init(SPI_HandleTypeDef *hspi);
void SX1272_Reset(void);
void SX1272_Write(uint8_t addr, uint8_t data);
uint8_t SX1272_Read(uint8_t addr);
void SX1272_WriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size);
void SX1272_ReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size);
void SX1272_SetFrequency(uint32_t freq);
void SX1272_SetTxPower(int8_t level);
void SX1272_SetSpreadingFactor(uint8_t sf);
void SX1272_SetBandwidth(uint32_t bw);
void SX1272_SetCodingRate4(uint8_t denominator);
void SX1272_SetSyncWord(uint8_t sw);
void SX1272_SetPreambleLength(uint16_t length);
void SX1272_EnableCRC(void);
void SX1272_SetLnaBoost(uint8_t boost);
int16_t SX1272_GetPacketRssi(void);
int8_t SX1272_GetPacketSnr(void);
void SX1272_SendPacket(uint8_t *buffer, uint8_t size);
uint8_t SX1272_ReceivePacket(uint8_t *buffer, uint8_t size);
void SX1272_StartReceiving(void);
uint8_t SX1272_CheckReceived(void);


#endif /* INC_SX1272_H_ */

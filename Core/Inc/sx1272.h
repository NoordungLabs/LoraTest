/*
 * sx1272.h
 *
 *  Created on: Aug 5, 2025
 *      Author: Leon
 */

#ifndef INC_SX1272_H_
#define INC_SX1272_H_

#include "stm32g4xx_hal.h"

// SX1272 Register Addresses
#define REG_FIFO            0x00
#define REG_OP_MODE         0x01
#define REG_FRF_MSB         0x06
#define REG_FRF_MID         0x07
#define REG_FRF_LSB         0x08
#define REG_PA_CONFIG       0x09
#define REG_FIFO_ADDR_PTR   0x0D
#define REG_FIFO_TX_BASE    0x0E
#define REG_FIFO_RX_BASE    0x0F
#define REG_FIFO_RX_CURRENT 0x10
#define REG_IRQ_FLAGS       0x12
#define REG_RX_NB_BYTES     0x13
#define REG_PKT_SNR_VALUE   0x19
#define REG_PKT_RSSI_VALUE  0x1A
#define REG_MODEM_CONFIG1   0x1D
#define REG_MODEM_CONFIG2   0x1E
#define REG_PAYLOAD_LENGTH  0x22
#define REG_MODEM_CONFIG3   0x26
#define REG_DIO_MAPPING1    0x40
#define REG_VERSION         0x42
#define REG_PA_DAC          0x4D

// SX1272 Modes
#define MODE_LONG_RANGE_MODE    0x80
#define MODE_SLEEP              0x00
#define MODE_STDBY              0x01
#define MODE_TX                 0x03
#define MODE_RX_CONTINUOUS      0x05
#define MODE_RX_SINGLE          0x06

// IRQ Masks
#define IRQ_TX_DONE_MASK        0x08
#define IRQ_RX_DONE_MASK        0x40
#define IRQ_CRC_ERROR_MASK      0x20

// NSS and DIO0 pin definitions
#define SX1272_NSS_PORT GPIOA
#define SX1272_NSS_PIN  GPIO_PIN_4
#define SX1272_DIO0_PORT GPIOB
#define SX1272_DIO0_PIN  GPIO_PIN_0

extern SPI_HandleTypeDef hspi1;

// Public API
void SX1272_Init(void);
void SX1272_Transmit(uint8_t *data, uint8_t len);
void SX1272_Receive(void);
void SX1272_HandleDIO0(void);





#endif /* INC_SX1272_H_ */

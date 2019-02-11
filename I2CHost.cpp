/*
 *   Copyright (C) 2019 by Andy Uribe CA6JAU
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Config.h"

#if defined(STM32_I2C_HOST)

#include "Globals.h"
#include "I2CHost.h"

extern "C" {
  void I2C2_EV_IRQHandler(void) {
    i2c.I2C_EVHandler();
  }

  void I2C2_ER_IRQHandler(void) {
    if (I2C_GetITStatus(I2C2, I2C_IT_AF)) {
      I2C_ClearITPendingBit(I2C2, I2C_IT_AF);
    }
  }
}

CI2CHost::CI2CHost()
{
}

void CI2CHost::Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  I2C_InitTypeDef  I2C_InitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  // Configure I2C GPIOs
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // Configure the I2C event interrupt
  NVIC_InitStructure.NVIC_IRQChannel                   = I2C2_EV_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 15;
  NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  // Configure the I2C error interrupt
  NVIC_InitStructure.NVIC_IRQChannel = I2C2_ER_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  // I2C configuration
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = I2C_ADDR << 1;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = I2C_CLK_FREQ;

  // Enable I2C
  I2C_Cmd(I2C2, ENABLE);
  // Apply I2C configuration
  I2C_Init(I2C2, &I2C_InitStructure);

  I2C_ITConfig(I2C2, I2C_IT_EVT, ENABLE);
  I2C_ITConfig(I2C2, I2C_IT_BUF, ENABLE);
  I2C_ITConfig(I2C2, I2C_IT_ERR, ENABLE);

  // Initialize the FIFOs
  txFIFO_head = 0U;
  txFIFO_tail = 0U;
  rxFIFO_head = 0U;
  rxFIFO_tail = 0U;
}

void CI2CHost::I2C_EVHandler(void) {
  uint32_t event = I2C_GetLastEvent(I2C2);

  switch (event) {
    case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED:
      break;
    case I2C_EVENT_SLAVE_BYTE_RECEIVED:
      if (rxFIFO_level() < I2C_RX_FIFO_SIZE) {
        rxFIFO[rxFIFO_head] = I2C_ReceiveData(I2C2);
        rxFIFO_head++;
        if (rxFIFO_head >= I2C_RX_FIFO_SIZE)
          rxFIFO_head = 0U;
      } else
        I2C_ReceiveData(I2C2);
      break;
    case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:
    case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:
      if (txFIFO_level() > 0) {
        I2C_SendData(I2C2, txFIFO[txFIFO_tail]);
        txFIFO_tail++;
        if (txFIFO_tail >= I2C_TX_FIFO_SIZE)
          txFIFO_tail = 0U;
      } else
        I2C_SendData(I2C2, 0U);
      break;
    case I2C_EVENT_SLAVE_STOP_DETECTED:
      I2C2_ClearFlag();
      break;
  }
}

void CI2CHost::I2C2_ClearFlag(void) {
  // Clear ADDR flag
  while((I2C2->SR1 & I2C_SR1_ADDR) == I2C_SR1_ADDR) {
    I2C2->SR1;
    I2C2->SR2;
  }

  // Clear STOPF flag
  while((I2C2->SR1 & I2C_SR1_STOPF) == I2C_SR1_STOPF) {
    I2C2->SR1;
    I2C2->CR1 |= 0x1;
  }
}

uint16_t CI2CHost::txFIFO_level(void)
{
  uint32_t tail = txFIFO_tail;
  uint32_t head = txFIFO_head;

  if (tail > head)
    return I2C_TX_FIFO_SIZE + head - tail;
  else
    return head - tail;
}

uint16_t CI2CHost::rxFIFO_level(void)
{
  uint32_t tail = rxFIFO_tail;
  uint32_t head = rxFIFO_head;

  if (tail > head)
    return I2C_RX_FIFO_SIZE + head - tail;
  else
    return head - tail;
}


uint8_t CI2CHost::txFIFO_put(uint8_t next)
{
  if (txFIFO_level() < I2C_TX_FIFO_SIZE) {
    txFIFO[txFIFO_head] = next;

    txFIFO_head++;
    if (txFIFO_head >= I2C_TX_FIFO_SIZE)
      txFIFO_head = 0U;
    return 1U;
  } else {
    return 0U;
  }
}

uint8_t CI2CHost::AvailI2C(void)
{
  if (rxFIFO_level() > 0U)
    return 1U;
  else
    return 0U;
}


uint8_t CI2CHost::ReadI2C(void)
{
  uint8_t data_c = rxFIFO[rxFIFO_tail];

  rxFIFO_tail++;
  if (rxFIFO_tail >= I2C_RX_FIFO_SIZE)
    rxFIFO_tail = 0U;

  return data_c;
}

void CI2CHost::WriteI2C(const uint8_t* data, uint16_t length)
{
  for (uint16_t i = 0U; i < length; i++)
    txFIFO_put(data[i]);
}

#endif
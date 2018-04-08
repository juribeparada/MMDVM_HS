/*
 *   Copyright (C) 2016 by Jim McLaughlin KI6ZUM
 *   Copyright (C) 2016,2017 by Andy Uribe CA6JAU
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

#if defined(STM32F10X_MD)

#include "Globals.h"
#include "SerialPort.h"

#if defined(STM32_USB_HOST)
#include <usb_serial.h>
#endif

/*
Pin definitions:

- Host communication:
USART1 - TXD PA9  - RXD PA10
or
USB VCOM

- Serial repeater
USART2 - TXD PA2  - RXD PA3 

*/

#define TX_SERIAL_FIFO_SIZE 256U
#define RX_SERIAL_FIFO_SIZE 256U

#if defined(STM32_USART1_HOST) && defined(STM32_USB_HOST)
#error "You have to select STM32_USART1_HOST or STM32_USB_HOST, but not both"
#endif

#if defined(STM32_USART1_HOST) || defined(SERIAL_REPEATER_USART1)

extern "C" {
  void USART1_IRQHandler();
}

/* ************* USART1 ***************** */

volatile uint8_t  TXSerialfifo1[TX_SERIAL_FIFO_SIZE];
volatile uint8_t  RXSerialfifo1[RX_SERIAL_FIFO_SIZE];
volatile uint16_t TXSerialfifohead1, TXSerialfifotail1;
volatile uint16_t RXSerialfifohead1, RXSerialfifotail1;

// Init queues
void TXSerialfifoinit1()
{
  TXSerialfifohead1 = 0U;
  TXSerialfifotail1 = 0U;
}

void RXSerialfifoinit1()
{
  RXSerialfifohead1 = 0U;
  RXSerialfifotail1 = 0U;
}

// How full is queue
// TODO decide if how full or how empty is preferred info to return
uint16_t TXSerialfifolevel1()
{
  uint32_t tail = TXSerialfifotail1;
  uint32_t head = TXSerialfifohead1;

  if (tail > head)
    return TX_SERIAL_FIFO_SIZE + head - tail;
  else
    return head - tail;
}

uint16_t RXSerialfifolevel1()
{
  uint32_t tail = RXSerialfifotail1;
  uint32_t head = RXSerialfifohead1;

  if (tail > head)
    return RX_SERIAL_FIFO_SIZE + head - tail;
  else
    return head - tail;
}

// Flushes the transmit shift register
// warning: this call is blocking
void TXSerialFlush1()
{
  // wait until the TXE shows the shift register is empty
  while (USART_GetITStatus(USART1, USART_FLAG_TXE))
    ;
}

uint8_t TXSerialfifoput1(uint8_t next)
{
  if (TXSerialfifolevel1() < TX_SERIAL_FIFO_SIZE) {
    TXSerialfifo1[TXSerialfifohead1] = next;

    TXSerialfifohead1++;
    if (TXSerialfifohead1 >= TX_SERIAL_FIFO_SIZE)
      TXSerialfifohead1 = 0U;

    // make sure transmit interrupts are enabled as long as there is data to send
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    return 1U;
  } else {
    return 0U; // signal an overflow occurred by returning a zero count
  }
}

void USART1_IRQHandler()
{
  uint8_t c;

  if (USART_GetITStatus(USART1, USART_IT_RXNE)) {
    c = (uint8_t) USART_ReceiveData(USART1);

    if (RXSerialfifolevel1() < RX_SERIAL_FIFO_SIZE) {
      RXSerialfifo1[RXSerialfifohead1] = c;

      RXSerialfifohead1++;
      if (RXSerialfifohead1 >= RX_SERIAL_FIFO_SIZE)
        RXSerialfifohead1 = 0U;
    } else {
      // TODO - do something if rx fifo is full?
    }

    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  }

  if (USART_GetITStatus(USART1, USART_IT_TXE)) {
    c = 0U;

    if (TXSerialfifohead1 != TXSerialfifotail1) { // if the fifo is not empty
      c = TXSerialfifo1[TXSerialfifotail1];

      TXSerialfifotail1++;
      if (TXSerialfifotail1 >= TX_SERIAL_FIFO_SIZE)
        TXSerialfifotail1 = 0U;

      USART_SendData(USART1, c);
    } else { // if there's no more data to transmit then turn off TX interrupts
      USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    }

    USART_ClearITPendingBit(USART1, USART_IT_TXE);
  }
}

void InitUSART1(int speed)
{
  // USART1 - TXD PA9  - RXD PA10
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  // USART IRQ init
  NVIC_InitStructure.NVIC_IRQChannel    = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 15;
  NVIC_Init(&NVIC_InitStructure);

  // Configure USART as alternate function
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;       //  Tx
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;       //  Rx
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // Configure USART baud rate
  USART_StructInit(&USART_InitStructure);
  USART_InitStructure.USART_BaudRate   = speed;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits   = USART_StopBits_1;
  USART_InitStructure.USART_Parity     = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  
  USART_Cmd(USART1, ENABLE);

  // initialize the fifos
  TXSerialfifoinit1();
  RXSerialfifoinit1();
}

uint8_t AvailUSART1()
{
  if (RXSerialfifolevel1() > 0U)
    return 1U;
  else
    return 0U;
}

uint8_t ReadUSART1()
{
  uint8_t data_c = RXSerialfifo1[RXSerialfifotail1];

  RXSerialfifotail1++;
  if (RXSerialfifotail1 >= RX_SERIAL_FIFO_SIZE)
    RXSerialfifotail1 = 0U;

  return data_c;
}

void WriteUSART1(const uint8_t* data, uint16_t length)
{
  for (uint16_t i = 0U; i < length; i++)
    TXSerialfifoput1(data[i]);
    
  USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

#endif

#if defined(SERIAL_REPEATER)

extern "C" {
  void USART2_IRQHandler();
}

/* ************* USART2 ***************** */

volatile uint8_t  TXSerialfifo2[TX_SERIAL_FIFO_SIZE];
volatile uint8_t  RXSerialfifo2[RX_SERIAL_FIFO_SIZE];
volatile uint16_t TXSerialfifohead2, TXSerialfifotail2;
volatile uint16_t RXSerialfifohead2, RXSerialfifotail2;

// Init queues
void TXSerialfifoinit2()
{
  TXSerialfifohead2 = 0U;
  TXSerialfifotail2 = 0U;
}

void RXSerialfifoinit2()
{
  RXSerialfifohead2 = 0U;
  RXSerialfifotail2 = 0U;
}

// How full is queue
// TODO decide if how full or how empty is preferred info to return
uint16_t TXSerialfifolevel2()
{
  uint32_t tail = TXSerialfifotail2;
  uint32_t head = TXSerialfifohead2;

  if (tail > head)
    return TX_SERIAL_FIFO_SIZE + head - tail;
  else
    return head - tail;
}

uint16_t RXSerialfifolevel2()
{
  uint32_t tail = RXSerialfifotail2;
  uint32_t head = RXSerialfifohead2;

  if (tail > head)
    return RX_SERIAL_FIFO_SIZE + head - tail;
  else
    return head - tail;
}

// Flushes the transmit shift register
// warning: this call is blocking
void TXSerialFlush2()
{
  // wait until the TXE shows the shift register is empty
  while (USART_GetITStatus(USART2, USART_FLAG_TXE))
    ;
}

uint8_t TXSerialfifoput2(uint8_t next)
{
  if (TXSerialfifolevel2() < TX_SERIAL_FIFO_SIZE) {
    TXSerialfifo2[TXSerialfifohead2] = next;

    TXSerialfifohead2++;
    if (TXSerialfifohead2 >= TX_SERIAL_FIFO_SIZE)
      TXSerialfifohead2 = 0U;

    // make sure transmit interrupts are enabled as long as there is data to send
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
    return 1U;
  } else {
    return 0U; // signal an overflow occurred by returning a zero count
  }
}

void USART2_IRQHandler()
{
  uint8_t c;
  
  if (USART_GetITStatus(USART2, USART_IT_RXNE)) {
    c = (uint8_t) USART_ReceiveData(USART2);

    if (RXSerialfifolevel2() < RX_SERIAL_FIFO_SIZE) {
      RXSerialfifo2[RXSerialfifohead2] = c;

      RXSerialfifohead2++;
      if (RXSerialfifohead2 >= RX_SERIAL_FIFO_SIZE)
        RXSerialfifohead2 = 0U;
    } else {
      // TODO - do something if rx fifo is full?
    }

    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
  }

  if (USART_GetITStatus(USART2, USART_IT_TXE)) {
    c = 0U;

    if (TXSerialfifohead2 != TXSerialfifotail2) { // if the fifo is not empty
      c = TXSerialfifo2[TXSerialfifotail2];

      TXSerialfifotail2++;
      if (TXSerialfifotail2 >= TX_SERIAL_FIFO_SIZE)
        TXSerialfifotail2 = 0U;

      USART_SendData(USART2, c);
    } else { // if there's no more data to transmit then turn off TX interrupts
      USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    }

    USART_ClearITPendingBit(USART2, USART_IT_TXE);
  }
}

void InitUSART2(int speed)
{
  
  // USART2 - TXD PA2  - RXD PA3
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

  // USART IRQ init
  NVIC_InitStructure.NVIC_IRQChannel    = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 15;
  NVIC_Init(&NVIC_InitStructure);

  // Configure USART as alternate function
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;       //  Tx
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;       //  Rx
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // Configure USART baud rate
  USART_StructInit(&USART_InitStructure);
  USART_InitStructure.USART_BaudRate   = speed;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits   = USART_StopBits_1;
  USART_InitStructure.USART_Parity     = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);

  USART_Cmd(USART2, ENABLE);

  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

  // initialize the fifos
  TXSerialfifoinit2();
  RXSerialfifoinit2();
}

uint8_t AvailUSART2()
{
  if (RXSerialfifolevel2() > 0U)
    return 1U;
  else
    return 0U;
}

uint8_t ReadUSART2()
{
  uint8_t data_c = RXSerialfifo2[RXSerialfifotail2];

  RXSerialfifotail2++;
  if (RXSerialfifotail2 >= RX_SERIAL_FIFO_SIZE)
    RXSerialfifotail2 = 0U;

  return data_c;
}

void WriteUSART2(const uint8_t* data, uint16_t length)
{
  for (uint16_t i = 0U; i < length; i++)
    TXSerialfifoput2(data[i]);
    
  USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

#endif

/////////////////////////////////////////////////////////////////

void CSerialPort::beginInt(uint8_t n, int speed)
{
  switch (n) {
    case 1U:
    #if defined(STM32_USART1_HOST)
      InitUSART1(speed);
    #elif defined(STM32_USB_HOST)
      usbserial.begin();
    #endif
      break;
    case 3U:
    #if defined(SERIAL_REPEATER)
      InitUSART2(speed);
    #elif defined(SERIAL_REPEATER_USART1)
      InitUSART1(speed);
    #endif
      break;
    default:
      break;
  }   
}

int CSerialPort::availableInt(uint8_t n)
{ 
  switch (n) {
    case 1U:
    #if defined(STM32_USART1_HOST)
      return AvailUSART1();
    #elif defined(STM32_USB_HOST)
      return usbserial.available();
    #endif
    case 3U: 
    #if defined(SERIAL_REPEATER)
      return AvailUSART2();
    #elif defined(SERIAL_REPEATER_USART1)
      return AvailUSART1();
    #endif
    default:
      return 0;
  }
}

uint8_t CSerialPort::readInt(uint8_t n)
{   
  switch (n) {
    case 1U:
    #if defined(STM32_USART1_HOST)
      return ReadUSART1();
    #elif defined(STM32_USB_HOST)
      return usbserial.read();
    #endif
    case 3U:
    #if defined(SERIAL_REPEATER)
      return ReadUSART2();
    #elif defined(SERIAL_REPEATER_USART1)
      return ReadUSART1();
    #endif
    default:
      return 0U;
  }
}

void CSerialPort::writeInt(uint8_t n, const uint8_t* data, uint16_t length, bool flush)
{
  switch (n) {
    case 1U:
    #if defined(STM32_USART1_HOST)
      WriteUSART1(data, length);
    if (flush)
      TXSerialFlush1();
    #elif defined(STM32_USB_HOST)
      usbserial.write(data, length);
      if (flush)
        usbserial.flush();
    #endif
      break;
    case 3U:
    #if defined(SERIAL_REPEATER)
      WriteUSART2(data, length);
      if (flush)
        TXSerialFlush2();
    #elif defined(SERIAL_REPEATER_USART1)
      WriteUSART1(data, length);
      if (flush)
        TXSerialFlush1();
    #endif
      break;
    default:
      break;
  }
}

#endif

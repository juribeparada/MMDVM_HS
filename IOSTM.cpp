/*
 *   Copyright (C) 2016 by Jim McLaughlin KI6ZUM
 *   Copyright (C) 2016,2017,2018 by Andy Uribe CA6JAU
 *   Copyright (C) 2017 by Danilo DB4PLE 
  
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

// Select initial release of ZUMspot RPi:
// #define PI_HAT_7021_REV_02

#if defined(STM32F10X_MD)

#include "Globals.h"
#include "IO.h"

#if defined(PI_HAT_7021_REV_02)

#define PIN_SCLK             GPIO_Pin_4
#define PORT_SCLK            GPIOB

#define PIN_SREAD            GPIO_Pin_5
#define PORT_SREAD           GPIOB

#define PIN_SDATA            GPIO_Pin_6
#define PORT_SDATA           GPIOB

#define PIN_SLE              GPIO_Pin_7
#define PORT_SLE             GPIOB

#define PIN_CE               GPIO_Pin_14
#define PORT_CE              GPIOC

#define PIN_RXD              GPIO_Pin_3
#define PORT_RXD             GPIOB

// TXD used in SPI Data mode of ADF7021
// TXD is TxRxCLK of ADF7021, standard TX/RX data interface
#define PIN_TXD              GPIO_Pin_15
#define PORT_TXD             GPIOA
#define PIN_TXD_INT          GPIO_PinSource15
#define PORT_TXD_INT         GPIO_PortSourceGPIOA

// CLKOUT used in SPI Data mode of ADF7021
#define PIN_CLKOUT           GPIO_Pin_14
#define PORT_CLKOUT          GPIOA
#define PIN_CLKOUT_INT       GPIO_PinSource14
#define PORT_CLKOUT_INT      GPIO_PortSourceGPIOA

#define PIN_LED              GPIO_Pin_13
#define PORT_LED             GPIOC

#define PIN_DEB              GPIO_Pin_11
#define PORT_DEB             GPIOA

#define PIN_DSTAR_LED        GPIO_Pin_14
#define PORT_DSTAR_LED       GPIOB

#define PIN_DMR_LED          GPIO_Pin_15
#define PORT_DMR_LED         GPIOB

#define PIN_YSF_LED          GPIO_Pin_13
#define PORT_YSF_LED         GPIOA

#define PIN_P25_LED          GPIO_Pin_12
#define PORT_P25_LED         GPIOA

#define PIN_NXDN_LED         GPIO_Pin_8
#define PORT_NXDN_LED        GPIOA

#define PIN_PTT_LED          GPIO_Pin_12
#define PORT_PTT_LED         GPIOB

#define PIN_COS_LED          GPIO_Pin_13
#define PORT_COS_LED         GPIOB

#elif defined(ZUMSPOT_ADF7021)

#define PIN_SCLK             GPIO_Pin_5
#define PORT_SCLK            GPIOB

#define PIN_SREAD            GPIO_Pin_6
#define PORT_SREAD           GPIOB

#define PIN_SDATA            GPIO_Pin_7
#define PORT_SDATA           GPIOB

#define PIN_SLE              GPIO_Pin_8
#define PORT_SLE             GPIOB

#define PIN_SLE2             GPIO_Pin_6
#define PORT_SLE2            GPIOA

#define PIN_CE               GPIO_Pin_14
#define PORT_CE              GPIOC

#define PIN_RXD              GPIO_Pin_4
#define PORT_RXD             GPIOB

// TXD used in SPI Data mode of ADF7021
// TXD is TxRxCLK of ADF7021, standard TX/RX data interface
#define PIN_TXD              GPIO_Pin_3
#define PORT_TXD             GPIOB
#define PIN_TXD_INT          GPIO_PinSource3
#define PORT_TXD_INT         GPIO_PortSourceGPIOB

// CLKOUT used in SPI Data mode of ADF7021
#define PIN_CLKOUT           GPIO_Pin_15
#define PORT_CLKOUT          GPIOA
#define PIN_CLKOUT_INT       GPIO_PinSource15
#define PORT_CLKOUT_INT      GPIO_PortSourceGPIOA

#define PIN_LED              GPIO_Pin_13
#define PORT_LED             GPIOC

#define PIN_DEB              GPIO_Pin_9
#define PORT_DEB             GPIOB

#define PIN_DSTAR_LED        GPIO_Pin_12
#define PORT_DSTAR_LED       GPIOB

#define PIN_DMR_LED          GPIO_Pin_13
#define PORT_DMR_LED         GPIOB

#define PIN_YSF_LED          GPIO_Pin_1
#define PORT_YSF_LED         GPIOB

#define PIN_P25_LED          GPIO_Pin_0
#define PORT_P25_LED         GPIOB

#define PIN_NXDN_LED         GPIO_Pin_8
#define PORT_NXDN_LED        GPIOA

#define PIN_PTT_LED          GPIO_Pin_14
#define PORT_PTT_LED         GPIOB

#define PIN_COS_LED          GPIO_Pin_15
#define PORT_COS_LED         GPIOB

#elif defined(LIBRE_KIT_ADF7021) || defined(MMDVM_HS_HAT_REV12) || defined(MMDVM_HS_DUAL_HAT_REV10) || defined(NANO_HOTSPOT) || defined(NANO_DV_REV10)

#define PIN_SCLK             GPIO_Pin_5
#define PORT_SCLK            GPIOB

#define PIN_SREAD            GPIO_Pin_7
#define PORT_SREAD           GPIOB

#define PIN_SDATA            GPIO_Pin_6
#define PORT_SDATA           GPIOB

#define PIN_SLE              GPIO_Pin_8
#define PORT_SLE             GPIOB

#define PIN_SLE2             GPIO_Pin_6
#define PORT_SLE2            GPIOA

#define PIN_CE               GPIO_Pin_14
#define PORT_CE              GPIOC

#define PIN_RXD              GPIO_Pin_4
#define PORT_RXD             GPIOB

#define PIN_RXD2             GPIO_Pin_4
#define PORT_RXD2            GPIOA

// TXD used in SPI Data mode of ADF7021
// TXD is TxRxCLK of ADF7021, standard TX/RX data interface
#define PIN_TXD              GPIO_Pin_3
#define PORT_TXD             GPIOB
#define PIN_TXD_INT          GPIO_PinSource3
#define PORT_TXD_INT         GPIO_PortSourceGPIOB

// TXD2 is TxRxCLK of the second ADF7021, standard TX/RX data interface
#define PIN_TXD2             GPIO_Pin_5
#define PORT_TXD2            GPIOA
#define PIN_TXD2_INT         GPIO_PinSource5
#define PORT_TXD2_INT        GPIO_PortSourceGPIOA

// CLKOUT used in SPI Data mode of ADF7021
#define PIN_CLKOUT           GPIO_Pin_15
#define PORT_CLKOUT          GPIOA
#define PIN_CLKOUT_INT       GPIO_PinSource15
#define PORT_CLKOUT_INT      GPIO_PortSourceGPIOA

#define PIN_LED              GPIO_Pin_13
#define PORT_LED             GPIOC

#define PIN_DEB              GPIO_Pin_9
#define PORT_DEB             GPIOB

#define PIN_DSTAR_LED        GPIO_Pin_12
#define PORT_DSTAR_LED       GPIOB

#define PIN_DMR_LED          GPIO_Pin_13
#define PORT_DMR_LED         GPIOB

#define PIN_YSF_LED          GPIO_Pin_1
#define PORT_YSF_LED         GPIOB

#define PIN_P25_LED          GPIO_Pin_0
#define PORT_P25_LED         GPIOB

#define PIN_NXDN_LED         GPIO_Pin_8
#define PORT_NXDN_LED        GPIOA

#define PIN_PTT_LED          GPIO_Pin_14
#define PORT_PTT_LED         GPIOB

#define PIN_COS_LED          GPIO_Pin_15
#define PORT_COS_LED         GPIOB

#else
#error "Either PI_HAT_7021_REV_02, ZUMSPOT_ADF7021, LIBRE_KIT_ADF7021, MMDVM_HS_HAT_REV12, MMDVM_HS_DUAL_HAT_REV10, NANO_HOTSPOT or NANO_DV_REV10 need to be defined"
#endif

extern "C" {
#if defined(PI_HAT_7021_REV_02)

#if defined(BIDIR_DATA_PIN)
  void EXTI15_10_IRQHandler(void) {
    if(EXTI_GetITStatus(EXTI_Line15)!=RESET) {
      io.interrupt();
    EXTI_ClearITPendingBit(EXTI_Line15);
    }
  }
#else
  void EXTI15_10_IRQHandler(void) {
    if(EXTI_GetITStatus(EXTI_Line14)!=RESET) {
      io.interrupt();
    EXTI_ClearITPendingBit(EXTI_Line14);
    }
  }
#endif

#elif defined(ZUMSPOT_ADF7021) || defined(LIBRE_KIT_ADF7021) || defined(MMDVM_HS_HAT_REV12) || defined(MMDVM_HS_DUAL_HAT_REV10) || defined(NANO_HOTSPOT) || defined(NANO_DV_REV10)

#if defined(BIDIR_DATA_PIN)
  void EXTI3_IRQHandler(void) {
    if(EXTI_GetITStatus(EXTI_Line3)!=RESET) {
      io.interrupt();
    EXTI_ClearITPendingBit(EXTI_Line3);
    }
  }
#else
  void EXTI15_10_IRQHandler(void) {
    if(EXTI_GetITStatus(EXTI_Line15)!=RESET) {
      io.interrupt();
    EXTI_ClearITPendingBit(EXTI_Line15);
    }
  }
#endif

#if defined(DUPLEX)
  void EXTI9_5_IRQHandler(void) {
    if(EXTI_GetITStatus(EXTI_Line5)!=RESET) {
      io.interrupt2();
    EXTI_ClearITPendingBit(EXTI_Line5);
    }
  }
#endif

#endif
}

void CIO::Init()
{ 
  // USB Conf IO:
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
  
#if defined(PI_HAT_7021_REV_02)
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
#elif defined(ZUMSPOT_ADF7021) || defined(LIBRE_KIT_ADF7021) || defined(MMDVM_HS_HAT_REV12) || defined(MMDVM_HS_DUAL_HAT_REV10) || defined(NANO_HOTSPOT) || defined(NANO_DV_REV10)
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
#endif

  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

  EXTI_InitTypeDef EXTI_InitStructure;
#if defined(DUPLEX)
  EXTI_InitTypeDef EXTI_InitStructure2;
#endif

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_StructInit(&GPIO_InitStruct);

  // Pin PA12 = LOW, USB Reset in generic boards
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_12;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_RESET);

  volatile unsigned int delay;
  for(delay = 0;delay<512;delay++);

  GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_12;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Pin SCLK
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_SCLK;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_SCLK, &GPIO_InitStruct);

  // Pin SDATA
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_SDATA;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_SDATA, &GPIO_InitStruct);
  
  // Pin SREAD
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_SREAD;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(PORT_SREAD, &GPIO_InitStruct);

  // Pin SLE
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_SLE;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_SLE, &GPIO_InitStruct);
  
#if defined(DUPLEX)
  // Pin SLE2
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_SLE2;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_SLE2, &GPIO_InitStruct);
  
  // Pin RXD2
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_RXD2;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(PORT_RXD2, &GPIO_InitStruct);
#endif

  // Pin CE
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_CE;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_CE, &GPIO_InitStruct);

  // Pin RXD
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_RXD;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(PORT_RXD, &GPIO_InitStruct);

  // Pin TXD
  // TXD is TxRxCLK of ADF7021, standard TX/RX data interface
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_TXD;
#if defined(BIDIR_DATA_PIN)
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
#else
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
#endif
  GPIO_Init(PORT_TXD, &GPIO_InitStruct);
#if defined(DUPLEX)
  GPIO_Init(PORT_TXD2, &GPIO_InitStruct);
#endif

  // Pin TXRX_CLK
#if !defined(BIDIR_DATA_PIN)
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_CLKOUT;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(PORT_CLKOUT, &GPIO_InitStruct);
#endif
 
  // Pin LED
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_LED;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_LED, &GPIO_InitStruct);

  // Pin Debug
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_DEB;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_DEB, &GPIO_InitStruct);

  // D-Star LED 
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_DSTAR_LED;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_DSTAR_LED, &GPIO_InitStruct);

  // DMR LED 
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_DMR_LED;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_DMR_LED, &GPIO_InitStruct);

  // YSF LED 
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_YSF_LED;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_YSF_LED, &GPIO_InitStruct);

  // P25 LED 
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_P25_LED;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_P25_LED, &GPIO_InitStruct);

  // NXDN LED 
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_NXDN_LED;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_NXDN_LED, &GPIO_InitStruct);

  // PTT LED
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_PTT_LED;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_PTT_LED, &GPIO_InitStruct);

  // COS LED 
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_COS_LED;
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(PORT_COS_LED, &GPIO_InitStruct);

#if defined(PI_HAT_7021_REV_02)

#if defined(BIDIR_DATA_PIN)
  // Connect EXTI15 Line
  GPIO_EXTILineConfig(PORT_TXD_INT, PIN_TXD_INT);
  // Configure EXTI15 line
  EXTI_InitStructure.EXTI_Line = EXTI_Line15;
#else
  // Connect EXTI14 Line
  GPIO_EXTILineConfig(PORT_CLKOUT_INT, PIN_CLKOUT_INT);
  // Configure EXTI14 line
  EXTI_InitStructure.EXTI_Line = EXTI_Line14;
#endif

#elif defined(ZUMSPOT_ADF7021) || defined(LIBRE_KIT_ADF7021) || defined(MMDVM_HS_HAT_REV12) || defined(MMDVM_HS_DUAL_HAT_REV10) || defined(NANO_HOTSPOT) || defined(NANO_DV_REV10)

#if defined(BIDIR_DATA_PIN)
  // Connect EXTI3 Line
  GPIO_EXTILineConfig(PORT_TXD_INT, PIN_TXD_INT);
  // Configure EXTI3 line
  EXTI_InitStructure.EXTI_Line = EXTI_Line3;
#else
  // Connect EXTI15 Line
  GPIO_EXTILineConfig(PORT_CLKOUT_INT, PIN_CLKOUT_INT);
  // Configure EXTI15 line
  EXTI_InitStructure.EXTI_Line = EXTI_Line15;
#endif

#if defined(DUPLEX)
  // Connect EXTI5 Line
  GPIO_EXTILineConfig(PORT_TXD2_INT, PIN_TXD2_INT);
  // Configure EXT5 line
  EXTI_InitStructure2.EXTI_Line = EXTI_Line5;
#endif

#endif

  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
#if defined(DUPLEX)
  EXTI_InitStructure2.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure2.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure2.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure2);
#endif
}

void CIO::startInt()
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
#if defined(DUPLEX)
  NVIC_InitTypeDef NVIC_InitStructure2;
#endif

#if defined(PI_HAT_7021_REV_02)

  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;

#elif defined(ZUMSPOT_ADF7021) || defined(LIBRE_KIT_ADF7021) || defined(MMDVM_HS_HAT_REV12) || defined(MMDVM_HS_DUAL_HAT_REV10) || defined(NANO_HOTSPOT) || defined(NANO_DV_REV10)

#if defined(BIDIR_DATA_PIN)
  // Enable and set EXTI3 Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
#else
  // Enable and set EXTI15 Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
#endif

#if defined(DUPLEX)
  NVIC_InitStructure2.NVIC_IRQChannel = EXTI9_5_IRQn;
#endif

#endif

  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 15;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
#if defined(DUPLEX)
  NVIC_InitStructure2.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure2.NVIC_IRQChannelSubPriority = 15;
  NVIC_InitStructure2.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure2);
#endif
}

#if defined(BIDIR_DATA_PIN)
// RXD pin is bidirectional in standard interfaces
void CIO::Data_dir_out(bool dir) 
{
  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin   = PIN_RXD;
  
  if(dir)
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
  else
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  
  GPIO_Init(PORT_RXD, &GPIO_InitStruct);
}
#endif

void CIO::SCLK_pin(bool on)
{
  GPIO_WriteBit(PORT_SCLK, PIN_SCLK, on ? Bit_SET : Bit_RESET);
}

void CIO::SDATA_pin(bool on)
{
  GPIO_WriteBit(PORT_SDATA, PIN_SDATA, on ? Bit_SET : Bit_RESET);
}

bool CIO::SREAD_pin()
{
  return GPIO_ReadInputDataBit(PORT_SREAD, PIN_SREAD) == Bit_SET;
}

void CIO::SLE_pin(bool on)
{
  GPIO_WriteBit(PORT_SLE, PIN_SLE, on ? Bit_SET : Bit_RESET);
}

#if defined(DUPLEX)
void CIO::SLE2_pin(bool on)
{
  GPIO_WriteBit(PORT_SLE2, PIN_SLE2, on ? Bit_SET : Bit_RESET);
}

bool CIO::RXD2_pin()
{
  return GPIO_ReadInputDataBit(PORT_RXD2, PIN_RXD2) == Bit_SET;
}
#endif

void CIO::CE_pin(bool on)
{
  GPIO_WriteBit(PORT_CE, PIN_CE, on ? Bit_SET : Bit_RESET);
}

bool CIO::RXD_pin()
{
  return GPIO_ReadInputDataBit(PORT_RXD, PIN_RXD) == Bit_SET;
}

bool CIO::CLK_pin()
{
#if defined(BIDIR_DATA_PIN)
  return GPIO_ReadInputDataBit(PORT_TXD, PIN_TXD) == Bit_SET;
#else
  return GPIO_ReadInputDataBit(PORT_CLKOUT, PIN_CLKOUT) == Bit_SET;
#endif
}

#if defined(BIDIR_DATA_PIN)
void CIO::RXD_pin_write(bool on)
{
  GPIO_WriteBit(PORT_RXD, PIN_RXD, on ? Bit_SET : Bit_RESET);
}
#endif

void CIO::TXD_pin(bool on)
{
  GPIO_WriteBit(PORT_TXD, PIN_TXD, on ? Bit_SET : Bit_RESET);
}

void CIO::LED_pin(bool on)
{
  GPIO_WriteBit(PORT_LED, PIN_LED, on ? Bit_SET : Bit_RESET);
}

void CIO::DEB_pin(bool on)
{
  GPIO_WriteBit(PORT_DEB, PIN_DEB, on ? Bit_SET : Bit_RESET);
}

void CIO::DSTAR_pin(bool on)
{
  GPIO_WriteBit(PORT_DSTAR_LED, PIN_DSTAR_LED, on ? Bit_SET : Bit_RESET);
}

void CIO::DMR_pin(bool on)
{
  GPIO_WriteBit(PORT_DMR_LED, PIN_DMR_LED, on ? Bit_SET : Bit_RESET);
}

void CIO::YSF_pin(bool on)
{
  GPIO_WriteBit(PORT_YSF_LED, PIN_YSF_LED, on ? Bit_SET : Bit_RESET);
}

void CIO::P25_pin(bool on)
{
  GPIO_WriteBit(PORT_P25_LED, PIN_P25_LED, on ? Bit_SET : Bit_RESET);
}

void CIO::NXDN_pin(bool on)
{
#if defined(USE_ALTERNATE_NXDN_LEDS)
  GPIO_WriteBit(PORT_YSF_LED, PIN_YSF_LED, on ? Bit_SET : Bit_RESET);
  GPIO_WriteBit(PORT_P25_LED, PIN_P25_LED, on ? Bit_SET : Bit_RESET);
#else
  GPIO_WriteBit(PORT_NXDN_LED, PIN_NXDN_LED, on ? Bit_SET : Bit_RESET);
#endif
}

void CIO::POCSAG_pin(bool on)
{
#if defined(USE_ALTERNATE_POCSAG_LEDS)
  GPIO_WriteBit(PORT_DSTAR_LED, PIN_DSTAR_LED, on ? Bit_SET : Bit_RESET);
  GPIO_WriteBit(PORT_DMR_LED, PIN_DMR_LED, on ? Bit_SET : Bit_RESET);
#endif
  // TODO: add a separate LED pin for POCSAG mode
}

void CIO::PTT_pin(bool on)
{
  GPIO_WriteBit(PORT_PTT_LED, PIN_PTT_LED, on ? Bit_SET : Bit_RESET);
}

void CIO::COS_pin(bool on)
{
  GPIO_WriteBit(PORT_COS_LED, PIN_COS_LED, on ? Bit_SET : Bit_RESET);
}


/**
 * Function delay_us() from stm32duino project
 *
 * @brief Delay the given number of microseconds.
 *
 * @param us Number of microseconds to delay.
 */
static inline void delay_us(uint32_t us) {
    us *= 12;

    /* fudge for function call overhead  */
    us--;
    asm volatile("   mov r0, %[us]          \n\t"
                 "1: subs r0, #1            \n\t"
                 "   bhi 1b                 \n\t"
                 :
                 : [us] "r" (us)
                 : "r0");
}

void CIO::delay_IFcal() {
  delay_us(10000);
}

void CIO::delay_reset() {
  delay_us(300);
}

void CIO::delay_us(uint32_t us) {
  ::delay_us(us);
}

static inline void delay_ns() {

    asm volatile("nop          \n\t"
                 "nop          \n\t"
                 "nop          \n\t"
                 );
}


void CIO::dlybit(void)
{
  delay_ns();
}
#endif

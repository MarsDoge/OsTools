/*
## @file
#
#  Copyright (c) 2018 Loongson Technology Corporation Limited (www.loongson.cn).
#  All intellectual property rights(Copyright, Patent and Trademark) reserved.
#
#  Any violations of copyright or other intellectual property rights of the Loongson Technology
#  Corporation Limited will be held accountable in accordance with the law,
#  if you (or any of your subsidiaries, corporate affiliates or agents) initiate
#  directly or indirectly any Intellectual Property Assertion or Intellectual Property Litigation:
#  (i) against Loongson Technology Corporation Limited or any of its subsidiaries or corporate affiliates,
#  (ii) against any party if such Intellectual Property Assertion or Intellectual Property Litigation arises
#  in whole or in part from any software, technology, product or service of Loongson Technology Corporation
#  Limited or any of its subsidiaries or corporate affiliates, or (iii) against any party relating to the Software.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
#  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION).
#
#
##
*/
#ifndef  _I2C_BUS_H_
#define  _I2C_BUS_H_

#include "def.h"

/*
 * I2C Reg Description
 * */
#define PRER_LO_REG 0x0
#define PRER_HI_REG 0x1
#define CTR_REG     0x2
#define TXR_REG     0x3
#define RXR_REG     0x3
#define CR_REG      0x4
#define SR_REG      0x4
#define SLV_CTRL    0X7

/*
 * I2C Reg Command
 * */
#define CR_START  0x80
#define CR_STOP   0x40
#define CR_READ   0x20
#define CR_WRITE  0x10
#define CR_ACK    0x8
#define CR_IACK   0x1

#define SR_NOACK  0x80
#define SR_BUSY   0x40
#define SR_AL     0x20
#define SR_TIP    0x2
#define SR_IF     0x1

#define MST_EN    0x00000020
#define SLV_EN    0x00000080

int
I2cCtlRead (
        UINTN                                CtlAddr,
        UINTN                                DeviceAddr,
        UINTN                                DataAddr,
        UINTN                                DataByte,
        VOID                             *Buffer
);
int
I2cCtlWrite (
        UINTN                                CtlAddr,
        UINTN                                DeviceAddr,
        UINTN                                DataAddr,
        UINTN                                DataByte,
        VOID                             *Buffer
);
int
I2cInitSetFreq (
        UINTN                                CtlAddr,
        UINTN                                CtlClock,
        UINTN                                Frequency, //Bus clk frequency
        UINT8                                Algorithm
);

#endif

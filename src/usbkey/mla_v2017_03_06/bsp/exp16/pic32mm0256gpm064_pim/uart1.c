/**
  UART1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    uart1.c

  @Summary
    This is the generated driver implementation file for the UART1 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides implementations for driver APIs for UART1.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - pic24-dspic-pic32mm : 1.53.0.1
        Device            :  PIC32MM0256GPM064
    The generated drivers are tested against the following:
        Compiler          :  XC32 v1.44
        MPLAB             :  MPLAB X v4.05
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#include <stdint.h>
#include <xc.h>
#include "uart1.h"

#define RXQSIZ 128
static volatile int rxi;
static int rxo;
static uint8_t rxq[RXQSIZ];

void __attribute__ ((vector(_UART1_RX_VECTOR), interrupt(IPL5SRS))) _UART1_RX_ISR()
{
    while (U1STA & _U1STA_URXDA_MASK) {
        rxq[rxi] = U1RXREG;
        rxi = (rxi + 1) % RXQSIZ;
    }

    if (U1STA & _U1STA_OERR_MASK) {
        U1STACLR = _U1STA_OERR_MASK;
    }

    IFS1CLR = _IFS1_U1RXIF_MASK;
}

int UART1_DataReady(void)
{
    return !(rxo == rxi);
}

void UART1_Initialize(void)
{
    // Interrupt controller setup for RX interrupt: priority 5, sub-priority 0
    IPC13bits.U1RXIP = 5;
    IPC13bits.U1RXIS = 0;

    // UART1 on level 5 uses SRS 1, everything else on SRS 0
    PRISS = _PRISS_PRI5SS_MASK;

    // Set UART1 RX (RA6) and TX (RC12) to digital I/O
    ANSELACLR = 1 << 6;
    ANSELCCLR = 1 << 12;

    // Also weak pull-up on UART1 RX
    CNPUASET = (1 << 6);

    // Set TX direction to output, value to 1
    LATCSET = 1 << 12;
    TRISCCLR = 1 << 12;

    // Set the UART1 module to the options selected in the user interface.

    // STSEL 1; PDSEL 8N; RTSMD disabled; OVFDIS disabled; ACTIVE disabled; RXINV disabled; WAKE disabled; BRGH disabled; IREN disabled; ON enabled; SLPEN disabled; SIDL disabled; ABAUD disabled; LPBACK disabled; UEN TX_RX; CLKSEL PBCLK; 
    U1MODE = 0; // disabling UART ON bit 

    // UTXISEL TX_ONE_CHAR; UTXINV disabled; ADDR 0; MASK 0; URXEN disabled; OERR disabled; URXISEL RX_ONE_CHAR; UTXBRK disabled; UTXEN disabled; ADDEN disabled; 
    U1STA = 0x0;

    // BaudRate = 115200
    U1BRG = ((uint32_t)SYSTEM_PERIPHERAL_CLOCK / (16 * (uint32_t)115200)) - 1;

    //Make sure to set LAT bit for TxPin high before UART initialization
    U1STASET = _U1STA_UTXEN_MASK;
    U1MODESET = _U1MODE_ON_MASK;  // enabling UART ON bit
    U1STASET = _U1STA_URXEN_MASK;
    
    // RX queue, enable RX interrupts
    rxi = rxo = 0;
    IEC1SET = _IEC1_U1RXIE_MASK;
}

uint8_t UART1_Read(void)
{
    uint8_t c;

    while (rxo == rxi);

    c = rxq[rxo];
    rxo = (rxo + 1) % RXQSIZ;

    return c;
}

void UART1_Write(uint8_t txData)
{
    while (U1STA & _U1STA_UTXBF_MASK);

    U1TXREG = txData;    // Write the data byte to the USART.
}

UART1_STATUS UART1_StatusGet(void)
{
    return U1STA;
}

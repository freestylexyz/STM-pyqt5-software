/*
 * SPI.c
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#include "SPI.h"

//
// InitSPI - This function initializes the SPI to a known state
//
void InitSpi(void)
{
    // Initialize SPI-A

    // Set reset low before configuration changes
    // Clock polarity (0 == rising, 1 == falling)
    // 16-bit character
    // Disable loop-back
    // Enable high speed mode
    SpiaRegs.SPICCR.bit.SPISWRESET = 0;
    SpiaRegs.SPICCR.bit.CLKPOLARITY = 1;
    SpiaRegs.SPICCR.bit.SPICHAR = (16-1);
    SpiaRegs.SPICCR.bit.SPILBK = 0;
    SpiaRegs.SPICCR.bit.HS_MODE = 0;

    // Enable master (0 == slave, 1 == master)
    // Enable transmission (Talk)
    // Clock phase (0 == normal, 1 == delayed)
    // SPI interrupts are disabled
    SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1;
    SpiaRegs.SPICTL.bit.TALK = 1;
    SpiaRegs.SPICTL.bit.CLK_PHASE = 1;
    SpiaRegs.SPICTL.bit.SPIINTENA = 0;

    // Set the baud rate
    SpiaRegs.SPIBRR.bit.SPI_BIT_RATE = SPI_BRR;

    // Set FREE bit
    // Halting on a breakpoint will not halt the SPI
    SpiaRegs.SPIPRI.bit.FREE = 1;

    // Release the SPI from reset
    SpiaRegs.SPICCR.bit.SPISWRESET = 1;


    //
    // Initialize SPI-B
    //

    // Set reset low before configuration changes
    // Clock polarity (0 == rising, 1 == falling)
    // 8-bit character
    // Disable loop-back
    // Enable high speed mode
    SpibRegs.SPICCR.bit.SPISWRESET = 0;
    SpibRegs.SPICCR.bit.CLKPOLARITY = 1;
    SpibRegs.SPICCR.bit.SPICHAR = (8-1);
    SpibRegs.SPICCR.bit.SPILBK = 0;
    SpibRegs.SPICCR.bit.HS_MODE = 0;

    // Enable master (0 == slave, 1 == master)
    // Enable transmission (Talk)
    // Clock phase (0 == normal, 1 == delayed)
    // SPI interrupts are disabled
    SpibRegs.SPICTL.bit.MASTER_SLAVE = 1;
    SpibRegs.SPICTL.bit.TALK = 1;
    SpibRegs.SPICTL.bit.CLK_PHASE = 1;
    SpibRegs.SPICTL.bit.SPIINTENA = 0;

    // Set the baud rate
    SpibRegs.SPIBRR.bit.SPI_BIT_RATE = SPI_BRR;

    // Set FREE bit
    // Halting on a breakpoint will not halt the SPI
    SpibRegs.SPIPRI.bit.FREE = 1;

    // Release the SPI from reset
    SpibRegs.SPICCR.bit.SPISWRESET = 1;
}

//
// spi_fifo_ini - This function initializes SPI FIFO
//
void spi_fifo_init()
{
    //
    // Initialize SPI FIFO registers
    //
    // SPI-A
    SpiaRegs.SPIFFTX.all = 0xE040;
    SpiaRegs.SPIFFRX.all = 0x2044;
    SpiaRegs.SPIFFCT.all = 0x0;
    // SPI-B
    SpibRegs.SPIFFTX.all = 0xE040;
    SpibRegs.SPIFFRX.all = 0x2044;
    SpibRegs.SPIFFCT.all = 0x0;

    //
    // Initialize core SPI registers
    //
    InitSpi();
}

//
// spia_xmit - This function transmit data to slave and return received data through SPIA
//
Uint16 spia_xmit(Uint16 sdata)
{
    Uint16 rdata;
    //Transmit data
    SpiaRegs.SPITXBUF = sdata;
    // Wait until data is received
    while(SpiaRegs.SPIFFRX.bit.RXFFST !=1) { }
    rdata = SpiaRegs.SPIRXBUF;
    return rdata;
}


//
// spib_xmit - This function transmit data to slave and return received data through SPIB
//
char spib_xmit(char sdata)
{
    char rdata;
    //Transmit data
    SpibRegs.SPITXBUF = (sdata & 0x00FF) << 8;              // For transmission data bit fewer than 16 bit, TX data is left-justified
    // Wait until data is received
    while(SpibRegs.SPIFFRX.bit.RXFFST !=1) { }
    rdata = (char) (0x00FF & (SpibRegs.SPIRXBUF));          // RX data is right-justified
    return rdata;
}


//
// End of file
//

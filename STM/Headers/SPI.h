/*
 * SPI.h
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_SPI_H_
#define HEADERS_SPI_H_

#include "DSP.h"

//
// Calculate BRR: 7-bit baud rate register value
// SPI CLK freq = 500 kHz
// LSPCLK freq  = CPU freq / 4  (by default)
// BRR          = (LSPCLK freq / SPI CLK freq) - 1
//
#if CPU_FRQ_200MHZ
#define SPI_BRRA        5        // Corresponding to 8.333MHz clock frequency
#define SPI_BRRB        13        // Corresponding to 3.57MHz clock frequency
#endif
//((200E6 / 4) / 10E6) - 1

//#if CPU_FRQ_150MHZ
//#define SPI_BRR        ((150E6 / 4) / 5E5) - 1
//#endif
//
//#if CPU_FRQ_120MHZ
//#define SPI_BRR        ((120E6 / 4) / 5E5) - 1
//#endif

typedef struct bytearray byte;

//
// Function declaration
//
void InitSpi(void);
void spi_fifo_init();
Uint16 spia_xmit(Uint16);
char spib_xmit(char);
byte spib_dac(byte);
byte spib_bit20(byte);

#endif /* HEADERS_SPI_H_ */
//
// End of file
//

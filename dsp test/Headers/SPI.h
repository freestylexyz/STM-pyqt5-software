/*
 * SPI.h
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_SPI_H_
#define HEADERS_SPI_H_

#include "F28x_Project.h"     // DSP28x Headerfile

//
// Calculate BRR: 7-bit baud rate register value
// SPI CLK freq = 500 kHz
// LSPCLK freq  = CPU freq / 4  (by default)
// BRR          = (LSPCLK freq / SPI CLK freq) - 1
//
#if CPU_FRQ_200MHZ
#define SPI_BRR        ((200E6 / 4) / 12.5E6) - 1
#endif

#if CPU_FRQ_150MHZ
#define SPI_BRR        ((150E6 / 4) / 12.5E6) - 1
#endif

#if CPU_FRQ_120MHZ
#define SPI_BRR        ((120E6 / 4) / 12.5E6) - 1
#endif

//
// Function declaration
//
void InitSpi(void);
void spi_fifo_init();
Uint16 spia_xmit(Uint16);
char spib_xmit(char);

#endif /* HEADERS_SPI_H_ */
//
// End of file
//

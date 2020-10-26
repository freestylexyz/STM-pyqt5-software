/*
 * Serial.h
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_SERIAL_H_
#define HEADERS_SERIAL_H_

#include "F28x_Project.h"     // DSP28x Headerfile

//
// Function declaration
//
void scia_echoback_init();
void scia_fifo_init();
char serialIn();
void serialOut_char(char);
void serialOut_two(Uint16 bytesOut);
void serialOut_word(Uint32);

#endif /* HEADERS_SERIAL_H_ */
//
// End of file
//

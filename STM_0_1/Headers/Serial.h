/*
 * Serial.h
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */
#include "DSP.h"
#ifndef HEADERS_SERIAL_H_
#define HEADERS_SERIAL_H_



//
// null symbol
//
#define Null       0x00
#define Stop       0xFF
#define Start      0xF0
#define Finish     0x0F
#define Ongoing    0x5A

typedef struct bytearray byte;

//
// Function declaration
//
void scia_echoback_init();
void scia_fifo_init();
byte serialIn(Uint16);
void serialOut(byte);
void serialOutArray(Uint16 *, Uint16);
void serialOutCharArray(char *, Uint16);
char serialCheck();
void serialEcho();


#endif /* HEADERS_SERIAL_H_ */
//
// End of file
//


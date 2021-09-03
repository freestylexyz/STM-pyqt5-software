/*
 * Fundamental.h
 *
 *  Created on: Nov 16, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_FUNDAMENTAL_H_
#define HEADERS_FUNDAMENTAL_H_

#include "DSP.h"

//
// Struct definination
//
struct bytearray
{
    Uint16 num;
    char arr[4];
};

typedef struct bytearray byte;

//
// numbers
//
#define Zero_16     0x8000
#define Max_16      0xFFFF
#define Min_16      0x0000
#define Zero_20     0x080000
#define Max_20      0x0FFFFF
#define Min_20      0x000000

//
// Function declaration
//
Uint32 combine(byte);
byte split(Uint32, Uint16);
Uint16 abs16(Uint16);
void updateLimit(Uint32*, Uint32*, Uint32);
Uint32 smaller(Uint32, Uint32);
Uint32 bigger(Uint32, Uint32);
bool reachLimit(char);
Uint32 current_output(char);
Uint32 output_limit(char);
void floatT();

#endif /* HEADERS_FUNDAMENTAL_H_ */
//
// End of file
//

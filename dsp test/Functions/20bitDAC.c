/*
 * 20bitDAC.c
 *
 *  Created on: Sep 9, 2020
 *      Author: yaoji
 */

#include "20bitDAC.h"

//
// Global variables definition
//
Uint32 last20bit = 0x00080000;

//
// bit20_RST_H - This function reset 20bitDAC
//
void bit20_RST_H()
{
    GPIO_WritePin(RST20, 0);     // Pull DACRST pin to low
    DELAY_US(1);                 // Wait 1us (waiting time need to longer than 35ns)
    GPIO_WritePin(RST20, 1);     // Pull DACRST pin to high
    DELAY_US(20);                // Wait 20us
}

//
// bit20_CLR_H - This function 20bitDAC to clear code
//
void bit20_CLR_H()
{
    GPIO_WritePin(CLR20, 0);    // Pull DACCLR pin to low
    DELAY_US(1);                // Wait 1us
    GPIO_WritePin(CLR20, 1);    // Pull DACCLR pin to high
    DELAY_US(20);               // Wait 20us
}

//
// bit20_W - This function write data to 20bitDAC register
// 1st MSB = 0 sets a write operation
//
void bit20_W(char addr, Uint32 data)
{
    char byteA, byteB, byteC;

    byteA = (char)((0x70 & addr) | ((data & 0x000F0000) >> 16));       //Patch address with write flag and 4 MSB of 20bit data for first sent byte
    byteB = (char)((data & 0x0000FF00) >> 8);                          //Split 16 LSB of 20bit data into 2 byte for sending
    byteC = (char)(data & 0x000000FF);

    GPIO_WritePin(CS20, 0);                                            //Pull CSB low to initiate data transmission
    spib_xmit(byteA);                                                  //Send first byte to DAC
    spib_xmit(byteB);                                                  //Send second byte to DAC
    spib_xmit(byteC);                                                  //Send third byte to DAC
    GPIO_WritePin(CS20, 1);                                            //Pull CSB high to terminate data transmission

    DELAY_US(20);                                                      // Wait 5us for output to stabilize, settling time 1us

    if ((0x70 & addr) == 0x10)
    {
        last20bit = data;                                              // Update last DAC output if writing to 20bit-DAC DAC register
    }
}

//
// bit20_W_DSP - This function received packed data from PC and output to 20bit-DAC
// 1st MSB = 0 sets a write operation
//
void bit20_W_DSP()
{
    char byteA, byteB, byteC;

    byteA = serialIn() & 0x7F;                             // Obtain first byte to send
                                                           // DB23: R/W
                                                           // DB22-DB20: Register address
    byteB = serialIn();                                    // Obtain second byte to send
                                                           // Data most significant byte
    byteC = serialIn();                                    // Obtain third byte to send
                                                           // Data least significant byte

    GPIO_WritePin(CS20, 0);                                 // Pull CS20 low to initiate data transmission
    spib_xmit(byteA);                                       // Send first byte to 20bit-DAC
    spib_xmit(byteB);                                       // Send second byte to 20bit-DAC
    spib_xmit(byteC);                                       // Send third byte to 20bit-DAC
    GPIO_WritePin(CS20, 1);                                 // Pull CS20 high to terminate data transmission

    DELAY_US(20);                                           // Wait 20us for output to stabilize, settling time 1us

    if ((0x70 & byteA) == 0x10)
    {
        last20bit = ((Uint32)(byteA & 0x0F) << 16) | ((Uint32)byteB << 8) | (Uint32)byteC;  // Update last DAC output if writing to 20bit-DAC DAC register
    }
}

//
// bit20_R_DSP - This function received packed data from PC read data from 20bitDAC register on command and return data to PC
// 1st MSB = 1 sets a read operation
//
void bit20_R_DSP()
{
    char byteA;
    char bytea, byteb, bytec;

    byteA = (0x70 & serialIn()) | 0x80;     // Patch address with read flag for first sent byte

    GPIO_WritePin(CS20, 0);                 // Pull CS20 low to initiate data transmission
    spib_xmit(byteA);                       // Send first byte to 20bit-DAC
    spib_xmit(0);                           // Send second byte to 20bit-DAC, 20bit-DAC doesn't care
    spib_xmit(0);                           // Send third byte to 20bit-DAC, 20bit-DAC doesn't care
    GPIO_WritePin(CS20, 1);                 // Pull CS20 high to terminate data transmission

    DELAY_US(1);                            // Wait 1us

    GPIO_WritePin(CS20, 0);                 // Pull CS20 low to initiate data transmission
    bytea = spib_xmit(0);                   // Send first byte to 20bit-DAC, no operation
    byteb = spib_xmit(0);                   // Send second byte to 20bit-DAC
    bytec = spib_xmit(0);                   // Send third byte to 20bit-DAC
    GPIO_WritePin(CS20, 1);                 // Pull CS20 high to terminate data transmission

    serialOut_char(bytea);                  // Return read address + DB19-DB16
    serialOut_char(byteb);                  // Return DB15-DB8
    serialOut_char(bytec);                  // Return DB7-DB0
}

//
// bit20Init - This function initiate 20bitDAC to a known state
//
void bit20Init()
{
    bit20_W(BIT20_CONTROL, 0x00000012);       // Linear error compensation selected to 10V span (DB9-DB6: 0000)
                                              // SDO enabled (DB5: 0)
                                              // DAC register uses offset binary coding style (DB4: 1)
                                              // Tri-state mode disabled (DB3: 0)
                                              // GND clamp disabled (DB2: 0)
                                              // Internal amplifier powered down (DB1: 1)
                                              // All other data bits are reserved, need to be set to zero
    bit20_W(BIT20_CLEARCODE, 0x00080000);     // Initial clear code register to mid scale code
    bit20_W(BIT20_DAC, 0x00080000);           // Initial DAC register to mid-scale code
}

//
// End of file
//

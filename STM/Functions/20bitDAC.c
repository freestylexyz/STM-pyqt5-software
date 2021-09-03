/*
 * 20bitDAC.c
 *
 *  Created on: Sep 9, 2020
 *      Author: yaoji
 */

#include "20bitDAC.h"


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
    byte sdata;
    sdata = split(data, 3);
    sdata.arr[0] = (char)((0x70 & addr) | (sdata.arr[0] & 0x0F));       //Patch address with write flag and 4 MSB of 20bit data for first sent byte

    spib_bit20(sdata);
    DELAY_US(5);                                                        // Wait 5us for output to stabilize, settling time 1us

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
    byte sdata;

    sdata = serialIn(3);
    sdata.arr[0] = sdata.arr[0] & 0x7F;         // Use effective address data only
    spib_bit20(sdata);

    DELAY_US(5);                                // Wait 5us for output to stabilize, settling time 1us

    if ((0x70 & sdata.arr[0]) == 0x10)
    {
        sdata.arr[0] = sdata.arr[0] & 0x0F;      // Remove address information
        last20bit = combine(sdata);             // Update last DAC output if writing to 20bit-DAC DAC register
    }
}

//
// bit20_R_DSP - This function received packed data from PC read data from 20bitDAC register on command and return data to PC
// 1st MSB = 1 sets a read operation
//
void bit20_R_DSP()
{
    byte rdata;
    byte zero_3 = split(0, 3);
    byte sdata = split(0, 3);

    sdata.arr[0] = (0x70 & combine(serialIn(1))) | 0x80;    // Patch address with read flag for first sent byte

    spib_bit20(sdata);                  // Transmit data
    DELAY_US(1);                        // Wait 1us
    rdata = spib_bit20(zero_3);         // Read data
    serialOut(rdata);                   // Serial output received data
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

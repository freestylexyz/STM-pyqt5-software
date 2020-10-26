/*
 * DAC.c
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#include "DAC.h"

//
// Global variables definition
//
Uint16 dacrange[4];
Uint16 lastdac[16];

//
// dac_RST_H - This function reset DAC
//
void dac_RST_H()
{
    GPIO_WritePin(DACRST, 0);   // Pull DACRST pin to low
    DELAY_US(1);                // Wait 1us (waiting time need to longer than 500ns)
    GPIO_WritePin(DACRST, 1);   // Pull DACRST pin to high
    DELAY_US(2000);             // Wait 2ms, communication is valid after 1ms
}

//
// dac_CLR_H - This function clear all DAC channels to clear code
// Unipolar, No differential -- Zero code
// Unipolar, Differential -- Mid-scale code
// Bipolar, No differential -- Mid-scale code
// Bipolar, Differential -- Mid-scale code
//
void dac_CLR_H()
{
    GPIO_WritePin(DACCLR, 0);   // Pull DACCLR pin to low
    DELAY_US(1);                // Wait 1us (waiting time need to be longer than 20ns
    GPIO_WritePin(DACCLR, 1);   // Pull DACCLR pin to high
    DELAY_US(20);               // Wait 20us
}

//
// dac_W - This function write data to DAC register
// 1st MSB = 0 sets a write operation
//
void dac_W(char addr, Uint16 data)
{
    char byteA, byteB, byteC;

    byteA = (0x3F & addr);                  // Patch address with write flag for first sent byte
    byteB = (data & 0xFF00) >> 8;           // Split data into 2 byte for sending
    byteC = data & 0x00FF;

    GPIO_WritePin(CSB, 0);                  // Pull CSB low to initiate data transmission
    spib_xmit(byteA);                       // Send first byte to DAC
    spib_xmit(byteB);                       // Send second byte to DAC
    spib_xmit(byteC);                       // Send third byte to DAC
    GPIO_WritePin(CSB, 1);                  // Pull CSB high to terminate data transmission

    DELAY_US(20);                           // Wait 20us, settling time is 12us

    if ((0x30 & addr) == 0x10)
    {
        lastdac[0x0F & addr] = data;         // Update last DAC output if writing to DAC register
    }
}

//
// DAC_W_DSP - This function received packed data from PC and output to DAC
// 1st MSB = 0 sets a write operation
//
void dac_W_DSP()
{
    char byteA, byteB, byteC;

    byteA = serialIn() & 0x3F;     // Obtain first byte to send
                                   // DB23: R/W
                                   // DB22: Don't care
                                   // DB21-DB16: register address
    byteB = serialIn();            // Obtain second byte to send
                                   // Data most significant byte
    byteC = serialIn();            // Obtain third byte to send
                                   // Data least significant byte

    GPIO_WritePin(CSB, 0);         // Pull CS20 low to initiate data transmission
    spib_xmit(byteA);              // Send first byte to 20bit-DAC
    spib_xmit(byteB);              // Send second byte to 20bit-DAC
    spib_xmit(byteC);              // Send third byte to 20bit-DAC
    GPIO_WritePin(CSB, 1);         // Pull CS20 high to terminate data transmission
    DELAY_US(20);                  // Wait 20us, settling time is 12us

    if ((0x30 & byteA) == 0x10)
    {
        lastdac[0x0F & byteA] = ((byteB & 0x00FF) << 8) | (byteC & 0x00FF);    // Update last DAC output if writing to DAC register
    }
}

//
// dac_R_DSP - This function received packed data from PC read data from DAC register on command and return data to PC
// 1st MSB = 1 sets a read operation
//
void dac_R_DSP()
{
    char byteA;
    char bytea, byteb, bytec;

    byteA = (0x3F & serialIn()) | 0x80;  // Patch address with read flag for first sent byte

    GPIO_WritePin(CSB, 0);               // Pull CSB low to initiate data transmission
    spib_xmit(byteA);                    // Send first byte to DAC
    spib_xmit(0);                        // Send second byte to DAC, DAC doesn't care
    spib_xmit(0);                        // Send third byte to DAC, DAC doesn't care
    GPIO_WritePin(CSB, 1);               // Pull CSB high to terminate data transmission

    DELAY_US(1);                         // Wait 1us

    GPIO_WritePin(CSB, 0);               // Pull CSB low to initiate data transmission
    bytea = spib_xmit(0);                // Send first byte to DAC, no operation
    byteb = spib_xmit(0);                // Send second byte to DAC
    bytec = spib_xmit(0);                // Send third byte to DAC
    GPIO_WritePin(CSB, 1);               // Pull CSB high to terminate data transmission

    serialOut_char(bytea);               // Return read address
    serialOut_char(byteb);               // Return DB15-DB8
    serialOut_char(bytec);               // Return DB7-DB0
}

//
// dacInit - This function initiate DAC to a known state
//
void dacInit()
{
    Uint16 i;
    dac_W(DAC_SPICONFIG, 0x0086);       // Reserved (DB15-DB12: 0000)
                                        // TEMPALM disabled (DB11: 0)
                                        // DACBUSY disabled (DB10: 0)
                                        // CRCALM disabled (DB9: 0)
                                        // Reserved (DB8-DB7: 01)
                                        // SOFTTOGGLE disabled (DB6: 0)
                                        // DEVICE activated (DB5: 0)
                                        // CRC disabled (DB4: 0)
                                        // Streaming mode disabled (DB3: 0)
                                        // SDO operational (DB2: 1)
                                        // SDO update during SCK falling edge (DB1: 1)
                                        // Reserved (DB0: 0)
    dac_W(DAC_GENCONFIG, 0x3F00);       // Reserved (DB15: 0)
                                        // Enable internal reference (DB14: 0)
                                        // Reserved (DB13-DB8: 111111)
                                        // Disable differential mode for all channels (DB7-DB0: 00000000)
    dac_W(DAC_RANGE0, 0xAAAA);          // Set range to +/-10V for channel 15-12
    dacrange[0] = 0xAAAA;
    dac_W(DAC_RANGE1, 0xAAAA);          // Set range to +/-10V for channel 11-8
    dacrange[1] = 0xAAAA;
    dac_W(DAC_RANGE2, 0xAAAA);          // Set range to +/-10V for channel 7-4
    dacrange[2] = 0xAAAA;
    dac_W(DAC_RANGE3, 0xAAAA);          // Set range to +/-10V for channel 3-0
    dacrange[3] = 0xAAAA;
    dac_W(DAC_PD, 0x0000);              // Disable power down mode for all channels
    dac_CLR_H();                        // Clear all DACs to their mid-scale code
    for (i = 0; i < 16; i++ )
    {
        lastdac[i] = 0x8000;            // Loop over lastdac and set to mid-scale code
    }
}

//
// dacRange - This function received packed data from PC and change the corresponding channel output range
// data format : channel, range
//
void dacRange()
{
    Uint16 rangeb, channel, range, chq, chr;
    char byteA, addr;

    byteA = serialIn();                     // Read data form PC
    channel = (byteA & 0xF0) >> 4;          // Obtain channel data
    range = byteA & 0x0F;                   // Obtain range data

    chq = channel / 4;                      // Figure out which register should be written
    chr = channel % 4;                      // Figure out which digits need to changed

    addr = 0x0A + chq;                      // Obtain the register address for selected channel

    // Patch data for sending based on channel select
    switch (chr){
    case 0 :
        rangeb = (0xFFF0 & dacrange[chq]) | (0x000F & range);
        break;
    case 1 :
        rangeb = (0xFF0F & dacrange[chq]) | ((0x000F & range) << 4);
        break;
    case 2 :
        rangeb = (0xF0FF & dacrange[chq]) | ((0x000F & range) << 8);
        break;
    case 3 :
        rangeb = (0x0FFF & dacrange[chq]) | ((0x000F & range) << 12);
        break;
    }

    dac_W(addr, rangeb);                    // Set desired range for selected channel
    dacrange[chq] = rangeb;                 // Update corresponding range variable
    }


//
// End of file
//

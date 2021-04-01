/*
 * DAC.c
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#include "DAC.h"

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
    byte sdata;
    sdata = split(data, 3);                 // Split data into bytes
    sdata.arr[0] = (0x3F & addr);           // Patch address with write flag for first sent byte

    spib_dac(sdata);                        // Transmit data
//    DELAY_US(20);                           // Wait 20us, settling time is 12us

    // Check if writing to DAC register
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
    Uint16 addr;
    byte sdata;
    sdata = serialIn(3);                    // Serial input 3 bytes of data

    addr = sdata.arr[0] & 0x3F;             // Pull out address information
    sdata.arr[0] = addr;                    // Use effective address data only

    spib_dac(sdata);                        // Transmit data
    DELAY_US(20);                           // Wait 20us, settling time is 12us

    // Check if writing to DAC register
    if ((0x30 & addr) == 0x10)
    {
        sdata.arr[0] = 0;                       // Remove address information
        lastdac[0x0F & addr] = combine(sdata);  // Update last DAC output if writing to DAC register
    }
}

//
// dac_R_DSP - This function received packed data from PC read data from DAC register on command and return data to PC
// 1st MSB = 1 sets a read operation
//
void dac_R_DSP()
{
    byte rdata;
    byte zero_3 = split(0, 3);
    byte sdata = split(0, 3);

    sdata.arr[0] = (0x3F & combine(serialIn(1))) | 0x80;    // Patch address with read flag for first sent byte

    spib_dac(sdata);                    // Transmit data
    DELAY_US(1);                        // Wait 1us
    rdata = spib_dac(zero_3);           // Read data
    serialOut(rdata);                   // Serial output received data
}

//
// dacInit - This function initiate DAC to a known state
//
void dacInit()
{
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
    dac_W(DAC_RANGE0, 0xAA9E);          // Set range for channel 15-12. +/-10V for channel 15 and 14. +/-2.5V for channel 12 (Bias offset). +/-2.5V for channel 13 (Bias).
    dac_W(DAC_RANGE1, 0xAAAA);          // Set range to +/-10V for channel 11-8
    dac_W(DAC_RANGE2, 0xAAAA);          // Set range to +/-10V for channel 7-4
    dac_W(DAC_RANGE3, 0xAEAA);          // Set range for channel 3-0. +/-10V for channel 3, 1 and 0. +/-2.5V for channel 2 (Z offset fine)
    dac_W(DAC_PD, 0x0000);              // Disable power down mode for all channels
    dac_CLR_H();                        // Clear all DACs to their mid-scale code
}

//
// dacRange - This function received packed data from PC and change the corresponding channel output range
// data format : channel, range
//
void dacRange()
{
    Uint16 rangeb, channel, range, chq, chr;
    char byteA, addr;

    byteA = combine(serialIn(1));           // Read data form PC
    channel = (byteA & 0xF0) >> 4;          // Obtain channel data
    range = byteA & 0x0F;                   // Obtain range data

    chq = channel / 4;                      // Figure out which register should be written
    chr = channel % 4;                      // Figure out which digits need to changed

    addr = 0x0D - chq;                      // Obtain the register address for selected channel

    // Patch data for sending based on channel select
    switch (chr){
    case 0 :
        rangeb = (0xFFF0 & dacrange[3 - chq]) | (0x000F & range);
        break;
    case 1 :
        rangeb = (0xFF0F & dacrange[3 - chq]) | ((0x000F & range) << 4);
        break;
    case 2 :
        rangeb = (0xF0FF & dacrange[3 - chq]) | ((0x000F & range) << 8);
        break;
    case 3 :
        rangeb = (0x0FFF & dacrange[3 - chq]) | ((0x000F & range) << 12);
        break;
    default:
        break;
    }

    dac_W(addr, rangeb);            // Set desired range for selected channel
    dacrange[3 - chq] = rangeb;     // Update corresponding range variable
    if(channel == (Bias - 16))      // If change bias output range
    {
        dac_W(Boff, offset[range]); // Write bias offset to corresponding offset value
    }
    }


//
// End of file
//

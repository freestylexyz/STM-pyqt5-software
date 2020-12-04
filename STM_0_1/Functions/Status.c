/*
 * status.c
 *
 *  Created on: Nov 16, 2020
 *      Author: yaoji
 */

#include "Status.h"

Uint16 dacrange[4];         // DAC range status
Uint16 lastdac[16];         // The last output data of all DAC channels
Uint16 offset[16];          // Offset status of different range of bias output and I set
char adcrange[8];           // ADC range status
Uint32 last20bit;           // The last output data of 20bit DAC
char lastdigital[14];       // Data order: dither0, dither1, feedback, retract, coarse, rot, xgain0, xgain1, ygain0, ygain1, zgain0, zgain1, zgain2, zgain3


//
// version - Output DSP version number
//
void version()
{
    char* dsp_version = FIRMWARE_VERSION;
    serialOutCharArray(dsp_version, 12);
}

//
// status - Output the current status of DSP
//
void status()
{
    serialOutArray(dacrange, 4);        // Send out DAC range data
    serialOutArray(lastdac, 16);        // Send out last DAC output data
    serialOutArray(offset, 16);         // Send out DAC offset data
    serialOutCharArray(adcrange, 8);    // Send out ADC range data
    serialOut(split(last20bit, 3));     // Send out last 20bit DAC output data
    lastdigital_O();                    // Send out last digital output data
}

//
// lastdigital_O - Output the last digital output data
//
void lastdigital_O()
{
    Uint16 i;
    char rdata;

    // Patch all 14 digital output into a 2 byte number
    rdata = 0x0001 & lastdigital[0];
    for(i = 1; i < 6; i++)
    {
        rdata = rdata << 1;
        rdata += 0x0001 & lastdigital[i];
    }
    serialOut(split(rdata,1));      // Serial out digital data

    rdata = 0x0001 & lastdigital[6];
    for(i = 7; i < 14; i++)
    {
        rdata = rdata << 1;
        rdata += 0x0001 & lastdigital[i];
    }
    serialOut(split(rdata,1));      // Serial out gain data
}

//
// initGlobal - This function initial all global variables
//
void initGlobal()
{
    Uint16 i;
    for(i = 0; i < 16; i++){lastdac[i] = 0x8000;}   // Initial all DAC last output to mid-scale
    for(i = 0; i < 4; i++){dacrange[i] = 0xAAAA;}   // Initial all DAC range to +/- 10V
    for(i = 0; i < 8; i++){adcrange[i] = 0x00;}     // Initial all ADC range to +/- 10.24V
    for(i = 0; i < 14; i++){lastdigital[i] = 0x01;} // Initial all digital output to 1
    last20bit = 0x00080000;

    lastdigital[0] = 0x00;      // Dither0 is off
    lastdigital[1] = 0x00;      // Dither1 is off
    lastdigital[3] = 0x00;      // Retract is off
    lastdigital[5] = 0x00;      // Translation mode

    dacrange[0] = 0xAA9E;       // Bias range (+/- 5V), Bias offset range (+/- 2.5V)
    dacrange[3] = 0xAEAA;       // Z offset fine range (+/- 2.5V)

    // NEED TO UPDATE OFFSET GLOBAL VARIABLE BEFORE SET OFFSET
    // This need to be updated after measure the real offset
    for(i = 0; i < 16; i++){offset[i] = 0x8000;}    // Initial all offset to mid-scale (0V)
    offset[15] = 0x8167;        // Iset offset is 0x8167
    offset[14] = 0x8026;        // Bias +/-2.5V offset is 0x8026
    offset[12] = 0x804e;        // Bias +/-20V offset is 0x804e
    offset[10] = 0x802d;        // Bias +/-10V offset is 0x802d
    offset[9] = 0x802e;         // Bias +/-5V offset is 0x802e
    offset[4] = 0x8066;         // Bias 0 - 40V offset is 0x8066
    offset[2] = 0x804f;         // Bias 0 - 20V offset is 0x804f
    offset[1] = 0x8042;         // Bias 0 - 10V offset is 0x8042
    offset[0] = 0x803e;         // Bias 0 - 5V offset is 0x803e
}

//
// loadOffset - This function load offset transmitted from PC
//
void loadOffset()
{
    Uint16 i, data, range;
    for(i = 0; i < 16; i++)
    {
        data = combine(serialIn(2));
        offset[i] = data;
    }
    range = (dacrange[0] & 0x00F0) >> 4;
    dac_W(Boff, offset[range]);
}
//
// End of file
//

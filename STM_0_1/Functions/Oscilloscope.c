/*
 * Oscilloscope.c
 *
 *  Created on: Nov 19, 2020
 *      Author: yaoji
 */

#include "Oscilloscope.h"

Uint16 ldata[4096];     // Large data storage

//
// OSC_C - This function perform continuous measurement of specific ADC channel on PC command
//
void OSC_C()
{
    char byteA;
    Uint16 i, n, command, rdata;
    Uint32 sum = 0;

    byteA = combine(serialIn(1));       // Obtain most significant byte for channel address
    command = ((byteA & 0x00FF) << 8);  // Patch command data
    n = combine(serialIn(2));           // Obtain average times for each data point

    adc_command(command);               // Start a conversion of specific channel

    serialOut(split(Start, 1));         // Send out start command
    while(true)
    {
        for(i = 0; i < n; i++){sum += adc_command(command);}   // Measure n times of selected channel
        rdata = sum / n;                    // Return averaged conversion result of corresponding channel
        serialOut(split(rdata, 2));         // Output data through serial
        if(serialCheck() == Stop)           //Check stop
        {break;}
    }
    serialOut(split(Finish, 1));            // Send out finish command
    serialOut(split(0xAA55A55A, 4));    // Send out a stop sequence to distinguish with remaining data
}

//
// OSC_N - This function perform continuous N samples (2048 max) measurement of specific ADC channel on PC command
//
void OSC_N()
{
    char byteA;
    Uint16 i, n, command;

    byteA = combine(serialIn(1));       // Obtain most significant byte for channel address
    command = ((byteA & 0x00FF) << 8);  // Patch command data
    n = combine(serialIn(2));           // Obtain average times for each data point

    adc_command(command);               // Start a conversion of specific channel

    for(i = 0; i < n; i++){ldata[i] = adc_command(command);}   // Measure n times of selected channel
    serialOutArray(ldata, n);                   // Transmit data
}

//
// End of file
//

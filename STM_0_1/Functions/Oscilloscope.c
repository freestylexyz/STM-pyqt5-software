/*
 * Oscilloscope.c
 *
 *  Created on: Nov 19, 2020
 *      Author: yaoji
 */

#include "Oscilloscope.h"

Uint16 ldata[4096];     // Large data storage

//
// OSC_C - This function perform continuous measurement of specific ADC channel
//
void OSC_C(Uint16 command, Uint16 average, Uint16 delay)
{
    Uint16 i, rdata;
    Uint32 sum = 0;
    adc_command(command);               // Start a conversion of specific channel
    while(true)
    {
        for(i = 0; i < average; i++){sum += adc_command(command);}  // Measure n times of selected channel
        rdata = sum / average;                                      // Return averaged conversion result of corresponding channel
        serialOut(split(rdata, 2));                                 // Output data through serial
        if(serialCheck() == Stop){break;}                           //Check stop
    }
}

//
// OSC_C_DSP - This function perform continuous measurement of specific ADC channel on PC command
//
void OSC_C_DSP()
{
    char byteA;
    Uint16 n, command, delay;

    byteA = combine(serialIn(1));       // Obtain most significant byte for channel address
    command = ((byteA & 0x00FF) << 8);  // Patch command data
    n = combine(serialIn(2));           // Obtain average times for each data point
    delay = combine(serialIn(2));       // Obtain delay time between data points

    serialOut(split(Start, 1));         // Send out start command
    OSC_C(command, n, delay);           // Continuous OSC
    serialOut(split(Finish, 1));        // Send out finish command
    serialOut(split(0xAA55A55A, 4));    // Send out a stop sequence to distinguish with remaining data
}

//
// OCS_N - This function perform N samples (4096 max) measurement of specific ADC channel
//
void OSC_N(Uint16 command, Uint16 n, Uint16 average, Uint16 delay)
{
    Uint32 sum = 0;
    Uint16 i, j;
    adc_command(command);               // Start a conversion of specific channel
    for(i = 0; i < n; i++)
    {
        for(j = 0; j < average; j++){sum += adc_command(command);}  // Measure n times of selected channel
        ldata[i] = sum / average;                                   // Return averaged conversion result of corresponding channel
        DELAY_US(delay);                                            // Delay
    }
}

//
// OSC_N_DSP - This function perform N samples (4096 max) measurement of specific ADC channel on PC command
//
void OSC_N_DSP()
{
    char byteA;
    Uint16 n, command, average, delay;

    byteA = combine(serialIn(1));       // Obtain most significant byte for channel address
    command = ((byteA & 0x00FF) << 8);  // Patch command data
    n = combine(serialIn(2));           // Obtain total number of data point
    average = combine(serialIn(2));     // Obtain average times for each data point
    delay = combine(serialIn(2));       // Obtain delay between reading

    OSC_N(command, n, average, delay);  // Measure n times of selected channel
    serialOutArray(ldata, n);           // Transmit data
}

//
// OSC_U - This function read one channel until specific change has happened with N samples mode
// Return total number data points read
//
Uint16 OCS_U(Uint16 command, Uint16 average, Uint16 delay, Uint16 limit, Uint16 stopNum, bool* detected)
{
    Uint32 initial, sum, change;
    Uint16 i, j, num, rdata, k;

    *detected = false;                      // Initialize detected to false
    rdata = adc_CNV_N(command, average);    // First read
    ldata[0] = rdata;                       // Store data into first spot
    initial = rdata + 0x80000000;           // Add a big offset to avoid data overflow during subtraction
    for(i = 1; i < 4096; i++)               // Start from second data
    {
        DELAY_US(delay);                        // Delay
        for(j = 0; j < average; j++){sum += adc_command(command);}  // Measure n times of selected channel
        rdata = sum / average;                                      // Return averaged conversion result of corresponding channel
        ldata[i] = rdata;                                           // Store data
        change = initial - rdata;                                   // Calculate change
        if(change > 0x80000000){change = change - 0x80000000;}       // Absolute change
        else{change = 0x80000000 - change;}
        if(change > limit)                                          // Determine change
        {
            *detected = true;
            break;
        }
    }
    if(*detected){i++;}                 // If detected, there in no chance to increase
    num = bigger(4096, i + stopNum);    // Calculate target total number of data points
    if((stopNum != 0) && *detected)
    {
        k = i;
        for(i = k; i < num; i++)
        {
            DELAY_US(delay);                                            // Delay
            for(j = 0; j < average; j++){sum += adc_command(command);}  // Measure n times of selected channel
            ldata[i] = sum / average;                                   // Return averaged conversion result of corresponding channel
        }
    }
    return i;
}

//
// End of file
//

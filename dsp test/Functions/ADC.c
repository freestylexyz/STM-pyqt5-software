/*
 * ADC.c
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#include "ADC.h"

//
// Global variables definition
//
char adcrange[8];

//
// adc_RST - This function place ADC into hardware RST mode
//
//void adc_RST_H()
//{
//    Uint16 i = 0;
//    GPIO_WritePin(ADCRST, 0);   // Pull ADCRST pin to low
//    for (i = 0; i < 1; i++){}   // Wait (waiting time need to longer than 40ns but smaller than 100ns)
//    GPIO_WritePin(ADCRST, 1);   // Pull ADCRST pin to high
//    DELAY_US(20);               // Wait 20us
//}
// Impossible to keep the low time smaller than 100ns, smallest low time = 310ns, every for loop cycle can delay additional 90ns
// Not able to achieve hardware reset with F28379D

//
// adc_PD - This function place ADC into hardware Power-Down mode
// Internal reference need at least 15ms to stabilize after device waken up from Power-Down mode
// Hardware Power-Down reset all program register
//
void adc_PD_H()
{
    GPIO_WritePin(ADCRST, 0);   // Pull ADCRST pin to low
    DELAY_US(1);                // Wait 1us (waiting time need to longer than 400ns)
    GPIO_WritePin(ADCRST, 1);   // Pull ADCRST pin to high
    DELAY_US(20);               // Wait 20us
}

//
// adc_command - This function send a command to ADC and return conversion result specified from last data frame
//
Uint16 adc_command(Uint16 command)
{
    Uint16 rdata;
    GPIO_WritePin(CSA, 0);       // Pull CSA low to initiate data transmission
    spia_xmit(command);          // Send command to ADC
    rdata = spia_xmit(0x0000);   // Acquire conversion data
    GPIO_WritePin(CSA, 1);       // Pull CSA high to terminate data transmission
    return rdata;
}

//
// adc_CNV - This function return the conversion result of corresponding channel
//
Uint16 adc_CNV(Uint16 command)
{
    GPIO_WritePin(CSA, 0);        // Pull CSA low to initiate data transmission
    spia_xmit(command);           // Send command to ADC
    GPIO_WritePin(CSA, 1);        // Pull CSA high to terminate data transmission
                                  // No need to read returned conversion data
    return adc_command(ADC_NOP);  // Return conversion result of corresponding channel
}

//
// adc_CNV_N - This function return the conversion result of corresponding channel averaged n times
//
Uint16 adc_CNV_N(Uint16 command, Uint16 n)
{
    Uint16 i;
    Uint32 sum = 0;

    GPIO_WritePin(CSA, 0);           // Pull CSA low to initiate data transmission
    spia_xmit(command);              // Send command to ADC
    GPIO_WritePin(CSA, 1);           // Pull CSA high to terminate data transmission
                                     // No need to read returned conversion data
    for (i=0; i<n; i++)
    {
        sum += adc_command(command); // Measure n times of selected channel
    }
    return (Uint16)(sum / n);        // Return averaged conversion result of corresponding channel
}

//
// adc_DSP - This function received channel address data from PC and send back the corresponding channel conversion result
//
void adc_DSP()
{
    char byteA;
    Uint16 command, rdata;

    byteA = serialIn();                         // Obtain most significant byte for channel address
    command = ((byteA & 0x00FF) << 8);          // Patch command data

    rdata = adc_CNV(command);                   // Return conversion result of corresponding channel

    serialOut_two(rdata);                       // Serial out 16-bit data, MSB first
}

//
// adc_N_DSP - This function received channel address data and averaging number from PC
// Send back the averaged corresponding channel conversion result
//
void adc_N_DSP()
{
    char byteA, byteB, byteC;
    Uint16 command, rdata, n;

    byteA = serialIn();                             // Obtain most significant byte for channel address
    byteB = serialIn();                             // Obtain most significant byte for averaging number
    byteC = serialIn();                             // Obtain most significant byte for averaging number

    command = ((byteA & 0x00FF) << 8);              // Patch command data
    n = ((byteB & 0x00FF) << 8) | (byteC & 0x00FF); //Patch averaging number data

    rdata = adc_CNV_N(command, n);                    // Return conversion result of corresponding channel
    serialOut_two(rdata);                           // Serial out 16-bit data, MSB first
}

//
// adc_W_DSP - This function received packed data from PC and output to ADC
// 8th MSB = 1 sets a write operation
//
void adc_W_DSP()
{
    char addr, data;
    Uint16 sdata, x;

    addr = serialIn();                                                  // Obtain address data
    data = serialIn();                                                  // Obtain data need to written

    x = addr & 0x00FE;                                                  // Intermediate variable
    sdata = (x << 8) | 0x0100 | (data & 0x00FF);                        // Patch data for sending

    GPIO_WritePin(CSA, 0);                                              // Pull CSA low to initiate data transmission
    spia_xmit(sdata);                                                   // Send data to ADC
    spia_xmit(0x0000);                                                  // Acquire read data
    GPIO_WritePin(CSA, 1);                                              // Pull CSA high to terminate data transmission

    if ((x > 0x08) && (x < 0x1A))
    {
        adcrange[(x >> 1) - 4] = data;                                   // Update ADC range if writing to ADC range register
    }
}

//
// adc_R_DSP - This function received packed data from PC read data from adc register on command and return data to PC
// 8th MSB = 0 sets a read operation
//
void adc_R_DSP()
{
    char addr;
    Uint16 sdata, rdata;

    addr = serialIn();                               // Obtain address data
    sdata = ((addr & 0x00FE) << 8);                  // Patch data for sending

    GPIO_WritePin(CSA, 0);                           // Pull CSA low to initiate data transmission
    spia_xmit(sdata);                                // Send data to ADC
    rdata = spia_xmit(0x0000);                       // Acquire read data
    GPIO_WritePin(CSA, 1);                           // Pull CSA high to terminate data transmission

    serialOut_char((char)((rdata & 0xFF00) >> 8));   // Return read data to PC
}

//
// adcInit - This function initiate ADC to a known state
//
void adcInit()
{
    adc_CNV(ADC_CH0);    // Activate ADC
    DELAY_US(100);       // Wait 100us
}

//
// End of file
//

/*
 * Fundamental.c
 *
 *  Created on: Nov 16, 2020
 *      Author: yaoji
 */

#include "Fundamental.h"

//
// combine - convert byte array into an integer number. MSB first
//
Uint32 combine(byte bytearray)
{
    int i;
    Uint32 num = (Uint32) bytearray.arr[0];                                 // Add first MSB

    for(i = 1; i < bytearray.num; i++)
        {
            num = num << 8;                                                 // Shift current number toward higher order bit
            num += ((Uint32) bytearray.arr[i]) & 0x000000FF;                // Add next byte
        }
    return num;
}

//
// split - convert an integer into a byte array. MSB first
//
byte split(Uint32 num, Uint16 byte_num)
{
    int i;
    static byte  bytearray;                                             // Need to copy the data out of this space. It will be overwritten every time when this function called

    bytearray.num = byte_num;                                           // Pass down byte number
    // Write array data
    for(i = 0; i < byte_num; i++)
    {
        bytearray.arr[byte_num - 1 - i] = (char) (num & 0x000000FF);    // Deal with LSB first
        num = (num & 0xFFFFFF00) >> 8;                                    // Shift the rest digits toward lower order bit
    }
    return bytearray;                                                   // Only the first "byte_num" of char space is meaningful. Rest of the space may contain arbitrary number.
}

//
// Take the absolute value of the input value with respect to Zero_16 (0x8000)
//
Uint16 abs16(Uint16 input)
{
    if(input < Zero_16){return(Zero_16 - input);}
    else{ return(input - Zero_16);}
}

//
// updateLimit - This function update the upper and lower limit
//
void updateLimit(Uint32* max, Uint32* min, Uint32 val)
{
    if(val > *max){*max = val;}
    else{if(val < *min){*min = val;}}
}

//
// smaller - This function output the smaller number
//
Uint32 smaller(Uint32 x, Uint32 y)
{
    if(x > y){return y;}
    else{return x;}
}

//
// bigger - This function output the bigger number
//
Uint32 bigger(Uint32 x, Uint32 y)
{
    if(x > y){return x;}
    else{return y;}
}


//
// reachLimit - This function deduce if the specific output channel at its limit
//
bool reachLimit(char channel)
{
    if(channel == 0x20){return ((last20bit >= 0x000FFFFF) || (last20bit == 0x00000000));}
    else{return ((lastdac[channel - 16] >= 0x0000FFFF) || (lastdac[channel - 16] == 0x00000000));}
}

//
// current_output - This function check current output of a specific channel
//
Uint32 current_output(char channel)
{
    if(channel == 0x20)
    {
        return last20bit;          // 20bit DAC last output
    }
    else
    {
        return lastdac[0x0F & channel];   // DAC last output
    }
}

//
// output_limit - This function an output channel upper limit
//
Uint32 output_limit(char channel)
{
    if(channel == 0x20)
    {
        return 0x000FFFFF;  // 20bit DAC upper limit
    }
    else
    {
        return 0x0000FFFF;  // DAC upper limit
    }
}

//
// floatT - Test float number calculation time
//
void floatT()
{
    Uint16 i, n, a;
    float x;
    n = combine(serialIn(2));
    a = combine(serialIn(2));

    serialOut(split(Start, 1));         // Send out start command
    while(true)
    {
        GPIO_WritePin(DITHER_0, 0);
        for(i = 0; i < n; i++)
        {
            x = a / 0.1;
            a = x * 0.1;
        }
        GPIO_WritePin(DITHER_0, 1);
        if(combine(serialIn(1)) == Stop){break;}    // Check stop, other wise continue (require to receive some continue command)
        for(i = 0; i < n; i++)
        {
            x = a / 0.1;
            a = x * 0.1;
        }
        GPIO_WritePin(DITHER_0, 1);
        if(combine(serialIn(1)) == Stop){break;}    // Check stop, other wise continue (require to receive some continue command)
    }
    serialOut(split(Finish, 1));                    // Send out finish command
    serialOut(split(a, 2));                         // Send back data
    digitalO(Dither_0, lastdigital[Dither_0]);      // Restore dither 0 initial state
}

//
// End of file
//

/*
 * Aout.c
 *
 *  Created on: Nov 18, 2020
 *      Author: yaoji
 */

#include "Aout.h"

//
// aOut - This function handle analog out of both DAC and 20bit DAC
//
void aOut(char channel, Uint32 data)
{
    if(channel == 0x20){bit20_W(BIT20_DAC, data);}
    else{dac_W(channel, data);}
}

//
// aOut_DSP - This function handle analog out of both DAC and 20bit DAC on PC command
//
void aOut_DSP()
{
    char channel;
    Uint32 data;
    channel = combine(serialIn(1));         // Receive channel. 20bit DAC channel = 0x20
    data = combine(serialIn(3));            // Receive data
    aOut(channel, data);
}

//
// sOut - This function shift analog output
//
void sOut(char channel, Uint32 step, bool dir)
{
    Uint32 cdata, max, min;
    min = 0;
    cdata = current_output(channel);
    max = output_limit(channel);

    if(dir){aOut(channel, smaller(max, cdata + step));}     // dir = True, shift up
    else{aOut(channel, bigger(min, cdata - step));}         // dir = False, shift down

}

//
// Square - Generate a square wave on PC's command
//
void Square()
{
    char channel;
    Uint32 bias1, bias2;
    Uint16 delay;
    channel = combine(serialIn(1));
    bias1 = combine(serialIn(3));
    bias2 = combine(serialIn(3));
    delay = combine(serialIn(2));

    serialOut(split(Start, 1));         // Send start command
    while(true)
    {
        aOut(channel, bias1);
        DELAY_US(delay);
        // Check if PC requies to stop
        if (serialCheck() == Stop)
        {
            break;
        }

        aOut(channel, bias2);
        DELAY_US(delay);
        // Check if PC requies to stop
        if (serialCheck() == Stop)
        {
            break;
        }
    }
    serialOut(split(Stop, 1));      // Send Finish command
    serialOut(split(current_output(channel), 3));   // Output the current value of output channel
}

//
// End of file
//

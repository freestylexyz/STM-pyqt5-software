/*
 * RampDiag.c
 *
 *  Created on: Nov 23, 2020
 *      Author: yaoji
 */

#include "RampDiag.h"

//
// rampDiag - This function ramp to two channels diagonally.
// NOT USE WITH 20BITDAC
//
void rampDiag(char channels, char channell, Uint16 targets, Uint16 targetl, Uint16 stepnum, Uint16 delay, Uint16 limit, bool checkstop)
{

    bool condition, stopped, protect_triggered, crashprotection;
    bool dirs, dirl;
    Uint16 ranges, rangel, currentl, currents;
    Uint16 i, startl, starts;
    float steps, stepl;

    condition = true;               // Initialize loop condition to true
    stopped = false;                // Initialize stop flag to false
    protect_triggered = false;      // Initialize protect triggered flag to false
    crashprotection = (limit != 0);     // Deduce crash protection enabled based on feedback Z limit
    currents = lastdac[channels - 16];  // Obtain the starting point of short channel
    currentl = lastdac[channell - 16];  // Obtain the starting point of long channel
    ranges = bigger(targets, currents) - smaller(targets, currents);    // Short channel range
    rangel = bigger(targetl, currentl) - smaller(targetl, currentl);    // Long channel range

    dirl = (targetl > currentl);    // Long channel ramp direction
    dirs = (targets > currents);    // Short channel ramp direction
    stepl = rangel / stepnum;       // Long channel step size
    steps = ranges / stepnum;       // Short channel step size

    startl = currentl;
    starts = currents;

    serialOut(split(Start, 1));     // Send out start command
    // Ramp loop
    for (i = 0; i < stepnum; i++)
    {
        dac_W(channell, currentl);  // Output long channel
        dac_W(channels, currents);  // Output short channel

        // Deduce the next cycle output value based on ramp direction
        if(dirl){currentl = (startl + (i * stepl));}
        else{currentl = (startl - (i * stepl));}
        if(dirs){currents = (starts + (i * steps));}
        else{currents = (starts - (i * steps));}

        // Ramp protection and update protect triggered flag
        protect_triggered = protect_triggered || protectRamp(crashprotection, limit, &condition);
        if(checkstop)  // Check stop
        {
            if (serialCheck() == Stop){stopped = true;} // Update stop flag
            else{serialOut(split(Ongoing, 1));}         // Tell PC one step is done
        }
        if(stopped || (! condition)){break;}            // If stop or protection turn the condition bad, break the loop

        DELAY_US(delay);
    }

    serialOut(split(Finish, 1));        // Send out Finish command only
    if(!stopped)    // If not stopped, output target value to make sure it reaches target
    {
        dac_W(channell, targetl);
        dac_W(channels, targets);
    }

    if(protect_triggered)
    {
        serialOut(split(Finish + 0x80, 1)); // Send out Finish command with protect triggered flag (MSB)
        serialOut(split(lastdac[Zoffc - 16], 2));
    }
    else
    {
        serialOut(split(Finish, 1));        // Send out Finish command only
    }

    serialOut(split(lastdac[channels - 16], 2));
    serialOut(split(lastdac[channell - 16], 2));
}

//
// rampDiag_DSP - This function ramp to two channels diagonally on PC command.
// NOT USE WITH 20BITDAC
//
void rampDiag_DSP()
{
    bool checkstop;
    char channels, channell;
    Uint16 targets, targetl;
    Uint16 stepnum, delay, limit;

    channels = combine(serialIn(1));     // Acquire short channel data
    channell = combine(serialIn(1));     // Acquire long channel data
    targets = combine(serialIn(2));      // Acquire short channel target data
    targetl = combine(serialIn(2));      // Acquire long channel target data
    stepnum = combine(serialIn(2));      // Acquire step number data
    delay = combine(serialIn(2));        // Acquire delay data
    limit = combine(serialIn(2));        // Acquire limit data
    checkstop = limit > 0x8000;          // The MSB of limit is check stop flag
    limit = limit & 0x7FFF;              // Rest of bits are real limit data
    rampDiag(channels, channell, targets, targetl, stepnum, delay, limit, checkstop);
}

//
// End of file
//

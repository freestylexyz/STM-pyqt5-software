/*
 * Ramp.c
 *
 *  Created on: Nov 18, 2020
 *      Author: yaoji
 */

#include "Ramp.h"

//
// rampCore - This function handles the core part of a positive ramp function. Output before using rampCore
// Return in range flag
//
bool rampCore(bool dir, char channel, Uint32 target, Uint32 step, Uint32* judge, Uint32* value, bool* condition)
{
    Uint32 max, min;
    bool inrange = true;
    min = 0x00000000;
    if(channel == 0x20){max = 0x000FFFFF;}
    else{max = 0x0000FFFF;}

    // Update value and condition based on ramping direction
    if(dir) // Ramping up
    {

        if(*value >= (max - step))
        {inrange = false;}    // Out of range
        *value += step;
        *condition = (*judge <= target) && inrange;   // Stop if reach target or out of range
    }
    else    // Ramping down
    {
        if(*value <= (min + step))
        {inrange = false;}    // Out of range
        *value -= step;
        *condition = (*judge >= target) && inrange;   // Stop if reach target or out of range
    }
    return inrange;
}


//
// rampTo - Ramp a DAC channel to specific value.
//
void rampTo(char channel, Uint32 target, Uint32 step, Uint16 delay, Uint16 limit, bool checkstop)
{
    Uint32 i;
    bool protect_triggered, crashprotection, stopped, condition, dir;

    stopped = false;                // Initial stopped flag
    protect_triggered = false;      // Initial protection triggered flag
    crashprotection = (limit != 0); // Set limit to 0 to disable crash protection
    i = current_output(channel);     // Initial i to last output value
    dir = i < target;               // Figure out ramp direction
    condition = true;               // Initial loop condition to true

    serialOut(split(Start, 1));     // Send out start command
    while(condition)
    {
        aOut(channel, i);           // Output current data
        DELAY_US(delay);            // Delay

        protect_triggered = protect_triggered || protectRamp(crashprotection, limit, &condition);

        // Update Z offset fine value and condition based on direction for next loop
        rampCore(dir, channel, target, step, &i, &i, &condition);

        if(checkstop)  // Check stop
        {
            if (serialCheck() == Stop)
            {
                stopped = true;
                break;
            }
            else
            {
                serialOut(split(Ongoing, 1));               // Tell PC one step is done
            }
        }
    }
    if(!stopped){aOut(channel, target);}    // If not stopped, output target value to make sure it reaches target

    if(protect_triggered)
    {
        serialOut(split(Finish + 0x80, 1)); // Send out Finish command with protect triggered flag (MSB)
        serialOut(split(lastdac[Zoffc - 16], 2));
    }
    else
    {
        serialOut(split(Finish, 1));        // Send out Finish command only
    }
    serialOut(split(current_output(channel), 3));   // Output the current value of output channel
}

//
// rampTo_S - Ramp a DAC channel to specific value without any feature. Internal use only
//
void rampTo_S(char channel, Uint32 target, Uint32 step, Uint16 delay)
{
    Uint32 i;
    bool dir, condition;
    i = current_output(channel);     // Initial i to last output value
    dir = i < target;               // Figure out ramp direction
    condition = true;               // Initial loop condition to true

    while(condition)
    {
        aOut(channel, i);           // Output current data
        DELAY_US(delay);            // Delay

        // Update Z offset fine value and condition based on direction for next loop
        rampCore(dir, channel, target, step, &i, &i, &condition);
    }
    aOut(channel, target);          // Output target value
}

//
// rampStep - Ramp a DAC by a fixed amount
//
void rampStep(char channel, Uint32 target, Uint32 step, Uint16 delay, bool dir)
{
    bool condition;
    Uint32 i, max;
    i = current_output(channel);            // Initial i to last output value
    max = output_limit(channel);            // Obtain output limit

    if(dir){target = smaller(max, i + target);} // dir = True, shift up
    else{target = bigger(0, i - target);}        // dir = False, shift down
    condition = true;                       // Initial loop condition to true

    while(condition)
    {
        aOut(channel, i);           // Output current data
        DELAY_US(delay);            // Delay

        // Update Z offset fine value and condition based on direction for next loop
        rampCore(dir, channel, target, step, &i, &i, &condition);
    }
    aOut(channel, target);          // Output target value
}


//
// rampTo_DSP - Ramp a DAC channel to specific value on PC's command
//
void rampTo_DSP()
{
    bool checkstop;
    char channel;
    Uint32 target;
    Uint16 step, delay, limit;

    channel = combine(serialIn(1));     // Acquire channel data
    target = combine(serialIn(3));      // Acquire target data
    step = combine(serialIn(3));        // Acquire step data
    delay = combine(serialIn(2));       // Acquire delay data
    limit = combine(serialIn(2));       // Acquire limit data
    checkstop = limit >= 0x8000;        // The MSB of limit is check stop flag
    limit = limit & 0x7FFF;             // Rest of bits are real limit data

    // Ramp
    rampTo(channel, target, step, delay, limit, checkstop);
}

//
// rampMeasure - Ramp a DAC channel to specific value and read with an ADC channel.
// No protection and always check stop. Return if measurement stopped
//
bool rampMeasure(char channel, Uint32 target, Uint32 step, Uint16 move_delay, Uint16 measure_delay)
{
    bool stopped, condition, dir;
    Uint32 i;

    stopped = false;                // Initial stopped flag
    i = current_output(channel); // Initial i to last output value
    dir = i < target;               // Figure out ramp direction
    condition = true;               // Initial loop condition to true

    serialOut(split(Start, 1));     // Send out start command
    while(condition)
    {
        rampTo_S(channel, i, 10, move_delay);   // Ramp to next step
        DELAY_US(measure_delay);                // Delay before measurement
        pointSeq(0);                       // Perform measurement sequence

        // Update Z offset fine value and condition based on direction for next loop
        rampCore(dir, channel, target, step, &i, &i, &condition);

        if (serialCheck() == Stop)
        {
            stopped = true;
            break;
        }
    }
    // Dump the last point (target)

    serialOut(split(Finish, 1));        // Send out Finish command only
    if(stopped)                         // If stopped
    {
        serialOut(split(0xAA55A55A, 4));// Send out a stop sequence to distinguish with remaining data
    }
    serialOut(split(current_output(channel), 3));   // Serial out current output of this channel
    return stopped;
}

//
// rampMeasure_DSP - Ramp a DAC channel to specific value and read with an ADC channel on PC command.
//
void rampMeasure_DSP()
{
    char channel;
    Uint32 target, step;
    Uint16 move_delay, measure_delay;

    channel = combine(serialIn(1));         // Acquire channel data
    target = combine(serialIn(3));          // Acquire target data
    step = combine(serialIn(3));            // Acquire step data
    move_delay = combine(serialIn(2));      // Acquire move delay data
    measure_delay = combine(serialIn(2));   // Acquire measure delay data
    setup_pointSeq();

    // Ramp and measure
    rampMeasure(channel, target, step, move_delay, measure_delay);
}


//
// End of file
//

/*
 * TipApproach.c
 *
 *  Created on: Nov 22, 2020
 *      Author: yaoji
 */

#include "TipApproach.h"

//
// parabSlide - This function ramp an output channel with a parabola function and end with high speed
// No intend to use this 20bitDAC channel
//
void parabSlide(char channel, Uint32 target, Uint32 g)
{
    Uint32 slope, current_temp, currentvalue, target_temp;
    bool dir, condition;
    currentvalue = lastdac[channel - 16];  // Obtain the current value
    dir = currentvalue < target;           // Figure out direction
    condition = true;                      // Initial condition to start loop
    slope = 0;                             // Initial slope

    current_temp = currentvalue << 5;
    target_temp = target << 5;
    // Parabola ramp to the target
    while(condition)
    {
        dac_W(channel, currentvalue);       // DAC output
        slope += g;                         // Increase speed

        // Figure out condition and output value for next loop based on direction
        if(dir)
        {
            condition = (current_temp <= (target_temp - slope));
            current_temp += slope;
        }
        else
        {
            condition = (current_temp >= (target_temp + slope));
            current_temp -= slope;
        }
        currentvalue = current_temp >> 5;
//        DELAY_US(2);        // Delay 2 us
    }
    dac_W(channel, target); // Make sure end at the target
}

//
// parabSteady - This function ramp an output channel with two parabola functions and end with near 0 speed
// No intend to use this 20bitDAC channel
//
void parabSteady(char channel, Uint32 target, Uint32 g)
{
    Uint32 currentvalue, slope, halftarget;
    Uint32 half_temp, current_temp, target_temp;
    bool dir, condition;

    currentvalue = lastdac[channel - 16];       // Obtain the current value
    dir = currentvalue < target;                // Figure out direction
    condition = true;                           // Initial condition to start loop
    slope = 0;                                  // Initial slope
    halftarget = (target + currentvalue) / 2;   // Calculate the half way target

    current_temp = currentvalue << 5;
    target_temp = target << 5;
    half_temp = halftarget << 5;

    // Parabola ramp to half way to the target
    while(condition)
    {
        dac_W(channel, currentvalue);       // DAC output
        slope += g;                         // Increase speed

        // Figure out condition and output value for next loop based on direction
        if(dir)
        {
            condition = (current_temp <= (half_temp - slope));
            current_temp += slope;
        }
        else
        {
            condition = (current_temp >= (half_temp + slope));
            current_temp -= slope;
        }
//        DELAY_US(2);
        currentvalue = current_temp >> 5;
    }
//    dac_W(channel, halftarget);    // Delay 2 us
    current_temp = half_temp;      // Make sure end at the half target
    condition = true;              // Re-initialize loop condition

    // Invert parabola ramp to the final target
    while(condition)
    {
        dac_W(channel, currentvalue);       // DAC output
        slope = bigger(g, slope - g);       // Decrease speed and limit the smallest value to g

        // Figure out condition and output value for next loop based on direction
        if(dir)
        {
            condition = (current_temp <= (target_temp - slope));
            current_temp += slope;
        }
        else
        {
            condition = (current_temp >= (target_temp + slope));
            current_temp -= slope;
        }
//        DELAY_US(2);        // Delay 2 us
        currentvalue = current_temp >> 5;
    }
    dac_W(channel, target); // Make sure end at the target
}


//
// babyStep -This function does baby step
// need to fix babyg after testing
//
bool babyStep(Uint16 stepSize, Uint16 limit, Uint16 babyg)
{
    Uint16 i, currentzouter;
    bool condition, nottunnel;

    currentzouter = lastdac[Zouter - 16];    // Obtain the current value
    DELAY_US(8000);                         // Wait for 8 ms

    i = adc_CNV_N(Preamp, 10);      // Read premap voltage
    if (abs16(i) > limit)           // If reading larger than the minimum tunneling limit
    {
        DELAY_US(5000);             // Wait for 5 ms
        i = adc_CNV_N(Preamp, 10);  // Read current again.
    }
    nottunnel = (abs16(i) < limit); // If reading smaller than the minimum tunneling limit, not tunneled
    condition = nottunnel;          // Initial the loop condition to not tunneled flag
    while(condition)
    {
        dac_W(Zouter, currentzouter);   // DAC output
        DELAY_US(20);                   // Delay 20 us
        i = adc_CNV_N(Preamp, 10);      // Read premap voltage
        if (abs16(i) > limit)           // If reading larger than the minimum tunneling limit
        {
            DELAY_US(5000);             // Wait for 5 ms
            i = adc_CNV_N(Preamp, 10);  // Read current again.
        }
        nottunnel = (abs16(i) < limit); // If reading smaller than the minimum tunneling limit, not tunneled
        condition = ( nottunnel && (currentzouter < (0xFFFF - stepSize)));  // Loop condition is decided by not tunnel flag and reach the maximum output together
        currentzouter += stepSize;      // Value to be output for next cycle
    }
    if(nottunnel){dac_W(Zouter, 0xFFFF);}   // If not tunnel, max out the output
    parabSteady(Zouter, Zero_16, babyg);     // Use steady parabola ramp back to 0
    return nottunnel;
}

//
// oneGiant - This function perform one step
//
void oneGiant(char channel, Uint16 x, Uint16 z, Uint16 delay, Uint16 g)
{
    parabSlide(channel, x, g);          // Move x with slide parabola ramp, so that sample can slide a little bit at the end
    dac_W(Zouter, z);                   // Piezo contract, and sample may start falling and continue sliding
    DELAY_US(15);
    dac_W(channel, Zero_16);            // Before sample touch W balls, restore x to starting position
    DELAY_US(delay);                    // Wait a long time, so that sample can fall and stabilize on 3 W balls
    parabSteady(Zouter, Zero_16, g);    // Steady parabola ramp Z outer to original place
}

//
// giantStep - This function does step up, step down and translate on PC command
// need to fix delay1 and gz after testing
//
void giantStep()
{
    char channel;
    Uint16 x, z, delay, g, stepnum, i;

    channel = combine(serialIn(1)); // Serial in 2 bytes to get giant step channel (Xin or Yin)
    x = combine(serialIn(2));       // Serial in 2 bytes to get x step
    z = combine(serialIn(2));       // Serial in 2 bytes to get z step
    delay = combine(serialIn(2));   // Serial in 2 bytes to get long delay for sample dropping
    g = combine(serialIn(1));       // Serial in 1 bytes to get
    stepnum = combine(serialIn(2)); // Serial in 2 bytes to get total step number

    serialOut(split(Start, 1));     // Send out start command
    for(i = 0; i < stepnum; i++)
    {
        DELAY_US(100);                          // Initial delay
        if (serialCheck() == Stop){break;}      // Check stop
        oneGiant(channel, x, z, delay, g);      // One giant step
        serialOut(split(Ongoing, 1));           // Tell PC one cycle is done

    }
    serialOut(split(Finish, 1));    // Send out Finish command only
}

//
// tipApproach - This function does tip approach on PC command
//
void tipApproach()
{
    bool condition;
    Uint16 x, z, delay, g, giant, baby, limit, i;

    x = combine(serialIn(2));       // Serial in 2 bytes to get x step
    z = combine(serialIn(2));       // Serial in 2 bytes to get z step
    delay = combine(serialIn(2));   // Serial in 2 bytes to get long delay for sample dropping
    g = combine(serialIn(1));       // Serial in 1 bytes to get z parabola ramp acceleration
    giant = combine(serialIn(2));   // Serial in 2 bytes to get giant step number for each cycle

    baby = combine(serialIn(2));    // Serial in 2 bytes to get baby step size for ramp
    limit = combine(serialIn(2));   // Serial in 3 bytes to get minimum tunneling current

    condition = babyStep(baby, limit, g); // Initialize the loop condition to if tunneling occurred
    serialOut(split(Start, 1));     // Send out start command
    while(condition)
    {
        // Perform giant step
        for(i = 0; i < giant; i++)
        {
            DELAY_US(100);                          // Initial delay
            oneGiant(Xin, x, z, delay, g);          // One giant step
        }
        if (serialCheck() == Stop){break;}          // Check stop
        DELAY_US(1);
        condition = babyStep(baby, limit, g);       // Decide loop condition based on if tunneled
        serialOut(split(Ongoing, 1));               // Tell PC one cycle is done
    }
    serialOut(split(Finish, 1));    // Send out Finish command only
}

//
// End of file
//

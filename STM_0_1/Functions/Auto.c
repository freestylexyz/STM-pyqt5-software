/*
 * Auto.c
 *
 *  Created on: Nov 19, 2020
 *      Author: yaoji
 */

#include "Auto.h"

//
// zAutoT - This function adjust Z offset so that Z feedback can reach a specific target
//
void zAutoT(Uint16 target, Uint16 step)
{
    bool condition, dir, inrange;
    Uint32 zfeedback, zoffset, delay;

    if(step > 10)
    {
        delay = 16000;          // Delay long time for large step
    }
    else
    {
        delay = 8000;           // Delay short time for small step
    }

    if(lastdac[Zofff -16] != Zero_16)
    {
        dac_W(Zofff, Zero_16);                      // Set Z offset fine to 0, if it is not.
        DELAY_US(delay);                             // Delay
    }

    condition = (lastdigital[2] & 0x01);             // Check if feedback is on, otherwise skip the loop
    zoffset = lastdac[Zoffc - 16];                  // Initial zoffset to current Z offset
    zfeedback = adc_CNV_N(Zout, 50);                // Read Z feedback, measure 50 times
    dir = zfeedback < target;                       // Check ramping direction

    while(condition)
    {
        dac_W(Zoffc, zoffset);                       // Output Z offset coarse
        DELAY_US(delay);                             // Wait delay for the feedback to respond
        zfeedback = adc_CNV_N(Zout, 50);            // Read Z feedback signal average 50 times

        // Update Z offset coarse value and condition based on direction for next loop
        inrange = rampCore(dir, Zoffc, target, step, &zfeedback, &zoffset, &condition);
    }
    if(! inrange)
    {
        if(dir){dac_W(Zoffc, 0xFFFF);}
        else{dac_W(Zoffc, 0);}
    }
}

//
// zAuto0 - Auto adjust Z feedback to 0.
//
void zAuto0()
{
    Uint16 gap;
    char z2gain;

    z2gain = ((lastdigital[12] & 0x01) << 1) | (lastdigital[13] & 0x01);  // Obtain the z2 gain
    gap = abs16(adc_CNV_N(Zout, 50));     // Read Z feedback and the gap between target (0x8000)

    // Deduce the real gap based on Z2 gain.
    // Z offset is always amplified by 10
    // Z feedback can be amplified by 0.1, 1 and 10 based on z2 gain
    // Detail: ADC range is +/- 10.24V while DAC range is +/- 10V. There will be a 1.024 coefficient between Z offset bit and Z feedback bit
    // If other ranges are applied to Zout(ADC) and Zoffset(DAC), this function may work in a funny way.
    switch(z2gain)
    {
    case 0x00:
        gap = gap / 100;    // 1 Z offset bit correspond to 100 Z feedback bit
        break;
    case 0x01:
        gap = gap /10;      // 1 Z offset bit correspond to 10 Z feedback bit
        break;
    case 0x03:
        gap = gap;          // 1 Z offset bit correspond to 1 Z feedback bit
        break;
    default:
        break;
    }

    if(gap > 101){zAutoT(Zero_16, 100);}    // Auto Z feedback to 0 with a big step size, if the gap is big
    if(gap > 11){zAutoT(Zero_16, 10);}      // Auto Z feedback to 0 with a medium step size, if the gap is medium
    zAutoT(Zero_16, 1);                     // Auto Z feedback to 0 with minimum step size to finish
}

//
// zAuto0_DSP - Auto adjust Z feedback to 0 on PC's command
//
void zAuto0_DSP()
{
    zAuto0();
    serialOut(split(lastdac[Zoffc -16], 2));
}

//
// iAutoT - This function adjust Z offset fine so that current can reach a specific target
// Need to specific step. Step size can be decided by the current difference
//
void iAutoT(Uint16 target, Uint16 step)
{
    bool condition, dir, inrange;
    Uint32 i, zoffset;

    target = abs16(target);                      // Absolute target current
    condition = ((lastdigital[2] & 0x01) == 0);  // Check if feedback is off, otherwise skip the loop
    zoffset = lastdac[Zofff - 16];               // Initial zoffset to current Z offset fine
    i = abs16(adc_CNV_N(Preamp, 100));           // Read pre-amp, measure 100 times
    dir = i < target;                            // Check ramping direction

    while(condition)
    {
        dac_W(Zofff, zoffset);              // Output Z offset coarse
        DELAY_US(600);                      // Wait 600 us for the pre-amp to respond (pre-amp rise time is 450 us @ gain 10)
        i = abs16(adc_CNV_N(Preamp, 100));  // Read pre-amp signal average 100 times

        // Update Z offset fine value and condition based on direction for next loop
        inrange = rampCore(dir, Zofff, target, step, &i, &zoffset, &condition);
    }
    if(! inrange)
    {
        if(dir){dac_W(Zofff, 0xFFFF);}
        else{dac_W(Zofff, 0);}
    }
}

//
// iAuto - This function adjust Z offset fine so that current can reach a specific target
// No need to to specific step. DO NOT USE IT FOR LARGE CURRENT DIFFERENCE
//
void iAuto(Uint16 target, bool fast)
{
    if(fast){iAutoT(target, 625);}
    iAutoT(target, 125);
    iAutoT(target, 25);
    iAutoT(target, 5);
    iAutoT(target, 1);
}

//
// iAuto_DSP - This function adjust Z offset fine so that current can reach a specific target on PC's command
// No need to to specific step. DO NOT USE IT FOR LARGE CURRENT DIFFERENCE
//
void iAuto_DSP()
{
    Uint16 target;
    target = combine(serialIn(2));
    iAutoT(target, 625);
    iAutoT(target, 125);
    iAutoT(target, 25);
    iAutoT(target, 5);
    iAutoT(target, 1);
    serialOut(split(lastdac[Zofff -16], 2));
}

//
// End of file
//

/*
 * Protection.c
 *
 *  Created on: Nov 19, 2020
 *      Author: yaoji
 */

#include "Protection.h"

//
// portectRamp - This function protect tip while ramping. Return if protection is triggered
//
bool protectRamp(bool enabled, Uint16 limit, bool* condition)
{
    Uint16 zfeedback;
    bool triggered = false;
    if(enabled && (lastdigital[2] & 0x01) && (!(lastdigital[3] & 0x01)))       // If enabled, feedback is on and retract is off, continue on following
    {
        zfeedback = adc_CNV_N(Zout, 50);        // Read Z feedback signal average 50 times
        if(abs16(zfeedback) > limit)
        {
            triggered = true;
            zAuto0();                           // Adjust Z feedback to zero if out of limit
            *condition &= (!reachLimit(Zoffc)); // Need to stop if Z offset coarse has reached the limit
        }
    }
    return triggered;
}

//
// protectScan - This function protect tip while scanning. . Return if protection is triggered
//
bool protectScan(char flag, Uint16 limit, Uint32* lastmax, Uint32* lastmin, Uint32* dz, bool* condition)
{
    Uint16 zfeedback_1, zfeedback_2, target;
    bool triggered = false;
    if(flag && (lastdigital[2] & 0x01) && (!(lastdigital[3] & 0x01)))       // If enabled, feedback is on and retract is off, continue on following
    {
        zfeedback_1 = adc_CNV_N(Zout, 50);  // Read Z feedback signal average 50 times
        updateLimit(lastmax, lastmin, zfeedback_1 + *dz); // Update lastmax and lastmin
        if(abs16(zfeedback_1) > limit)
        {
            triggered = true;
            switch(flag)
            {
            case 1:     // Stop
                *condition = false;
                break;
            case 2:     // Auto to 0 and continue
                zAuto0();                                   // Adjust Z feedback to zero
                zfeedback_2 = adc_CNV_N(Zout, 50);          // Read Z feedback signal average 50 times
                *dz = (*dz + zfeedback_2 + zfeedback_1);    // Add new change to accumulated delta z
                if(reachLimit(Zoffc)){*condition = false;}   // Stop loop if Z offset coarse reaches limit
                break;
            case 3:     // Auto to previous mid-point and continue
                target = (*lastmax - *lastmin) / 2;          // Obtain the middle point of previous lines

                // Continue if no data overflow issue
                if(target < Zero_16)
                {
                    if(zfeedback_1 > Zero_16){target = Zero_16 + target;}
                    else{target = Zero_16 - target;}
                    zAutoT(target, 20);                         // Adjust Z to the middle point
                    zfeedback_2 = adc_CNV_N(Zout, 50);          // Read Z feedback signal average 50 times
                    *dz = (*dz + zfeedback_1 - zfeedback_2);    // Add new change to accumulated delta z
                    if(reachLimit(Zoffc)){*condition = false;}   // Stop loop if Z offset coarse reaches limit
                }
                else{*condition = false;}
                break;
            default:
                break;
            }
        }
    }
    return triggered;
}

//
// End of file
//

/*
 * Protection.c
 *
 *  Created on: Nov 19, 2020
 *      Author: yaoji
 */

#include "Protection.h"

Uint16 stored_iset;                 // Store I set value for later undo tip protection
Uint16 stored_zoffc;                // Store Z offset coarse for later undo tip protection
bool tip_protect_status = false;    // Tip protection status

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
// protectScan - This function protect tip while scanning. Return if protection is triggered
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
                *dz = (*dz + zfeedback_1 - zfeedback_2);    // Add new change to accumulated delta z
                if(reachLimit(Zoffc)){*condition = false;}   // Stop loop if Z offset coarse reaches limit
                break;
            case 3:     // Auto to previous mid-point and continue
                target = ((*lastmax + *lastmin) / 2) - 0x800000;          // Obtain the middle point of previous lines

                // Continue if no data overflow issue
                if(target < 0xffff)
                {
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
// protectTip - This function protect sharp tip during scan, requires knowing the scan area
//
void protectTip(bool enabled, Uint16 data, bool unprotect)
{
    // Do tip protection if enabled
    if(enabled && (unprotect == tip_protect_status))
    {
        if(unprotect)
        {
            if(lastdigital[2] == 0)
            {
                rampTo_S(Zoffc, stored_zoffc, 5, 5);    // Ramp Z offset coarse back to original condition
                DELAY_US(2000);                         // Delay 2ms to let feedback respond
            }
            else
            {
                rampTo_S(Iset, stored_iset, 10, 1);     // Ramp I set back to original condition
                DELAY_US(10000);                        // Delay 10ms to let feedback respond
            }
            tip_protect_status = false;         // Toggle tip protection flag
        }
        else
        {
            stored_iset = current_output(Iset);     // Store current I set output
            stored_zoffc = current_output(Zoffc);   // Store current Z offset fine output
            if(lastdigital[2] == 0)
            {
                rampStep(Zoffc, data, 5, 5, false); // Shift ramp Z offset coarse
                DELAY_US(2000);                     // Delay 2ms to let feedback respond
            }
            else
            {
                rampTo_S(Iset, data, 10, 1);    // Ramp I set to a safer condition
                DELAY_US(10000);                // Delay 10ms to let feedback respond
            }
            tip_protect_status = true;         // Toggle tip protection flag
        }
    }
}

//
// protectTip - This function protect sharp tip during scan, requires knowing the scan area on PC's command
//
void protectTip_DSP()
{
    bool unprotect;
    Uint16 data;
    data = combine(serialIn(2));                // Acquire retract bits data
    unprotect = (combine(serialIn(1)) == 0);    // Acquire unprotect flag

    // Execute protect tip
    protectTip(true, data, unprotect);
}


//
// End of file
//

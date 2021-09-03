/*
 * Protection.c
 *
 *  Created on: Nov 19, 2020
 *      Author: yaoji
 */

#include "Protection.h"

Uint16 stored_zofff;                // Store Z offset fine for later undo tip protection
bool stored_feedback;               // Store feedback for later undo tip protection
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
            zAutoT(Zero_16, 30);                // Adjust Z feedback to zero if out of limit
            *condition &= (!reachLimit(Zoffc)); // Need to stop if Z offset coarse has reached the limit
        }
    }
    return triggered;
}

//
// protectScan - This function protect tip while scanning. Return if protection is triggered
//
bool protectScan(char flag, Uint16 limit, Uint32* lastmax, Uint32* lastmin, Uint32* dz)
{
    Uint16 zfeedback_1, zfeedback_2, target;
    bool stop_scan = false;
    if(flag && (lastdigital[2] & 0x01) && (!(lastdigital[3] & 0x01)))       // If enabled, feedback is on and retract is off, continue on following
    {
        zfeedback_1 = adc_CNV_N(Zout, 50);  // Read Z feedback signal average 50 times
        updateLimit(lastmax, lastmin, zfeedback_1 + *dz); // Update lastmax and lastmin
        if(abs16(zfeedback_1) > limit)
        {
            switch(flag)
            {
            case 1:     // Stop
                stop_scan = true;
                break;
            case 2:     // Auto to 0 and continue
                zAuto0();                                   // Adjust Z feedback to zero
                zfeedback_2 = adc_CNV_N(Zout, 50);          // Read Z feedback signal average 50 times
                *dz = (*dz + zfeedback_1 - zfeedback_2);    // Add new change to accumulated delta z
                if(reachLimit(Zoffc)){stop_scan = true;}   // Stop loop if Z offset coarse reaches limit
                break;
            case 3:     // Auto to previous mid-point and continue
                target = ((*lastmax + *lastmin) / 2) - 0x800000;          // Obtain the middle point of previous lines
                // Continue if no data overflow issue
                if(target < 0xffff)
                {
                    zAutoT(target, 20);                         // Adjust Z to the middle point
                    zfeedback_2 = adc_CNV_N(Zout, 50);          // Read Z feedback signal average 50 times
                    *dz = (*dz + zfeedback_1 - zfeedback_2);    // Add new change to accumulated delta z
                    if(reachLimit(Zoffc)){stop_scan = true;}   // Stop loop if Z offset coarse reaches limit
                }
                else{stop_scan = false;}
                break;
            default:
                break;
            }
        }
    }
    return stop_scan;
}

//
// protectTip - This function protect sharp tip during scan, requires knowing the scan area
//
void protectTip(bool enabled, Uint16 data, bool unprotect, Uint16 target)
{
    // Do tip protection if enabled
    if(enabled && (unprotect == tip_protect_status))
    {
        if(unprotect)
        {
            if(stored_feedback)
            {
                iAutoT(target, 500);                    // Match current
                digitalO(2, 1);                         // Turn feedback on
                DELAY_US(2000);                         // Delay 2ms to let feedback relay respond
                rampTo_S(Zofff, stored_zofff, 500, 5);  // Ramp Z offset fine back to original condition
                DELAY_US(10000);                        // Delay 10ms to let feedback loop respond
            }
            else
            {
                rampTo_S(Zofff, stored_zofff, 500, 5);  // Ramp Z offset fine back to original condition
                DELAY_US(600);                          // Delay 600 us to let preamp respond
            }
            tip_protect_status = false;                 // Toggle tip protection flag
        }
        else
        {
            stored_zofff = current_output(Zofff);   // Store current Z offset fine output
            stored_feedback = lastdigital[2];       // Store feedback
            if(stored_feedback)
            {
                digitalO(2, 0);                     // Turn feedback off
                DELAY_US(2000);                     // Delay 2ms to let feedback relay respond
            }
            rampStep(Zofff, data, 500, 5, false);   // Shift ramp Z offset fine
            DELAY_US(600);                          // Delay 600us to let preamp respond
            tip_protect_status = true;              // Toggle tip protection flag
        }
    }
}

//
// protectTip - This function protect sharp tip during scan, requires knowing the scan area on PC's command
//
void protectTip_DSP()
{
    bool unprotect;
    Uint16 data, target;
    data = combine(serialIn(2));                // Acquire retract bits data
    target = combine(serialIn(2));              // Acquire match current target
    unprotect = (combine(serialIn(1)) == 0);    // Acquire unprotect flag

    // Execute protect tip
    protectTip(true, data, unprotect, target);
}


//
// End of file
//

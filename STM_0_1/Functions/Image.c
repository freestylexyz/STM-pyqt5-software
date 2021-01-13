/*
 * Image.c
 *
 *  Created on: Dec 15, 2020
 *      Author: yaoji
 */

#include "Image.h"

//
// scanLine - This function scan one line
//
void scan()
{
    // Variables sent by PC
    char channel_x, channel_y, flag, scan_protect_flag;
    bool tip_protection, dir_x;
    Uint16 move_delay, measure_delay, stepsize, stepnum, target;
    Uint32 line_delay;

    Uint16 limit, tip_protect_data;

    // Variables used inside function
    Uint16 i, j;
    Uint32 lastmax, lastmin, deltaZ, stored_x, stored_y, current_x, current_y;
    bool condition, stopped;

    // Acquire data from serial
    channel_x = combine(serialIn(1));           // Acquire channel data
    channel_y = combine(serialIn(1));           // Acquire channel data
    flag = combine(serialIn(1));                // Acquire flag data
    stepsize = combine(serialIn(2));            // Acquire step size data
    stepnum = combine(serialIn(2));             // Acquire step number data
    move_delay = combine(serialIn(2));          // Acquire move delay data
    measure_delay = combine(serialIn(2));       // Acquire measure delay data
    line_delay = combine(serialIn(2)) * 1000;   // Acquire line delay data (received data is in ms)
    limit = combine(serialIn(2));               // Acquire limit data for scan protection
    tip_protect_data = combine(serialIn(2));    // Acquire retract bits data for tip protection
    target = combine(serialIn(2));              // Acquire match current target
    setup_pointSeq();                           // Acquire point sequence

    // Split flag data
    scan_protect_flag = flag & 0x03;            // Scan protection mode
    tip_protection = ((flag & 0x08) == 0x08);   // Tip protection enabled
    dir_x = ((flag & 0x10) == 0x10);            // X scan direction

    // Variables initialization
    lastmax = 0x80000;                      // Initialize last maximum
    lastmin = 0x80000;                      // Initialize last minimum
    deltaZ = 0x80000;                       // Initialize delta Z
    if(scan_protect_flag == 0){deltaZ = 0;} // Change delta Z to zero if scan protection disabled. Save one byte for Z out

    stored_x = current_output(channel_x);    // Initial channel x to last output value
    stored_y = current_output(channel_y);    // Initial channel y to last output value
    condition = true;                        // Initialize scan protect condition
    stopped = false;                         // Initialize stopped flag

    serialOut(split(Start, 1));     // Send out start command
    for (i = 0; i < stepnum; i++)   // Line loop
    {
        // Figure out target based on direction
        current_y = stored_y + (i * stepsize);
        rampTo_S(channel_y, current_y, 1, move_delay);              // Ramp to next line
        protectTip(tip_protection, tip_protect_data, true, target); // Undo tip protection
        DELAY_US(line_delay);                                       // Line delay
        for (j = 0; j < stepnum; j++)       // Point loop
        {
            // Figure out target based on direction
            if(dir_x){current_x = stored_x + (j * stepsize);}
            else{current_x = stored_x - (j * stepsize);}
            rampTo_S(channel_x, current_x, 1, move_delay);              // Ramp to next step
            DELAY_US(measure_delay);                                    // Measure delay
            pointSeq(deltaZ, ptSeq);                                    // Perform measurement sequence
            protectScan(flag, limit, &lastmax, &lastmin, &deltaZ, &condition);  // Do scan protection
            if (serialCheck() == Stop){stopped = true;}                 // Check stop
            if (stopped || (! condition)){break;}                       // If stopped or scan protection tells not continue, break loop
        }
        protectTip(tip_protection, tip_protect_data, false, target);    // Do tip protection
        if (stopped || (! condition)){break;}                           // If stopped or scan protection tells not continue, break loop
        rampTo_S(channel_x, stored_x, 1, move_delay);                   // Ramp back to starting point
    }
    serialOut(split(Finish, 1));            // Send out Finish command only
    if(stopped)                             // If stopped
    {
        serialOut(split(0xAA55A55A, 4));    // Send out a stop sequence to distinguish with remaining data
    }
    serialOut(split(lastdac[Zoffc - 16], 2));   // Send out Z offset coarse
}

//
// End of file
//

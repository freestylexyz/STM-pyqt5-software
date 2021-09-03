/*
 * Image.c
 *
 *  Created on: Dec 15, 2020
 *      Author: yaoji
 */

#include "Image.h"

//Uint16 sin_table[256];

//
// scan - This function does scan
//
void scan()
{
    // Variables sent by PC
    char channel_x, channel_y, flag, scan_protect_flag;
    bool tip_protection, dir_x, stop_scan;
    Uint16 move_delay, measure_delay, stepsize, stepnum, target;
    Uint32 line_delay;

    Uint16 limit, tip_protect_data;

    // Variables used inside function
    Uint16 i, j;
    Uint32 lastmax, lastmin, deltaZ, stored_x, stored_y, current_x, current_y;
    bool stopped;

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
    lastmax = 0x800000;                      // Initialize last maximum
    lastmin = 0x800000;                      // Initialize last minimum
    deltaZ = 0x800000;                       // Initialize delta Z
    if(scan_protect_flag == 0){deltaZ = 0;} // Change delta Z to zero if scan protection disabled. Save one byte for Z out

    stored_x = current_output(channel_x);    // Initial channel x to last output value
    stored_y = current_output(channel_y);    // Initial channel y to last output value
    stopped = false;                         // Initialize stopped flag

    serialOut(split(Start, 1));     // Send out start command
    for (i = 0; i < stepnum; i++)   // Line loop
    {
        rampTo_S(channel_x, stored_x, 1, move_delay);               // Ramp back to starting point
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
            serialOut(split(Ongoing, 1));                               // Output ongoing command
            DELAY_US(measure_delay);                                    // Measure delay
            pointSeq(deltaZ, ptSeq);                                    // Perform measurement sequence
            stop_scan = protectScan(scan_protect_flag, limit, &lastmax, &lastmin, &deltaZ);  // Do scan protection
            if (serialCheck() == Stop){stopped = true;}                 // Check stop
 //           if (stopped || (! condition)){break;}                       // If stopped or scan protection tells not continue, break loop
            if (stopped || stop_scan ){break;}
        }
        protectTip(tip_protection, tip_protect_data, false, target);    // Do tip protection
 //       if (stopped || (! condition)){break;}                           // If stopped or scan protection tells not continue, break loop
        if (stopped || stop_scan ){break;}
    }
    serialOut(split(Finish, 1));            // Send out Finish command only
//    if(stopped)                             // If stopped
//    {
//        serialOut(split(0xAA55A55A, 4));    // Send out a stop sequence to distinguish with remaining data
//    }
    serialOut(split(lastdac[Zoffc - 16], 2));   // Send out Z offset coarse
}

//
// scan_cc - This function does regular constant current scan with protection
//
//void scan_cc()
//{
//    // Variables sent by PC
//    char flag, scan_protect_flag;
//    bool tip_protection, dir_x;
//    Uint16 move_delay, measure_delay, stepsize, stepnum, target, average;
//    Uint16 limit, tip_protect_data;
//    Uint32 line_delay;
//
//    // Variables used inside function
//    char channel_x, channel_y;
//    Uint16 i, j, zfeedback_1, zfeedback_2;
//    Uint32 lastmax, lastmin, deltaZ, stored_x, stored_y, current_x, current_y, z;
//    bool condition, stopped;
//
//    // Acquire data from serial
//    flag = combine(serialIn(1));                // Acquire flag data
//    stepsize = combine(serialIn(2));            // Acquire step size data
//    stepnum = combine(serialIn(2));             // Acquire step number data
//    move_delay = combine(serialIn(2));          // Acquire move delay data
//    measure_delay = combine(serialIn(2));       // Acquire measure delay data
//    line_delay = combine(serialIn(2)) * 1000;   // Acquire line delay data (received data is in ms)
//    limit = combine(serialIn(2));               // Acquire limit data for scan protection
//    tip_protect_data = combine(serialIn(2));    // Acquire retract bits data for tip protection
//    target = combine(serialIn(2));              // Acquire match current target
//    average = combine(serialIn(2));             // Acquire average number
//
//    // Split flag data
//    scan_protect_flag = flag & 0x03;            // Scan protection mode
//    tip_protection = ((flag & 0x08) == 0x08);   // Tip protection enabled
//    dir_x = ((flag & 0x10) == 0x10);            // X scan direction
//    if((flag & 0x08) == 0x08)
//    {
//        channel_x = Xin;
//        channel_y = Yin;
//    }
//    else
//    {
//        channel_x = Yin;
//        channel_y = Xin;
//    }
//
//    // Variables initialization
//    lastmax = 0x80000;                       // Initialize last maximum
//    lastmin = 0x80000;                       // Initialize last minimum
//    deltaZ = 0x80000;                        // Initialize delta Z
//    stored_x = current_output(channel_x);    // Initial channel x to last output value
//    stored_y = current_output(channel_y);    // Initial channel y to last output value
//    condition = true;                        // Initialize scan protect condition
//    stopped = false;                         // Initialize stopped flag
//
//    // Start scan
//    serialOut(split(Start, 1));             // Send out start command
//    for (i = 0; i < stepnum; i++)           // Line loop
//    {
//        current_y = stored_y + (i * stepsize);                      // Figure out line target based on direction
//        rampTo_S(channel_x, stored_x, 1, move_delay);               // Ramp back to point starting position
//        rampTo_S(channel_y, current_y, 1, move_delay);              // Ramp to next line
//        protectTip(tip_protection, tip_protect_data, true, target); // Undo tip protection
//        DELAY_US(line_delay);                                       // Line delay
//        for (j = 0; j < stepnum; j++)       // Point loop
//        {
//            // Figure out target based on direction
//            if(dir_x){current_x = stored_x + (j * stepsize);}
//            else{current_x = stored_x - (j * stepsize);}
//            rampTo_S(channel_x, current_x, 1, move_delay);                          // Ramp to next step
//            serialOut(split(Ongoing, 1));                                           // Output ongoing command
//            DELAY_US(measure_delay);                                                // Measure delay
//            zfeedback_1 = adc_CNV_N(Zout, average);                                 // Obtain N times averaged
//            z = zfeedback_1 + deltaZ;                                               // Calculate compensated Z
//            serialOut(split(z, 3));                                                 // Output Zout data
//            if(scan_protect_flag && (lastdigital[2] & 0x01) && (!(lastdigital[3] & 0x01)))// If enabled, feedback is on and retract is off, continue on following
//            {
//                updateLimit(&lastmax, &lastmin, zfeedback_1 + deltaZ);       // Update lastmax and lastmin
//                if(abs16(zfeedback_1) > limit)
//                {
//                    switch(flag)
//                    {
//                    case 1:     // Stop
//                        condition = false;
//                        break;
//                    case 2:     // Auto to 0 and continue
//                        zAuto0();                                           // Adjust Z feedback to zero
//                        break;
//                    case 3:     // Auto to previous mid-point and continue
//                        target = ((lastmax + lastmin) / 2) - 0x800000;      // Obtain the middle point of previous lines
//                        if(target < 0xffff){zAutoT(target, 20);}            // Adjust Z to the middle point, if no data overflow issue
//                        else{condition = false;}
//                        break;
//                    default:
//                        break;
//                    }
//                    if((flag > 1) && condition)
//                    {
//                        zfeedback_2 = adc_CNV_N(Zout, 50);              // Read Z feedback signal average 50 times
//                        deltaZ = (deltaZ + zfeedback_1 - zfeedback_2);  // Add new change to accumulated delta z
//                        if(reachLimit(Zoffc)){condition = false;}       // Stop loop if Z offset coarse reaches limit
//                    }
//                }
//            }
//            if (serialCheck() == Stop){stopped = true;}                 // Check stop
//            if (stopped || (! condition)){break;}                       // If stopped or scan protection tells not continue, break loop
//        }
//        protectTip(tip_protection, tip_protect_data, false, target);    // Do tip protection
//        if (stopped || (! condition)){break;}                           // If stopped or scan protection tells not continue, break loop
//    }
//    serialOut(split(Finish, 1));            // Send out Finish command only
//    serialOut(split(lastdac[Zoffc - 16], 2));   // Send out Z offset coarse
//}

//
// scan_ex - This function does experimental scan with protection
//
//void scan_ex()
//{
//    // Variables sent by PC
//    Uint16 delay_p, stepnum_p, stepnum_l, stepsize, angle_index;
//    Uint32 delay_l;
//    float angle_i, angle_j;
//
//    // Variables used inside function
//    char channel_p, channel_l;
//    Uint16 i, j;
//    Uint32 stored_p, stored_l, current_p, current_l;
//    bool stopped;
//
//    // Acquire data from serial
//    channel_p = combine(serialIn(1));           // Acquire channel data
//    channel_l = combine(serialIn(1));           // Acquire channel data
//    angle_index = combine(serialIn(1));           // Acquire angle data
//    stepsize = combine(serialIn(2));            // Acquire step size data
//    stepnum_p = combine(serialIn(2));             // Acquire step number data
//    stepnum_l = combine(serialIn(2));             // Acquire step number data
//    delay_p = combine(serialIn(2));       // Acquire measure delay data
//    delay_l = combine(serialIn(2)) * 1000;   // Acquire line delay data (received data is in ms)
//    setup_pointSeq();                           // Acquire point sequence
//
//    angle_i = (float)sin_table[angle_index] / (float)65535;
//    angle_j = (float)sin_table[255 - angle_index] / (float)65535;
//
//    // Variables initialization
//    stored_p = current_output(channel_p);    // Initial channel x to last output value
//    stored_l = current_output(channel_l);    // Initial channel y to last output value
//    stopped = false;                         // Initialize stopped flag
//
//    // Start scan
//    serialOut(split(Start, 1));             // Send out start command
//    for (i = 0; i < stepnum_l; i++)           // Line loop
//    {
//        current_p = int(stored_p + (i * stepsize * angle_i));
//        current_l = int(stored_l + (i * stepsize * angle_j)); // Figure out line target based on direction
//        rampTo_S(channel_l, current_p, stepsize, delay_p);       // Ramp back to point starting position
//        rampTo_S(channel_p, current_p, stepsize, delay_p);       // Ramp back to point starting position
//        DELAY_US(delay_l);                                      // Line delay
//        for (j = 0; j < stepnum_p; j++)       // Point loop
//        {
//            current_p = int(stored_p + (j * stepsize * angle_j) + (i * stepsize * angle_i));
//            current_l = int(stored_l + (j * stepsize * angle_i) + (i * stepsize * angle_j)); // Figure out line target based on direction
//            aOut(channel_p, current_p)                          // Move to next step
//            aOut(channel_l, current_l)
//            serialOut(split(Ongoing, 1));                       // Output ongoing command
//            pointSeq(0, ptSeq);                                 // Perform measurement sequence
//            if (serialCheck() == Stop)
//            {
//                stopped = true;
//                break;
//            }                 // Check stop
//        }
//        if (stopped){break;}                           // If stopped or scan protection tells not continue, break loop
//    }
//    serialOut(split(Finish, 1));            // Send out Finish command only
//}
//
// End of file
//

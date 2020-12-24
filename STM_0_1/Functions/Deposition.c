/*
 * Deposition.c
 *
 *  Created on: Dec 16, 2020
 *      Author: yaoji
 */

#include "Deposition.h"

//
// deposition - This function does deposition
//
void deposition()
{
    // Variables sent by PC
    char read_flag;
    Uint16 read_delay, read_delay2, read_num, average, limit, stopNum, read_ch;

    // Variables used inside function
    bool detected;

    // Acquire data from serial
    read_ch = combine(serialIn(1)) << 8;
    read_flag = combine(serialIn(1));
    read_delay = combine(serialIn(2));
    read_delay2 = combine(serialIn(2));
    read_num = combine(serialIn(2));
    average = combine(serialIn(2));
    limit = combine(serialIn(2));
    stopNum = combine(serialIn(2));
    setup_depSeq();

    // Split data
    read_flag = (read_flag & 0x03);              // 1st and 2nd bits

    // Execution
    serialOut(split(Start, 1));     // Send out start command

    pointSeq(0, depSeqF);           // Do forward sequence
    DELAY_US(read_delay);           // Delay some time to let system stabilize (DAC settling ~ 10us, piezo response ~ 100us, pre-amp response ~ 1ms and feedback loop ~10ms)
    switch(read_flag)
    {
    case 1:     // Continuous read, until told stop
        OSC_C(read_ch, average, read_delay2);
        break;
    case 2:     // Read until a change has been detected
        read_num = OCS_U(read_ch, average, read_delay2, limit, stopNum, & detected);
        break;
    case 3:     // Read fixed number of data points
        OSC_N(read_ch, read_num, average, read_delay2);
        break;
    default:
        break;
    }
    pointSeq(0, depSeqB);               // Do backward sequence

    serialOut(split(Finish, 1));        // Send out finish command
    serialOut(split(0xAA55A55A, 4));    // Send out a stop sequence to distinguish with remaining data

    // Send out data array if reading with N sample or read until
    if((read_flag & 0x02) == 0x02)
    {
        serialOut(split(read_num, 2));      // Tell PC how many data to be expected
        serialOutArray(ldata, read_num);    // Send out data
    }
}


//
// End of file
//

/*
 * Sequence.c
 *
 *  Created on: Nov 18, 2020
 *      Author: yaoji
 */

#include "Sequence.h"

sc ptSeq;       // Point sequence storage
sc depSeqF;     // Forward deposition sequence
sc depSeqB;     // Backward deposition sequence

//
// pointSeq - This function execute a series of operation for a single data point
//
void pointSeq(Uint32 dz, sc seq)
{
    Uint16 i, inchannel;
    Uint32 data, rdata, step;
    bool dir;
    char outchannel, flag;

    for(i = 0; i < seq.seqnum; i++)
    {
        flag = seq.commandlist[i] & 0xE0;     // Use the first 3 bits to determine the command type
        switch(flag)
        {
        case 0x00:        // Wait
            DELAY_US(seq.datalist[i]);
            break;
        case 0x20:        // Auto I
            data = seq.datalist[i] & 0x0000FFFF;
            iAuto(seq.datalist[i] & 0x0000FFFF, (seq.datalist[i] & 0x80000000) == 0x80000000);
            break;
        case 0x40:        // Digital output
            digitalO(seq.commandlist[i] & 0x07, seq.datalist[i]);
            break;
        case 0x60:        // Shift analog output.
            outchannel = (seq.commandlist[i] & 0x1F) + 16;
            dir = ((seq.datalist[i] & 0x80000000) == 0x80000000);   // Fetch shift direction
            data = seq.datalist[i] & 0x000FFFFF;                    // Fetch shift data
            sOut(outchannel, data, dir);                              // Shift output
            break;
        case 0x80:        // Direct analog output
            aOut((seq.commandlist[i] & 0x1F) + 16, seq.datalist[i]);
            break;
        case 0xA0:        // Ramp analog output
            outchannel = (seq.commandlist[i] & 0x1F) + 16;                    // Fetch output channel
            step = seq.datalist[i] & 0xFFF00000 >> 20;                        // Fetch step data
            rampTo_S(outchannel, seq.datalist[i] & 0x000FFFFF, step, 100);    // Ramp output
            break;
        case 0xC0:        // Read
            inchannel = (seq.commandlist[i] & 0x00FF) << 8;
            rdata = adc_CNV_N(inchannel, seq.datalist[i] & 0x0000FFFF);     // Obtain N times averaged
            if((dz != 0) && (inchannel == Zout))                            // If reading Z feedback and delta z none zero
            {
                rdata = rdata + dz;         // Add delta Z
                serialOut(split(rdata, 3)); // Output 3 byte data through serial
            }
            else            // For all other channels
            {
                serialOut(split(rdata, 2)); // Output 2 byte data through serial
            }
            break;
        case 0xE0:        // Ramp shift
            dir = ((seq.datalist[i] & 0x80000000) == 0x80000000);                 // Fetch shift direction
            outchannel = (seq.commandlist[i] & 0x1F) + 16;                        // Fetch output channel
            step = (seq.datalist[i] & 0x7FF00000) >> 20;                            // Fetch step data
            rampStep(outchannel, seq.datalist[i] & 0x000FFFFF, step, 100, dir);   // Ramp to
            break;
        default:
            break;
        }
    }
}

//
// setup_pointSeq - Set up point sequence command on PC command
//
void setup_pointSeq()
{
    Uint16 i;
    ptSeq.seqnum = combine(serialIn(1));                // Obtain number of sequence
    for(i = 0; i < ptSeq.seqnum; i++)
    {
        ptSeq.commandlist[i] = combine(serialIn(1));    // Obtain command list
        ptSeq.datalist[i] = combine(serialIn(4));       // Obtain data list
    }
}

//
// setup_depSeq - This function setup deposition sequence
//
void setup_depSeq()
{
    Uint16 i, num, b_i;
    Uint32 stored, step;
    char flag, channel;

    num = combine(serialIn(1)); // Obtain number of sequence
    depSeqF.seqnum = num;       // Set up forward sequence number
    depSeqB.seqnum = num;       // Set up back sequence number

    // Set up back sequence command list and data list
    for(i = 0; i < num; i++)
    {
        depSeqF.commandlist[i] = combine(serialIn(1));  // Obtain command list
        depSeqF.datalist[i] = combine(serialIn(4));     // Obtain data list
        flag = depSeqF.commandlist[i] & 0xE0;           // Use the first 3 bits to determine the command type
        b_i = num - i - 1;                              // Backward index
        switch(flag)
        {
//        case 0x20:        // Auto I
//            depSeqB.commandlist[b_i] = 0xA0 || Zoffc;       // Return to original Z offset fine by ramping
//            step = 0x0FF00000;                              // 256 bits step size for ramp back
//            stored = current_output(Zoffc) & 0x000FFFFF;    // Store current Z offset fine status
//            depSeqB.datalist[b_i] = step || stored;
//            break;
        case 0x40:        // Digital output
            depSeqB.commandlist[b_i] = depSeqF.commandlist[i];                  // Copy command
            depSeqB.datalist[b_i] = lastdigital[depSeqF.commandlist[i] & 0x07]; // Current corresponding digital channel status
            break;
        case 0x60:        // Shift analog output.
            channel = depSeqF.commandlist[i] & 0x1F;            // Obtain output channel
            depSeqB.commandlist[b_i] = 0x80 || channel;         // Return to current status with direct analog output
            depSeqB.datalist[b_i] = current_output(channel);    // Current corresponding DAC channel status
            break;
        case 0x80:        // Direct analog output
            depSeqB.commandlist[b_i] = depSeqF.commandlist[i];                      // Copy command
            depSeqB.datalist[b_i] = current_output(depSeqF.commandlist[i] & 0x1F);  // Current corresponding DAC channel status
            break;
        case 0xA0:        // Ramp analog output
            channel = depSeqF.commandlist[i] & 0x1F;            // Obtain output channel
            step = depSeqF.commandlist[i] & 0xFFF00000;         // Obtain ramping step
            stored = current_output(channel) & 0x000FFFFF;      // Store the current corresponding DAC channel status
            depSeqB.commandlist[b_i] = depSeqF.commandlist[i];  // Copy command
            depSeqB.datalist[b_i] = step || stored;             // Patch data
            break;
        case 0xE0:        // Ramp shift
            channel = depSeqF.commandlist[i] & 0x1F;            // Obtain output channel
            step = depSeqF.commandlist[i] & 0x7FF00000;         // Obtain ramping step
            stored = current_output(channel) & 0x000FFFFF;      // Store the current corresponding DAC channel status
            depSeqB.commandlist[b_i] = 0xA0 || channel;         // Return to current status by ramping
            depSeqB.datalist[b_i] = step || stored;             // Patch data
            break;
        default:          // Copy
            depSeqB.commandlist[b_i] = depSeqF.commandlist[i];
            depSeqB.datalist[b_i] = depSeqF.datalist[i];
            break;
        }
    }
}

//
// End of file
//

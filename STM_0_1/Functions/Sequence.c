/*
 * Sequence.c
 *
 *  Created on: Nov 18, 2020
 *      Author: yaoji
 */

#include "Sequence.h"

sc ptSeq;       // Point sequence storage

//
// pointSeq - This function execute a series of operation for a single data point
//
void pointSeq(Uint32 dz)
{
    Uint16 i, flag, step;
    Uint32 data, rdata;
    bool dir;
    char outchannel;

    for(i = 0; i < ptSeq.seqnum; i++)
    {
        flag = ptSeq.commandlist[i] & 0xE000;     // Use the first 3 bits to determine the command type
        switch(flag)
        {
        case 0x0000:        // Wait
            DELAY_US(ptSeq.datalist[i]);
            break;
        case 0x2000:        // Auto I
            iAuto(data);
            break;
        case 0x4000:        // Digital output
            digitalO(ptSeq.commandlist[i] & 0x0007, ptSeq.datalist[i]);
            break;
        case 0x6000:        // Shift analog output.
            outchannel = ptSeq.commandlist[i] & 0x003F;
            dir = ((ptSeq.datalist[i] & 0x00800000) == 0x00800000);   // Fetch shift direction
            data = ptSeq.datalist[i] & 0x000FFFFF;                    // Fetch shift data
            sOut(outchannel, data, dir);                               // Shift output
            break;
        case 0x8000:        // Direct analog output
            aOut(ptSeq.commandlist[i] & 0x003F, ptSeq.datalist[i]);
        case 0xA000:        // Ramp analog output
            outchannel = ptSeq.commandlist[i] & 0x003F;               // Fetch output channel
            step = output_limit(outchannel) / 1000;                 // Max 1000 steps finish a full range is good size
            rampTo_S(outchannel, ptSeq.datalist[i], step, 10);        // Ramp output
            break;
        case 0xC000:        // Read
            rdata = adc_CNV_N(ptSeq.commandlist[i], ptSeq.datalist[i]);     // Obtain N times averaged
            if((dz != 0) && (ptSeq.commandlist[i] == Zout))               // If reading Z feedback and delta z none zero
            {
                rdata = rdata + dz;         // Add delta Z
                serialOut(split(rdata, 3)); // Output 3 byte data through serial
            }
            else            // For all other channels
            {
                serialOut(split(rdata, 2)); // Output 2 byte data through serial
            }
            break;
        case 0xE000:
            dir = ((ptSeq.datalist[i] & 0x00800000) == 0x00800000);   // Fetch shift direction
            outchannel = ptSeq.commandlist[i] & 0x003F;               // Fetch output channel
            step = output_limit(outchannel) / 1000;                 // Max 1000 steps finish a full range is good size
            rampStep(outchannel, ptSeq.datalist[i], step, 10, dir);    // Ramp to
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
    ptSeq.seqnum = combine(serialIn(1));                // obtain number of sequence
    for(i = 0; i < ptSeq.seqnum; i++)
    {
        ptSeq.commandlist[i] = combine(serialIn(2));    // Obtain command list
        ptSeq.datalist[i] = combine(serialIn(3));       // Obtain data list
    }
}

//
// End of file
//

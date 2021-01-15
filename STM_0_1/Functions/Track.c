/*
 * Track.c
 *
 *  Created on: Dec 16, 2020
 *      Author: yaoji
 */

#include "Track.h"

//
// track - This function perform tracking on PC command
//
void track()
{
    // Variables need to be received from PC
    Uint32 tiltx, tilty;
    Uint16 step, in_ch, average, delay, stay_delay;
    bool track_min;
    // Variables used only inside function
    Uint32 best;
    Uint16 x, y;
    char dx, dy, sx, sy, sdata;

    // Receive data from PC
    in_ch = combine(serialIn(1)) << 8;          // Acquire input channel data
    delay = combine(serialIn(2));               // Acquire scan delay data
    stay_delay = combine(serialIn(2));          // Acquire stay delay data
    step = combine(serialIn(2));                // Acquire scan step data
    average = combine(serialIn(2));             // Acquire average number data
    track_min = (combine(serialIn(1)) != 0);    // Acquire track_min data
    tiltx = combine(serialIn(4));               // Acquire plane fit data for x
    tilty = combine(serialIn(4));               // Acquire plane fit data for y

    // Initialize variables
    x = current_output(Xin);
    y = current_output(Xin);

    // Execution
    serialOut(split(Start, 1));     // Send out start command
    while(true)
    {
        // Initialize position to upper left corner
        x = x - step;
        y = y - step;
        // Send to upper left corner
        rampTo_S(Xin, x, 1, 10);
        rampTo_S(Yin, y, 1, 10);
        // "S" shape Scan
        for(dx = 1; dx < 4; dx++)
        {
            rampTo_S(Xin, x + ((dx - 1) * step), 1, 10);            // Send x first
            for(dy = 1; dy < 4; dy++)
            {
                if(dx == 0x02)      // Deal with "S" shape scan
                {
                    rampTo_S(Yin, y + ((3 - dy) * step), 1, 10);    // Send y
                    DELAY_US(delay);                                // Delay
                    updatePos(track_min, in_ch, average, tiltx, tilty, dx, dy, &sx, &sy, &best);    // Measure and update best position
                }
                else
                {
                    rampTo_S(Yin, y + ((dy - 1) * step), 1, 10);
                    DELAY_US(delay);
                    updatePos(track_min, in_ch, average, tiltx, tilty, dx, dy, &sx, &sy, &best);
                }
            }
        }
        // Update the real bits of best position
        x = x + ((sx - 1) * step);
        y = y + ((sy - 1) * step);
        rampTo_S(Xin, x, 1, 10);                            // Send x to best position
        rampTo_S(Yin, y, 1, 10);                            // Send y to best position
        sdata = ((sx & 0x03) << 6) || ((sy & 0x03) << 4);   // Patch data of best position shift, use the 4 MSB to distinguish with finish command
        serialOut(split(sdata, 1));                         // Send out the shift of best position
        DELAY_US(stay_delay);                               // Stay at best station for some time
        if(serialCheck() == Stop){break;}                   //Check stop
    }
    serialOut(split(Finish, 1));                // Send out Finish command
    serialOut(split(current_output(Xin), 2));   // Send out Xin position
    serialOut(split(current_output(Yin), 2));   // Send out Yin position
}

//
// updatePos - This function update position for track. Return if need to update
//
void updatePos(bool track_min, Uint16 in_ch, Uint16 average, Uint32 tiltx, Uint32 tilty, char dx, char dy, char* sx, char* sy,  Uint32* best)
{
    Uint32 rdata;
    bool txs, tys, xs, ys;
    txs = ((tiltx & 0x80000000) == 0x80000000);                         // Figure out tilt x sign
    tys = ((tilty & 0x80000000) == 0x80000000);                         // Figure out tilt y sign
    xs = ((dx & 0x02) == 0x02);                                         // Figure out shift x sign
    ys = ((dy & 0x02) == 0x02);                                         // Figure out shift y sign
    rdata = ((Uint32)adc_CNV_N(in_ch, average)) * ((Uint32)0x7FFF);     // Read data
    if(txs ^ xs){rdata = rdata - ((dx & 0x1) * (tiltx & 0x7FFFFFFF));}  // Correct x based on plane fit
    else{rdata = rdata + ((dx & 0x1) * (tiltx & 0x7FFFFFFF));}
    if(tys ^ ys){rdata = rdata - ((dy & 0x1) * (tilty & 0x7FFFFFFF));}  // Correct y based on plane fit
    else{rdata = rdata + ((dy & 0x1) * (tilty & 0x7FFFFFFF));}

    // Update best value and best position
    if((dx > 1) && (dy > 1))
    {
        if(track_min)
        {
            if(*best > rdata)
            {
                *best = rdata;
                *sx = dx;
                *sy = dy;
            }
        }
        else
        {
            if(*best < rdata)
            {
                *best = rdata;
                *sx = dx;
                *sy = dy;
            }
        }
    }
    else        // Initialize if this is starting point
    {
        *best = rdata;
        *sx = dx;
        *sy = dy;
    }
}

//
// End of file
//

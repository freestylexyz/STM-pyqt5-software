/*
 * Digital.c
 *
 *  Created on: Sep 28, 2020
 *      Author: yaoji
 */

#include "Digital.h"

//
// gain_DSP - This function change X, Y, Z1, Z2 gain on PC's command
//
void gain_DSP()
{
    char command;
    Uint16 ch, d0, d1;
    command = combine(serialIn(1));
    ch = (command & 0x00F0) >> 4;
    d1 = command & 0x0001;
    d0 = (command & 0x0002) >> 1;
    switch(ch)
    {
    case 0:
        GPIO_WritePin(XGAIN_0, d0);
        GPIO_WritePin(XGAIN_1, d1);
        break;
    case 1:
        GPIO_WritePin(YGAIN_0, d0);
        GPIO_WritePin(YGAIN_1, d1);
        break;
    case 2:
        GPIO_WritePin(ZGAIN_0, d0);
        GPIO_WritePin(ZGAIN_1, d1);
        break;
    case 3:
        GPIO_WritePin(ZGAIN_2, d0);
        GPIO_WritePin(ZGAIN_3, d1);
        break;
    default:
        break;
    }
    lastdigital[(ch * 2) + 6] = (char)d0;
    lastdigital[(ch * 2) + 7] = (char)d1;

}

//
// digitalO() - This function handles digital output
//
void digitalO(Uint16 ch, Uint16 data)
{
    switch(ch)
        {
        case 0:
            GPIO_WritePin(DITHER_0, data);
            break;
        case 1:
            GPIO_WritePin(DITHER_1, data);
            break;
        case 2:
            GPIO_WritePin(FEEDBACK, data);
            break;
        case 3:
            GPIO_WritePin(RETRACT, data);
            break;
        case 4:
            GPIO_WritePin(COARSE, data);
            break;
        case 5:
            GPIO_WritePin(ROT, data);
            break;
        default:
            break;
        }
    lastdigital[ch] = (char)data;

}

//
// digitalO_DSP() - This function handles digital output on PC's command
//
void digitalO_DSP()
{
    char command;
    Uint16 ch, data;
    command = combine(serialIn(1));
    ch = (command & 0x0070) >> 4;       // MSB for channel selection
    data = command & 0x0001;            // LSB for data
    digitalO(ch, data);
}

//
// digitalScan_DSP() - This function set up digital outputs for scan or tip approach on PC's command
//
void digitalScan_DSP()
{
    Uint16 val, i;
    val = combine(serialIn(1)) & 0x01;  // val = 0 scan, else tip approach
    GPIO_WritePin(XGAIN_0, val);        // XIN gain 10 (0b11) for tip approach, gain 1 (0b01) for scan
    GPIO_WritePin(XGAIN_1, 1);
    GPIO_WritePin(YGAIN_0, val);        // YIN gain 10 (0b11) for tip approach, gain 1 (0b01) for scan
    GPIO_WritePin(YGAIN_1, 1);
    GPIO_WritePin(ZGAIN_0, val);        // Z1 gain 0.1 (0b11) for tip approach, gain 1 (0b01) for scan
    GPIO_WritePin(ZGAIN_1, 1);
    GPIO_WritePin(ZGAIN_2, val);        // Z2 gain 10 (0b11) for tip approach, gain 1 (0b01) for scan
    GPIO_WritePin(ZGAIN_3, 1);

    GPIO_WritePin(DITHER_0, 0);         // Dither0 off for both cases
    GPIO_WritePin(DITHER_1, 0);         // Dither1 off for both cases
    GPIO_WritePin(FEEDBACK, 1);         // Feedback on for both cases
    GPIO_WritePin(RETRACT, 0);          // Retract off for both cases
    GPIO_WritePin(COARSE, val);         // coarse mode (5/10/15 = 10 * ZOUTER) for tip approach, fine mode (5/10/15 = 0) for scan
    GPIO_WritePin(ROT, 1 - val);        // rotational mode for tip approach, translation mode for scan


    lastdigital[0] = 0;         // Update dither0 global variable
    lastdigital[1] = 0;         // Update dither1 global variable
    lastdigital[2] = 1;         // Update feedback global variable
    lastdigital[3] = 0;         // Update retract global variable
    lastdigital[4] = val;       // Update coarse global variable
    lastdigital[5] = 1 - val;   // Update rot global variable

    // Loop over update gain global variable
    for (i = 0; i < 4; i++ )
    {
        lastdigital[(2 * i) + 6] = val;
        lastdigital[(2 * i) + 7] = 1;
    }
}

//
// End of file
//

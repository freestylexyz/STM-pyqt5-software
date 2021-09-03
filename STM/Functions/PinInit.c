/*
 * PinInit.c
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#include "PinInit.h"

//
// pinInit - This function initial all GPIO pin
//
void pinInit()
{

    // Setup SCI GPIO
    GPIO_SetupPinMux(43, GPIO_MUX_CPU1, 15);    //Set up RX pin
    GPIO_SetupPinOptions(43, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(42, GPIO_MUX_CPU1, 15);    //Set up TX pin
    GPIO_SetupPinOptions(42, GPIO_OUTPUT, GPIO_ASYNC);

    // Setup SPI-A GPIO
    GPIO_SetupPinMux(58, GPIO_MUX_CPU1, 15);    //Set up SPISIMOA
    GPIO_SetupPinOptions(58, GPIO_OUTPUT, GPIO_PULLUP|GPIO_ASYNC);
    GPIO_SetupPinMux(59, GPIO_MUX_CPU1, 15);    //Set up SPISOMIA
    GPIO_SetupPinOptions(59, GPIO_INPUT, GPIO_PULLUP|GPIO_ASYNC);
    GPIO_SetupPinMux(60, GPIO_MUX_CPU1, 15);    //Set up SPICLKA
    GPIO_SetupPinOptions(60, GPIO_OUTPUT, GPIO_PULLUP|GPIO_ASYNC);
//    GPIO_SetupPinMux(61, GPIO_MUX_CPU1, 15);    //Set up SPISTEA
//    GPIO_SetupPinOptions(61, GPIO_OUTPUT, GPIO_PULLUP|GPIO_ASYNC);

    // Setup SPI-B GPIO (could be output for all pins)
    GPIO_SetupPinMux(63, GPIO_MUX_CPU1, 15);    //Set up SPISIMOB
    GPIO_SetupPinOptions(63, GPIO_OUTPUT, GPIO_PULLUP|GPIO_ASYNC);
    GPIO_SetupPinMux(64, GPIO_MUX_CPU1, 15);    //Set up SPISOMIB
    GPIO_SetupPinOptions(64, GPIO_INPUT, GPIO_PULLUP|GPIO_ASYNC);
    GPIO_SetupPinMux(65, GPIO_MUX_CPU1, 15);    //Set up SPICLKB
    GPIO_SetupPinOptions(65, GPIO_OUTPUT, GPIO_PULLUP|GPIO_ASYNC);
//    GPIO_SetupPinMux(66, GPIO_MUX_CPU1, 15);    //Set up SPISTEB
//    GPIO_SetupPinOptions(66, GPIO_OUTPUT, GPIO_PULLUP|GPIO_ASYNC);

    // Setup GPIO output pins for DAC
    outputPinInit(DACCLR, 1);
    outputPinInit(DACRST, 1);
    outputPinInit(CSB, 1);

    // Setup GPIO output pins for ADC
    outputPinInit(ADCRST, 1);
    outputPinInit(CSA, 1);

    // Setup GPIO output pins for 20bit DAC
    outputPinInit(CS20, 1);
    outputPinInit(RST20, 1);
    outputPinInit(CLR20, 1);

    // Pins are initialized for tip approach
    outputPinInit(XGAIN_0, 1);       // Initial gain 10 for XIN
    outputPinInit(XGAIN_1, 1);
    outputPinInit(YGAIN_0, 1);       // Initial gain 10 for YIN
    outputPinInit(YGAIN_1, 1);
    outputPinInit(ZGAIN_0, 1);       // Initial gain 0.1 for Z1
    outputPinInit(ZGAIN_1, 1);
    outputPinInit(ZGAIN_2, 1);       // Initial gain 10 for Z2(HV)
    outputPinInit(ZGAIN_3, 1);

    outputPinInit(COARSE, 1);        // Initial coarse mode (5/10/15 = 10 * ZOUTER)
    outputPinInit(ROT, 0);           // Initial rational mode

    outputPinInit(DITHER_0, 0);      // Initial Dither0 off
    outputPinInit(DITHER_1, 0);      // Initial Dither1 off
    outputPinInit(RETRACT, 0);       // Initial Retract off
    outputPinInit(FEEDBACK, 1);      // Initial feedback on
}

//
// outputPinInit - This function initial corresponding pin to GPIO output ASYNC mode with initial specified desired state
//
void outputPinInit(Uint16 pinNumber, Uint16 val)
{
    GPIO_SetupPinMux(pinNumber, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(pinNumber, GPIO_OUTPUT, GPIO_ASYNC);
    GPIO_WritePin(pinNumber, val);
}

//
// End of file
//


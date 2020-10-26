//
// Included Files
//
#include "F28x_Project.h"
#include "DSP_FUNCTIONS.h"


void outputPinInit(int, int16);
void inputPinInit(int, int16);

void pinInit()
{

    // Setup SCI Receiving Pin
    inputPinInit(43, 15);

    // Setup SCI Transmitting Pin
    outputPinInit(42, 15);


    // Initialize pins for input and  for communication with Computer Interface.
    // Start with input pins

    inputPinInit(IN_DATA_BIT_0, 0);
    inputPinInit(IN_DATA_BIT_1, 0);
    inputPinInit(IN_DATA_BIT_2, 0);
    inputPinInit(IN_DATA_BIT_3, 0);
    inputPinInit(IN_DATA_BIT_4, 0);
    inputPinInit(IN_DATA_BIT_5, 0);
    inputPinInit(IN_DATA_BIT_6, 0);
    inputPinInit(IN_DATA_BIT_7, 0);


    // Output pin setup
    outputPinInit(OUT_DATA_BIT_0, 0);
    outputPinInit(OUT_DATA_BIT_1, 0);
    outputPinInit(OUT_DATA_BIT_2, 0);
    outputPinInit(OUT_DATA_BIT_3, 0);
    outputPinInit(OUT_DATA_BIT_4, 0);
    outputPinInit(OUT_DATA_BIT_5, 0);
    outputPinInit(OUT_DATA_BIT_6, 0);
    outputPinInit(OUT_DATA_BIT_7, 0);


    // Channel Selection Pins
    // Input Channel Select pins
    outputPinInit(CH_0, 0);
    outputPinInit(CH_1, 0);

    // and Output Channel Select pins
    outputPinInit(CH_2, 0);
    outputPinInit(CH_3, 0);
    outputPinInit(CH_4, 0);
    outputPinInit(CH_5, 0);


    // High Res DAC Pins
    outputPinInit(HIGH_RES_DAC_0, 0);
    outputPinInit(HIGH_RES_DAC_1, 0);
    outputPinInit(HIGH_RES_DAC_2, 0);
    outputPinInit(HIGH_RES_DAC_3, 0);
    outputPinInit(HIGH_RES_DAC_4, 0);
    inputPinInit(HIGH_RES_DAC_5, 0);

    //Special Exception for 20 Bit DAC communication latch
    GPIO_WritePin(HIGH_RES_DAC_3, 1);
}

void outputPinInit(int pinNumber, int16 muxValue)
{
    GPIO_SetupPinMux(pinNumber, GPIO_MUX_CPU1, muxValue);
    GPIO_SetupPinOptions(pinNumber, GPIO_OUTPUT, GPIO_ASYNC);
    GPIO_WritePin(pinNumber, 0);
}

void inputPinInit(int pinNumber, int16 muxValue)
{
    GPIO_SetupPinMux(pinNumber, GPIO_MUX_CPU1, muxValue);
    GPIO_SetupPinOptions(pinNumber, GPIO_INPUT, GPIO_PUSHPULL);
}

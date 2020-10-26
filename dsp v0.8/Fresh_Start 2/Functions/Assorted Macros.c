#include "DSP_FUNCTIONS.h"  //DSP specific functions
unsigned int absin(unsigned int);
void clearInFifo(void);
void nullDelay(void);
void bitTest(void);

//Take the absolute value of the input value with respect to INPUT_ZERO (0x8000)
unsigned int absin(unsigned int input)
{
	if(input < INPUT_ZERO)
	{
		//set #1 bit in scan_flags
		return(INPUT_ZERO - input);
	}
	else
	{
		//clear #1 bit in scan_flags
		return(input - INPUT_ZERO);
	}
}

//###########################################################################
// FUNCTION NAME: CLEAR IN-FIFO
// DESCRIPTION: Clear hardware input vector (fifo)
// EXPECTED PARAMETERS: none
// RETURN: none
//###########################################################################
void clearInFifo(void)
{
    unsigned int fifoSize = 0, i = 0;

    fifoSize = FIFO_IN_DATA_AVAILABLE_SIZE;

    for(i=0;i<fifoSize;i++)
    {
        //null read the fifo buffer in order to clear it
        serialIn();
    }

    //points RX FIFO to address 0 again and restart overwritting values
    SciaRegs.SCIFFRX.bit.RXFIFORESET = 0;

    SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;
}

//###########################################################################
// FUNCTION NAME: NULL DELAY
// DESCRIPTION: nullDelay created in order to allow time for the processor to
// remask some GPIO bits when the masking is not done for the whole port (32 bits) at once
// EXPECTED PARAMETERS: none
// RETURN: none
//###########################################################################
void nullDelay(void)
{
    //usually it takes 7 NULL OPERATION CYCLES (NOP) to be able to execute
    //a new bit masking operation but since calling a function takes some
    //cycles to copy registers/memory and branch that means the function can simply be empty
    //          asm("    NOP"); //calls a NULL OPERATION command in assembly language
    //          asm("    NOP");
    //          asm("    NOP");
    //          asm("    NOP");
    //          asm("    NOP");
    //          asm("    NOP");
    //          asm("    NOP");
    //
}





//###########################################################################
// FUNCTION NAME: PINS TEST
// DESCRIPTION: Turn on and off all the output pins during (BLINK_DELAY_US)us
// in order to ease perception of faulty pins
// EXPECTED PARAMETERS: none
// RETURN: none
//###########################################################################
/*
void bitTest(void)
{
    OUT_DATA_BIT_0 = 1;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_0 = 0;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_1 = 1;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_1 = 0;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_2 = 1;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_2 = 0;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_3 = 1;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_3 = 0;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_4 = 1;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_4 = 0;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_5 = 1;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_5 = 0;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_6 = 1;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_6 = 0;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_7 = 1;
    DELAY_US(BLINK_DELAY_US);
    OUT_DATA_BIT_7 = 0;
    DELAY_US(BLINK_DELAY_US);

    CH_0 = 1;
    DELAY_US(BLINK_DELAY_US);
    CH_0 = 0;
    DELAY_US(BLINK_DELAY_US);
    CH_1 = 1;
    DELAY_US(BLINK_DELAY_US);
    CH_1 = 0;
    DELAY_US(BLINK_DELAY_US);
    CH_2 = 1;
    DELAY_US(BLINK_DELAY_US);
    CH_2 = 0;
    DELAY_US(BLINK_DELAY_US);
    CH_3 = 1;
    DELAY_US(BLINK_DELAY_US);
    CH_3 = 0;
    DELAY_US(BLINK_DELAY_US);
    CH_4 = 1;
    DELAY_US(BLINK_DELAY_US);
    CH_4 = 0;
    DELAY_US(BLINK_DELAY_US);

}
*/

#include "DSP_FUNCTIONS.h"  //DSP specific functions

//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!
void delay(char * inData, char * outData, unsigned long receivedDelay) //FUNCTION NOT TESTED!
{
    unsigned long delayUs = 0;
    unsigned long delayCycles = 0;

	if (receivedDelay == 0)
	{
	    //Receive number of cycles to delay
	    delayCycles = inData[0];
	    delayCycles = delayCycles<<8;
	    delayCycles += inData[1];
	    delayCycles = delayCycles<<8;
	    delayCycles += inData[2];

	    delayUs = ((delayCycles * DELAY_CONVERTION_FACTOR)/1000);
	    DELAY_US(delayUs);

		//write d to out fifo to indicate the delay is done
		FIFO_OUT = DSPCMD_DELAY; 
	}

	else
	{
		//Used as internal delay.  Doesn't use serial bytes.
		delayUs = ((receivedDelay * DELAY_CONVERTION_FACTOR)/1000);
		DELAY_US(delayUs);
	}


}

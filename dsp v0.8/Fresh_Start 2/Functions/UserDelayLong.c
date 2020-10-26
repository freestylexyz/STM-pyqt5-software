#include "DSP_FUNCTIONS.h"  //DSP specific functions

//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!
//Receives 3 byte value via serial, MS byte first.
void delayLong(char * inData, char * outData)
{
	unsigned long delayTime = ((((unsigned long)inData[0])<<8)<<8) + (((unsigned long)inData[1])<<8) + ((unsigned long)inData[2]);

	DELAY_US(delayTime * 1000);

	FIFO_OUT = DSPCMD_DELAYLONG;
}

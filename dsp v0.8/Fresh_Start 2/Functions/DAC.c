#include "DSP_FUNCTIONS.h"  //DSP specific functions

//This sends the current values in each DAC channel to serial
//This necessitates storing each value that was last output to each channel.
//We could use a global variable or array that stores each output at appropriate positions, channel 0 output to array[0], etc.
//Saving the output of each channel should be done during the analogOut function.  That way, the only way to output to a channel would require saving the output.
void dac(void)
{
	unsigned int i; 
	//format is 0x0DDD
	
	//send output channel bits to serial
	
	//start with channel 0 and increment channel through channel 15.
	for(i = 0; i < NUM_OUT_CHANNEL; i++)
	{
		//serialOut_two((i<<12) + lastOutput[i]);
	    serialOut_two(lastOutput[i]);
	}
}

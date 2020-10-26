#include "DSP_FUNCTIONS.h"  //DSP specific functions

void version(char * inData, char * outData)
{
	unsigned int i = 0;

	memcpy(outData,FIRMWARE_VERSION,OUT_VERSION_SIZE); //receive version as char array from DEFINES
	
	for(i=0;i<OUT_VERSION_SIZE;i++)
	{
	    serialOut_char(outData[i]); //write to out fifo
	}
}

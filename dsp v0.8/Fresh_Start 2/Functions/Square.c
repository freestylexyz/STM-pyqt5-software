#include "DSP_FUNCTIONS.h"  //DSP specific functions

void squareWave(char * inData, char * outData)
{
	char checkCMD;
	unsigned long delayOldCycles;
	unsigned int firstBias, secondBias;

	//Receive three bytes for the microsecond delay
	delayOldCycles = inData[0];
	delayOldCycles = delayOldCycles << 8;
	delayOldCycles += inData[1];
	delayOldCycles = delayOldCycles << 8;
	delayOldCycles += inData[2];
	
	//Receive two bytes for second bias
	secondBias = inData[3];
	secondBias = secondBias << 8;
	secondBias += inData[4];
	
	//Receive two bytes for the first bias
	firstBias = inData[5];
	firstBias = firstBias << 8;
	firstBias += inData[6];
	
	do
	{
		//Output the first bias to channel 0
		analogOut(BIAS_OUT_CHANNEL, firstBias);
		
		//Delay before second bias output
		delay(0, 0, delayOldCycles);
		
		//Output the second bias to channel 0
		analogOut(BIAS_OUT_CHANNEL, secondBias);
		
		//Delay before first bias output
		delay(0, 0, delayOldCycles);
		
		//break loop if we get CMDSQUARE ('q' or $71) from serial
		checkCMD = SciaRegs.SCIRXBUF.all;
		
		if(checkCMD == DSPCMD_SQUAREWAVE)
		{
			break;
		}
	}
	while(true);
}

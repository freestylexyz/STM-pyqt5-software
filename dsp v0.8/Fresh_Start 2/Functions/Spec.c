#include "DSP_FUNCTIONS.h"  //DSP specific functions

unsigned long specLower;
unsigned long specHigher;
unsigned int specStepSize;
unsigned long specMoveDelay;

//Store parameters for Spec function
void specSetup(char *inData , char *outData)
{	
	//Spec_Lower is 2 bytes
	//Spec_Higher is 2
	//Spec_StepSize is 1
	//Spec_MoveDelay is 3 
	//Scan_Readdelay is 3  Note: This can be overwritten by ScanSetup

	specLower = 0;
	specHigher = 0;
	specStepSize = 0;
	specMoveDelay = 0;
	scanReadDelay = 0;
	
	specLower = inData[0];
	specLower = specLower << 8;
	specLower += inData[1];
    specLower = specLower << 8;
    specLower += inData[2];

	specHigher = inData[3];
	specHigher = specHigher << 8;
	specHigher += inData[4];
    specHigher = specHigher << 8;
    specHigher += inData[5];

	specStepSize = inData[6];
	specStepSize = specStepSize << 8;
	specStepSize += inData[7];

	specMoveDelay = inData[8];
	specMoveDelay = specMoveDelay << 8;
	specMoveDelay += inData[9];
	specMoveDelay = specMoveDelay << 8;
	specMoveDelay += inData[10];

	scanReadDelay = inData[11];
	scanReadDelay = scanReadDelay << 8;
	scanReadDelay += inData[12];
	scanReadDelay = scanReadDelay << 8;
	scanReadDelay += inData[13];
}
	
void spec(char *inData , char *outData)
{
	char flags;
	bool readForward, readBackward, highRes, higherRes;
	unsigned long channel, currentOutput, i, j, numSteps;

	//receive byte from serial  (inData)
	flags = inData[0];
	
	//#4 is set for higher-res
	//#5 is set for hi-res
	//#6 is set to read during backwards half of spectroscopy scan
	//#7 is set to read during forwards half of spectroscopy scan
	//LSBs are channel information
	//0bFFFFcccc
	readForward = (bool)((flags>>7) & 1);
	readBackward = (bool)((flags>>6) & 1);
	highRes = (bool)((flags>>5) & 1);
	higherRes = (bool)((flags>>4) & 1);
	channel = ((unsigned int)(flags & 0b00001111))<<4;
		
	//From upper (Spec_Higher), lower (Spec_Lower) limit and step size (Spec_StepSize), figure out how many steps are needed.
	numSteps = (specHigher - specLower) / specStepSize;

	//output initial value (Spec_Lower).  Use hi-res output if appropriate
	currentOutput = specLower;
	if(higherRes)
	{
	    higherResOutput(currentOutput);
	}
	else if(highRes)
	{
	    highResOutput(currentOutput, channel);
	}
	else
	{
	    analogOut(channel, currentOutput);
	}

	//if reading during forward pass, READSEQ and output data to serial now.
	if(readForward)
	{
		readSeq();
	}
	else
	{
	    serialOut_char(DSPCMD_UPDATE);
	}
	

	//forward pass
	for(i = 0; i < numSteps; i++)
	{
		for(j = 0; j < specStepSize; j++)
		{
			delay(0, 0, specMoveDelay);
			
			//increment current output value
			currentOutput++;
			
			//output new value, hi-res if appropriate
			//This ramps the current output value to the next step, bit by bit.
			if(higherRes)
			{
			    higherResOutput(currentOutput);
			}
			else if(highRes)
			{
			    highResOutput(currentOutput, channel);
			}
			else
			{
			    analogOut(channel, currentOutput);
			}
		}
		//if reading during forward pass, READSEQ now.
		if(readForward)
		{
			readSeq();
		}
		else
		{
		    serialOut_char(DSPCMD_UPDATE);
		}
	}
	
	delay(0, 0, specMoveDelay);
	
	//output current output value, hi-res if appropriate.  Not sure if this is needed.  Ramp should have left it here.  A bump?
	if(higherRes)
	{
	    higherResOutput(currentOutput);
	}
	else if(highRes)
	{
	    highResOutput(currentOutput, channel);
	}
	else
	{
	    analogOut(channel, currentOutput);
	}
	
	//if reading during backwards pass, READSEQ and send data to serial.
	if(readBackward)
	{
		readSeq();
	}
	else
	{
	    serialOut_char(DSPCMD_UPDATE);
	}
	
	//backwards pass
	for(i = 0; i < numSteps; i++)
	{
		for(j = 0; j < specStepSize; j++)
		{
			delay(0, 0, specMoveDelay);
			
			//decrement current output value
			currentOutput--;
			
			//output new value, hi-res if appropriate
			//Just like forward pass, but going down instead of up
			if(higherRes)
			{
			    higherResOutput(currentOutput);
			}
			else if(highRes)
			{
			    highResOutput(currentOutput, channel);
			}
			else
			{
			    analogOut(channel, currentOutput);
			}
		}
		
		//if reading during backwards pass, READSEQ now
		if(readBackward)
		{
			readSeq();
		}
		else
		{
		    serialOut_char(DSPCMD_UPDATE);
		}
	}
	
	//if remaining memory isn't empty, use XDSO to output.
	//send data to serial now
	serialOut_xdso();
}

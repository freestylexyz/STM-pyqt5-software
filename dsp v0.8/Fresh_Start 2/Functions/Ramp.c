#include "DSP_FUNCTIONS.h"  //DSP specific functions

bool crashProtectionEnabled;
unsigned int outputChannel;
unsigned int inputChannel;
unsigned long currentOutputValue;
data readData;

void rampRead(char *, char *);
void ramp(char *, char *);
unsigned long rampDir(unsigned long, unsigned long);
void rampLoop(bool, unsigned long, unsigned long);
bool rampDone(unsigned long, unsigned long);


bool rampUp = true;
unsigned long delayCycles;


void rampRead(char * inData, char * outData)
{
	bool readDuringRamp = true;
	unsigned int stepsHigh = 0;
	unsigned int stepsLow = 0;
	unsigned long steps;
	unsigned long initialVector = 0;
	unsigned long finalVector = 0;
	
	//Receive input channel from serial (1 byte) "0b000000cc"
	inputChannel = ((unsigned int)inData[0]) & 0x03;
	
	//Receive output channel and crash protection from serial (1 byte) "0bp000cccc"	
	crashProtectionEnabled = inData[1] >> 7;
	outputChannel = ((unsigned int)inData[1] & 0x0F)<<4;
	
	//Receive starting value of ramp from serial (Recieve_Three)
    initialVector = inData[2];
    initialVector = initialVector<<8;
    initialVector += inData[3];
    initialVector = initialVector<<8;
    initialVector += inData[4];
	
	//Receive final value of ramp from serial (Recieve_Three)
    finalVector = inData[5];
    finalVector = finalVector<<8;
    finalVector += inData[6];
    finalVector = finalVector<<8;
    finalVector += inData[7];
	
	//Determine direction and total number of steps of ramp.
	steps = rampDir(initialVector, finalVector);
	
	//Receive delay value from serial (Recieve_Three)
	delayCycles = inData[8];
	delayCycles = delayCycles<<8;
	delayCycles += inData[9];
	delayCycles = delayCycles<<8;
	delayCycles += inData[10];

    serialOut_char(DSPCMD_RAMPREAD);

    //Old software reads number of steps backwards.
    stepsHigh = (steps & 0xFF00)>>8;
    stepsLow =steps & 0x00FF;
    serialOut_char(stepsHigh);
    serialOut_char(stepsLow);

	rampLoop(readDuringRamp, initialVector, steps);
	
	//Send ramp command via serial to show completion.
	serialOut_char(DSPCMD_RAMP);
	
	//if crash protection was on, send new z-offset value and final output value (Serial_Two)
	if(crashProtectionEnabled)
	{
		serialOut_two(currentZOffset);
		serialOut_two(currentOutputValue);
	}
	
	return;
}
	
void ramp(char * inData, char * outData)
{
	//Receive output channel and crash protection from serial (1 byte) "0bp000cccc"
	bool readDuringRamp = false;
	unsigned long steps;
	unsigned long initialVector = 0;
	unsigned long finalVector = 0;

	
	crashProtectionEnabled = inData[0] >> 7;
	outputChannel = ((unsigned int)inData[0] & 0x0F)<<4;
	
	//if hi-res mode is on (set prior), figure out if channel 10 or 11 is given by serial
	
	
	//rampPrep(inData);
	//Receive starting value of ramp from serial (Recieve_Three)
	initialVector = inData[1];
	initialVector = initialVector<<8;
	initialVector += inData[2];
	initialVector = initialVector<<8;
	initialVector += inData[3];

	//Receive final value of ramp from serial (Recieve_Three)
	finalVector = inData[4];
	finalVector = finalVector<<8;
	finalVector += inData[5];
	finalVector = finalVector<<8;
	finalVector += inData[6];

	//Receive delay value from serial (Recieve_Three)
	delayCycles = inData[7];
	delayCycles = delayCycles<<8;
	delayCycles += inData[8];
	delayCycles = delayCycles<<8;
	delayCycles += inData[9];

    //Determine direction and total number of steps of ramp.
    steps = rampDir(initialVector, finalVector);

	rampLoop(readDuringRamp, initialVector, steps);

	//Send ramp command via serial to show completion.
    serialOut_char(DSPCMD_RAMP);
	
	//if crash protection was on, send new z-offset value and final output value (Serial_Two)
	if(crashProtectionEnabled)
	{
		serialOut_two(currentZOffset);
		serialOut_two(currentOutputValue);
	}
	
	return;
}

unsigned long rampDir(unsigned long start, unsigned long end)
{
	unsigned long totalSteps = 0;
	if (end > start)
	{
		rampUp = true;
		totalSteps = end - start;
	}
	else 
	{
		rampUp = false;
		totalSteps = start - end;
	}
	
	//if not hi-res, splice output channel into initial value (0xCDDD) (Assembly artifact)
	
	return totalSteps;
}
	
void rampLoop(bool readEnabled, unsigned long start, unsigned long numsteps)
{
	unsigned long i = 0;
	unsigned int feedbackCurrent, absFeedback;
	
	//currentOutputValue = lastOutput[outputChannel>>4];
	currentOutputValue = start;

	//for(currentOutputValue = lastOutput[outputChannel>>4]; ((currentOutputValue < end) && (rampUp)) || ((currentOutputValue > end) && (!rampUp)); )
	for(i = 0; i < numsteps; i++)
	{
		//save current output value to memory
		//output value to output channel (normal or hi-res as appropriate.)
		//analogOut(outputChannel, currentOutputValue);

		if(outputChannel == 0b11000000)
	    {
	        higherResOutput(currentOutputValue);
	    }
	    else if((outputChannel == 0b10110000) | (outputChannel == 0b10100000))
	    {
	        highResOutput(currentOutputValue, outputChannel);
	    }
	    else
	    {
	        analogOut(outputChannel, currentOutputValue);
	    }

		
		delay(0, 0, delayCycles);

		//serialOut_char(rampDone(currentOutputValue, end));

		if (crashProtectionEnabled)
		{
			//read z-feedback
			feedbackCurrent = analogIn(ZFB_IN_CHANNEL);
			
			absFeedback = absin(feedbackCurrent);

			if (absFeedback > absin(scanLimit)) //scanLimit set during last scanLine function call.
			{
				autoSub(INPUT_ZERO);
				
				//if autoplus maxed out z-offset, stop the loop
				if((currentZOffset == OUTPUT_MAX) || (currentZOffset == 0))
				{
					break;
				}
			}
		}
		
		if(readEnabled)
        {
		    readData.data[readData.numPoints] = analogIn(inputChannel);
		    readData.numPoints++;

		    if (readData.numPoints == MAXSIZE)
		    {
		        serialOut_xdso();
		    }
        }
		
		if(rampUp)
		{
			//increment current output value
			currentOutputValue++;
		}
		else
		{
			//decrement current output value
			currentOutputValue--;
		}
	}
}

bool rampDone(unsigned long currentOutputValue, unsigned long end)
{
    if(rampUp)
    {
        return(currentOutputValue >= end);
    }
    else
    {
        return(currentOutputValue <= end);
    }
}

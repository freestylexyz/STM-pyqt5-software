#include "DSP_FUNCTIONS.h"  //DSP specific functions
void autoPlus(char *, char *);
void autoMinus(char *, char *);
void autoSub(unsigned int);

unsigned int currentZOffset;

//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!
void autoPlus(char * inData, char * outData)
	{
	currentZOffset = 0;
	unsigned int outputChannel = 0;
	unsigned int autoTarget;
	unsigned int currentZFeedback;
	
	//inData expected to be 3 bytes
	//First two bytes are current z-offset value, most significant first
	currentZOffset = inData[0];
	currentZOffset = currentZOffset << 8;
	currentZOffset += inData[1];

	//Rearrange channel byte to 0xcccc0000
	outputChannel = inData[2];
	outputChannel = outputChannel << 4; //Not used!
	
	//Set auto target to INZERO (0x8000 = 0V)
	autoTarget = INPUT_ZERO;
	
	//AutoSub
	autoSub(autoTarget);
	
	currentZFeedback = analogIn(ZFB_IN_CHANNEL);
	
	//Adjust z-feedback to be positive.
	if (currentZFeedback < autoTarget)
	{
		currentZOffset--;
		analogOut(ZOFF_OUT_CHANNEL, currentZOffset);
	}
	
	//Output final z offset value to serial
	serialOut_two(currentZOffset);
	
	DELAY_US(8000);
	}

//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!
void autoMinus(char * inData, char * outData)
	{
	currentZOffset = 0;
	unsigned int outputChannel = 0;
	unsigned int autoTarget;
	unsigned int currentZFeedback;
	
	//inData expected to be 3 bytes
	//First two bytes are current z-offset value, most significant first
    currentZOffset = inData[0];
    currentZOffset = currentZOffset << 8;
    currentZOffset += inData[1];
	
	//Rearrange channel byte to 0xcccc0000
    outputChannel = inData[2];
    outputChannel = outputChannel << 4; //Not used!
	
	//Set auto target to INZERO (0x8000 = 0V)
	autoTarget = INPUT_ZERO;
	
	//AutoSub
	autoSub(autoTarget);
	
	currentZFeedback = analogIn(ZFB_IN_CHANNEL);
	
	//Adjust z-feedback to be negative.
	if (currentZFeedback > autoTarget)
	{
		currentZOffset++;
		analogOut(ZOFF_OUT_CHANNEL, currentZOffset);
	}
	
	//Output final z offset value to serial
    serialOut_two(currentZOffset);
	
	DELAY_US(8000);
	
	}
	

//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!	
void autoSub(unsigned int autoTarget)
{
	bool feedbackTooHigh;
	
	//read z-feedback (AnalogIn)
	unsigned int currentZFeedback = analogIn(ZFB_IN_CHANNEL);
	
	//note if we are above or below the target value
	//This will affect the direction of the ramp
	//if above target value, ramp down, else ramp up
	if(currentZFeedback < autoTarget)
	{
		feedbackTooHigh = true;
	}
	else
	{
		feedbackTooHigh = false;
	}
	
	while(true)
	{
		//read z-feedback (ReadIn)
		currentZFeedback = analogIn(ZFB_IN_CHANNEL);
		
		if (feedbackTooHigh==true)
		{	
		    //if new z-feedback is below target, break loop
		    if (currentZFeedback > autoTarget)
		    {
		        break;

		    }
			//increment z-offset value by 1 bit to decrease z-feedback.
			currentZOffset++;
			
			//if new z-offset value is at maximum, break loop
			if (currentZOffset == OUTPUT_MAX)
			{
				break;
			}
		}
		else
		{
            //if new z-feedback is above target, break loop
            if (currentZFeedback < autoTarget)
            {
                break;
            }

            //Decrement z-offset value by 1 bit to increase z-feedback.
			currentZOffset--;
			
			//if new z-offset value is below 0x0000, break loop.
			if (currentZOffset == 0)
			{
				break;
			}
		}

		//output new z-offset value.
		analogOut(ZOFF_OUT_CHANNEL, currentZOffset);
		DELAY_US(8000);
	}
	
}

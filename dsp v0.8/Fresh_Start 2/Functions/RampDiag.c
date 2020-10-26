#include "DSP_FUNCTIONS.h"  //DSP specific functions

unsigned int diagLongStart;
unsigned int diagLongFinal;
unsigned int diagShortStart;
unsigned int diagZStart;
unsigned int diagStepSize;
unsigned long diagRatio;
unsigned int diagZRatio;
unsigned long diagMoveDelay;

void rampDiagSetup(char * inData, char * outData)
{
	//Store parameters for RampDiag function MSB first
	
	//Diag_Long_Start is 2 bytes.		Long channel starting value
	//Diag_Long_Final is 2.				Long channel final value
	//Diag_Short_Start is 2.			Short channel starting value
	//Diag_Z_Start is 2.				Z starting value
	//Diag_Step_Size is 1.				# of bits per step (Read every N bits)
	//Diag_Ratio is 3.					Ratio?
	//Diag_Z_Ratio is 2.				z offset ratio
	//Diag_Move_Delay is 3.				Move delay in microseconds
	//Scan_Readdelay is 3.				Read delay - can be overwritten by ScanSetup or SpecSetup


	diagLongStart = 0;
	diagLongFinal = 0;
	diagShortStart = 0;
	diagZStart = 0;
	diagStepSize = 0;
	diagRatio = 0;
	diagZRatio = 0;
	diagMoveDelay = 0;
	scanReadDelay = 0;
	
	diagLongStart = inData[0];
	diagLongStart = diagLongStart << 8;
	diagLongStart += inData[1];

	diagLongFinal = inData[2];
	diagLongFinal = diagLongFinal << 8;
	diagLongFinal += inData[3];

	diagShortStart = inData[4];
	diagShortStart = diagShortStart << 8;
	diagShortStart += inData[5];

	diagZStart = inData[6];
	diagZStart = diagZStart << 8;
	diagZStart += inData[7];

	diagStepSize = inData[8];

	diagRatio = inData[9];
	diagRatio = diagRatio << 8;
	diagRatio += inData[10];
	diagRatio = diagRatio << 8;
	diagRatio += inData[11];

	diagZRatio = inData[12];
	diagZRatio = diagZRatio << 8;
	diagZRatio += inData[13];
    diagZRatio = diagZRatio << 8;
    diagZRatio += inData[14];

	diagMoveDelay = inData[15];
	diagMoveDelay = diagMoveDelay << 8;
	diagMoveDelay += inData[16];
	diagMoveDelay = diagMoveDelay << 8;
	diagMoveDelay += inData[17];

	scanReadDelay = inData[18];
	scanReadDelay = scanReadDelay << 8;
	scanReadDelay += inData[19];
	scanReadDelay = scanReadDelay << 8;
	scanReadDelay += inData[20];
}
	
void rampDiag(char *inData, char *outData)
{
	unsigned int shortChannel, longChannel, currentShortOutput, currentLongOutput, currentZOffsetOutput;
	char flags;
	bool diagConstZ, incZOffset, diagReadOn, diagLongUp, diagShortUp, diagDone;
	int i;
	
	diagDone = false;
	
	//receive short channel from serial (1 byte) Handled by inData.
	flags =  inData[0];
	shortChannel = (unsigned int)(flags & 0b00001111)<<4;
	
	//MSB is set if using constant height ramp
	diagConstZ = (bool) (flags & 0b10000000);
	
	//2nd MSB is set if increasing z offset, clear if decreasing.
	incZOffset = (bool) (flags & 0b01000000);
	
	//Splice together short channel and Diag_Short_Start  (Assembly artifact)
	
	
	//receive long channel from serial (1 byte)
	flags =  inData[1];
	longChannel = (unsigned int)(flags & 0b00001111)<<4;
	
	//MSB is set if reading during ramp
	diagReadOn = (bool) (flags & 0b10000000);
	
	//2nd MSB is set if incrementing long channel, clear if decrementing.
	diagLongUp = (bool) (flags & 0b01000000);
	
	//3rd MSB is set if incrementing short channel, clear if decrementing.
	diagShortUp = (bool) (flags & 0b00100000);
	
	//splice together long channel and Diag_Long_Start  (Assembly artifact)
	
	//output initial short channel value
	analogOut(shortChannel, diagShortStart);
	currentShortOutput = diagShortStart;
	
	do
	{
		for(i = 0; i < diagStepSize; i++)
		{
			delay(0, 0, diagMoveDelay);
			
			//output current long channel value
			analogOut(longChannel, diagLongStart);
			
			//check if short channel value needs to increment.  Differences of absolute values here need to be treated carefully since the variables are unsigned.
			if (diagShortUp)
			{
				if(diagLongUp)
				{
					if(diagRatio * (currentShortOutput - diagShortStart) <= (currentLongOutput - diagLongStart))
					{
						currentShortOutput++;
					}
				}
				else
				{
					if(diagRatio * (currentShortOutput - diagShortStart) <= (diagLongStart - currentLongOutput))
					{
						currentShortOutput++;
					}
				}
			}
			else
			{
				if(diagLongUp)
				{
					if(diagRatio * (diagShortStart - currentShortOutput) <= (currentLongOutput - diagLongStart))
					{
						currentShortOutput--;
					}
				}
				else
				{
					if(diagRatio * (diagShortStart - currentShortOutput) <= (diagLongStart - currentLongOutput))
					{
						currentShortOutput--;
					}
				}
			}
			
			//save and output short channel value
			analogOut(shortChannel, currentShortOutput);

			//check if z offset needs to increment.  Differences of absolute values here need to be treated carefully since the variables are unsigned.
			if (!diagConstZ)
			{
				if(incZOffset)
				{					
					if(diagLongUp)
					{
						if(diagZRatio * (currentZOffsetOutput - diagZStart) <= (currentLongOutput - diagLongStart))
						{
							currentZOffsetOutput++;
						}
					}
					else
					{
						if(diagZRatio * (currentZOffsetOutput - diagZStart) <= (diagLongStart - currentLongOutput))
						{
							currentZOffsetOutput++;
						}
					}
				}
				else
				{
					if(diagLongUp)
					{
						if(diagZRatio * (diagZStart - currentZOffsetOutput) <= (currentLongOutput - diagLongStart))
						{
							currentZOffsetOutput--;
						}
					}
					else
					{
						if(diagZRatio * (diagZStart - currentZOffsetOutput) <= (diagLongStart - currentLongOutput))
						{
							currentZOffsetOutput--;
						}
					}
				}
			}
			
			if (currentLongOutput == diagLongFinal)
			{
				diagDone = true;
				break;
			}
			
			//inc/dec current long value
			if(diagLongUp)
			{
				currentLongOutput++;
			}
			else
			{
				currentLongOutput--;
			}
		}
		
		if (diagDone)
		{
			break;
		}
		
		if (diagReadOn)
		{
			readSeq();
		}
	}
	while(true);
	
	//if reading, output data to serial using XDSO
	if(diagReadOn)
	{
		serialOut_xdso();
	}
	
	//output diag command to serial to show completion
	serialOut_char(DSPCMD_DIAG);
	
	//if not in constant z mode
	if(!diagConstZ)
	{
		//output final z offset (Serial_Two)
		serialOut_two(currentZOffsetOutput);
	}
	
	//output final long output (Serial_Two)
	serialOut_two(currentLongOutput);
	
	//output final short output (Serial_Two)
	serialOut_two(currentShortOutput);
}

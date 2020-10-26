#include "DSP_FUNCTIONS.h"  //DSP specific functions

unsigned int readSeqNum;
unsigned long readSeqCommands[128];  //Matching the limits of the old DSP code.  Trying to avoid a memory leak by not making this dynamically allocated.
extern data readData;

void readSeqSetup(char * inData, char * outData, Uint16 readSeqWords)
{	
	unsigned int i;
	//unsigned int readSeqWords = ((sizeof(inData) / sizeof(inData[0])) - 1) / 3;
	//Store parameters for ReadSeq function

	//Number of Words (3 byte segments) is 1 byte.  This is received at memory allocation step.
	//Number of Steps (ReadSeqNum) is 1 byte
	//Note not every step is 3 bytes long.
	readSeqNum = inData[0];

	for (i = 0; i < readSeqWords; i++)
	{
	//  Store words in ordered list
	    readSeqCommands[i] = inData[3*i+1];
        readSeqCommands[i] = readSeqCommands[i] << 8;
	    readSeqCommands[i] += inData[3*i+2];
        readSeqCommands[i] = readSeqCommands[i] << 8;
        readSeqCommands[i] += inData[3*i+3];
	}
}
	
void readSeq()
{
	unsigned int i, j, readSeqChannel, command, outputData;
	unsigned long step, delayMult, readSeqAvg, k, accumulation, seqMultiplier;
	
	//Delay for 116ns * Scan_Readdelay.  originally minimum 116ns, maximum 1.95s.  
	delay(0, 0, scanReadDelay);
	
	//Make sure that the read sequence queue is not empty.
	//If the queue is empty, skip to the end.
	if (readSeqNum == 0)
	{
		return;
	}
	
	j = 0;
	//do (readSeqNum) times 
	for (i = 0; i < readSeqNum; i++)
	{
		//Pull next sequence step from the list
		step = readSeqCommands[j++];
		command = (step & 0x00F00000)>>20;
		
		switch(command)
		{
			case 1: //Read
	//			step here is the input channel to read and the number of times to read and average
	
	//			channel data is c.  (0x0000ccaa|aaaaaaaa|aaaaaaaa) lines here are just to help guide the eye.
	//			These bits are saved into the LSBs of ReadSeq_Ch (0x000000cc)
				readSeqChannel = (step & 0x000C0000)>>18;
				
	//			pull the averaging number "READSEQAVG" from command (listed as "a" above) current max = 2^18 -1 = 262143 times
				readSeqAvg = step & 0x0003FFFF;

	//          Read the next word in the sequence for the "sequence multiplier" (1/readSeqAvg)
	            seqMultiplier = readSeqCommands[j++];
	
				accumulation = 0;
				for(k = 0; k < readSeqAvg; k++)
				{
					//	read and stitch together the bits from ReadSeq_Ch
					accumulation += analogIn(readSeqChannel);
				}

				//  multiply the result by 1/READSEQAVG and add to accumulation
				accumulation /= readSeqAvg;
				
	//			if reading from z-feedback, adjust value according to z-offset
	//			This uses flagZero from scanline
				if(readSeqChannel == ZFB_IN_CHANNEL)
				{
				    if(flagZero)
				    {
				        accumulation += scanDeltaZ;
				    }
				    else
				    {
				        accumulation -= scanDeltaZ;
				    }
				}
				
	//			store value in memory
				readData.data[readData.numPoints] = accumulation;
				readData.numPoints++;

	//			if memory is full, output saved values to serial. (XDSO)
				if(readData.numPoints == MAXSIZE)
				{
				    serialOut_xdso();
				}

				break;
	
			case 2: //Analog Output
	//			step here is the channel and output data (0xCcdddd)
	//			C is 0010 (output), c is the channel, d is the output data
	//			pull apart channel and output data using masks
				readSeqChannel = (step & 0x000F0000) >>16;
				outputData = step & 0x0000FFFF;  //Note here that the 4 MSBs will not be output.
				
	//			reduce channel info to one byte (0xccccXXXX)  Artifact
				
	//			Send channel and output data to output function for 12 bits
				analogOut(readSeqChannel, outputData);
				break;
				
			case 3: //Serial Output
	//			step here is a single byte to be sent to serial
				serialOut_char((char)(step & 0x000000FF));
				break;
	
			case 4: //Dither channel 0
	//			step here is the channel to be changed and the value to add to it
								
	//			Dither Mask ($00F7FF for channel 0 dither) to pull out value to add
	//			read current dither value for appropriate channel (Channel 13)
				outputData = (step & 0x0000F7FF) + lastOutput[DITHER_OUT_CHANNEL>>4];
					
	//			add value masked from command to current value and output
				analogOut(DITHER_OUT_CHANNEL, outputData);
				break;

			case 6: //High Res Analog Output
	//          Send all data bits to the 20 bit DAC
			    higherResOutput(step & 0x000FFFFF);
			    break;

			case 8: //Toggle Feedback
	//			step here is the toggle
	
	//			read current feedback toggle value (channel 14)
	//			use feedback mask (0x00FFFFFB) on current value
	//			add in relevant bits from command to current value (#22-23 of command to #0-1 of current value)
	//			Note, current value #0-1 erased by feedback mask.
				outputData = (lastOutput[FB_OUT_CHANNEL>>4] & 0x00FFFFFB) + ((step & 0x00C00000)>>22);
				
	//			Output new feedback value
				analogOut(FB_OUT_CHANNEL, outputData);
				break;


	
			case 12: //Dither channel 1
	//			step here is the channel to be changed and the value to add to it
				
	//			Dither Mask ($00FBFF for channel 1 dither) to pull out value to add
	//			read current dither value for appropriate channel (Channel 13)
				outputData = (step & 0x0000FBFF)  + lastOutput[DITHER_OUT_CHANNEL>>4];
								
	//			add value masked from command to current value and output
				analogOut(DITHER_OUT_CHANNEL, outputData);
				break;
	
			default: //Delay
	//		    step here is the number of cycles to delay
	//          skip the next step to ignore the delay conversion
			    delayMult = readSeqCommands[j++];
			    delay(0, 0, step * delayMult);
			    break;
		}
	}
	
	return;
}

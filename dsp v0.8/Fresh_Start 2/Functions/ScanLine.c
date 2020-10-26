#include "DSP_FUNCTIONS.h"  //DSP specific functions

unsigned int scanProtection;
unsigned int scanLimit;
unsigned int zOffset;  //May be shared by other functions.
unsigned int zOffsetChannel;
unsigned int scanSize;
unsigned int scanStep;
unsigned long scanInterDelay;
unsigned long scanReadDelay;

unsigned int thisLineMax;
unsigned int thisLineMin;
unsigned int lastLineMax;
unsigned int lastLineMin;

bool flagZero = true;
unsigned int scanDeltaZ; //Used in Scan Protection

void scanSetup(char * inData, char * outData)
{
	//Store parameters for ScanLine function
	
	scanProtection = 0;	//Scan_Protection is 1 byte
	scanLimit = 0;		//Scan_Limit is 2
	zOffset = 0;		//z_offset is 2
	zOffsetChannel = 0;	//z_off_ch is 1, received in four MSB bits (0xcccc0000), stored in 4 LSB bits. (0x0000cccc)
	scanSize = 0;		//Scan_Size is 2
	scanStep = 0;		//Scan_Step is 2
	scanInterDelay = 0;	//Scan_Interdelay is 3
	scanReadDelay = 0;	//Scan_Readdelay is 3
	
	scanProtection = inData[0];

	scanLimit = inData[1];
	scanLimit = scanLimit << 8;
	scanLimit += inData[2];

	zOffset = inData[3];
	zOffset = zOffset << 8;
	zOffset += inData[4];

	zOffsetChannel = inData[5];

	scanSize = inData[6];
	scanSize = scanSize << 8;
	scanSize += inData[7];

	scanStep = inData[8];
	scanStep = scanStep << 8;
	scanStep += inData[9];

	scanInterDelay = inData[10];
	scanInterDelay = scanInterDelay << 8;
	scanInterDelay += inData[11];
	scanInterDelay = scanInterDelay << 8;
	scanInterDelay += inData[12];

    scanReadDelay = inData[13];
    scanReadDelay = scanReadDelay << 8;
    scanReadDelay += inData[14];
    scanReadDelay = scanReadDelay << 8;
    scanReadDelay += inData[15];
}

void scanLine(char * inData, char * outData)
{
	unsigned int outChannel;
	bool readDuringRamp;
	bool direction; //True for up, false for down
	bool startNew;	//True to clear Scan_DeltaZ and MIN/MAX, false to save, different name?
	bool scanSuccessful;
	unsigned long totalScanSize;
	unsigned int currentOutput;
	unsigned int scanOldZ;
	unsigned int scanNewZ;

	unsigned int protTarget;
	unsigned int tempint;
	
	unsigned int i, j;

	//4 bits are output channel info, MSB is read on/off, second is up/down, third is clear/keep Scan_DeltaZ and Min+Max
	//0xddd*cccc
	readDuringRamp = ((inData[0] & 0b10000000)>>7);
	direction = ((inData[0] & 0b01000000)>>6);
	startNew = ((inData[0] & 0b00100000)>>5);
	outChannel = (inData[0] & 0b00001111)<<4;
	
	//If third MSB is set, set Scan_DeltaZ to zero.
	if (startNew)
	{
		scanDeltaZ = 0;
	}
	
	//Get total scan size in bits from Scan_Step and Scan_Size(Scan_Size is in steps/scan line, not bits)
	totalScanSize = scanStep * scanSize;
	
	//if reading forward, get the total number of values above zero ("zero" - (bits/scanline)/2)
	//else get the number of values below zero ("zero" + (bits/scanline)/2 -1)
	//this value is the starting point of a square scan with the middle at "zero"
	//"zero" is 0x0800.  Represents 0V for 12bit output.
	if(direction)
	{
		currentOutput = OUTPUT_ZERO - (unsigned int)(totalScanSize / 2);
	}
	else
	{
		currentOutput = OUTPUT_ZERO + (unsigned int)(totalScanSize / 2) - 1;
	}
	
	//save and output starting value.
	analogOut(outChannel, currentOutput);
	
	//nested do loop:
	for (i = 0; i < scanSize; i++)
	{
		for (j = 0; j < scanStep; j++)
		{
			//Delay for Scan_Interdelay cycles.
			delay(0, 0, scanInterDelay);

			//Save and output current output value.
			analogOut(outChannel, currentOutput);

			//Increment or Decrement output value (DO NOT OUTPUT HERE) depending on direction.
			if(direction)
			{
				currentOutput++;
			}
			else
			{
			    //Make sure the changed value is not out of range. (Don't decrement 0x0000.)
			    if (currentOutput == 0)
			    {
			        currentOutput = 0;
			    }
			    else
			    {
			        currentOutput--;
			    }
			}

		}
		
		//Scan Protection
		
		//Read z-feedback and save (Scan_OldZ)
		scanOldZ = analogIn(ZFB_IN_CHANNEL);
		
		//If this is the first bit read in the line, save z-feedback value as ThisLine_Max and ThisLine_Min
		if (i == 0)
		{
			thisLineMax = scanOldZ;
			thisLineMin = scanOldZ;
		}
		//If this is the first bit read in the scan (first line, first bit), save z-feedback value as LastLine_Max and LastLine_Min
		if((i == 0) && (startNew))
		{
			lastLineMax = scanOldZ;
			lastLineMin = scanOldZ;
		}
		
		//check to see if Scan_Limit - #>INZERO < |z-feedback - #>INZERO| (Are we outside protection limit?)
		//If we are and if scan protection is on
		if((scanLimit - INPUT_ZERO < absin(scanOldZ)) && scanProtection)
		{
			//	autosub to adjust z-feedback + z-offset (Give current z-offset value to autoplus)
			//	Use autosub to target +/- ((max - min)/2), if too high/low
			if(scanOldZ < INPUT_ZERO)
			{
				protTarget = INPUT_ZERO + (lastLineMax - lastLineMin)/2;
			}
			else
			{
				protTarget = INPUT_ZERO - (lastLineMax - lastLineMin)/2;
			}	
			autoSub(protTarget);
		
			//	save new z-offset value from autoSub
			//	the new z-offset value is saved as currentZOffset global variable in AutoPlusMinus code.
			
			//	if we maxed out z-offset (from autoSub), the scan failed.  Stop the loop.
			if(currentZOffset == OUTPUT_MAX)
			{
				scanSuccessful = false;
				break;
			}
		
			//	check z-feedback and compare to saved value.
			scanNewZ = analogIn(ZFB_IN_CHANNEL);
			
			//	b = |new z-feedback - old z-feedback|
			if(scanNewZ > scanOldZ)
			{
				tempint = scanNewZ - scanOldZ;
			}
			else
			{
				tempint = scanOldZ - scanNewZ;
			}
		
			//	scan_flags#0 also used in ReadSeq, never given a name.
			//	Was initialized to true during DSP setup in assembly code
			//	if (new z-feedback > old z-feedback) XOR (scan_flags#0)
			if((scanNewZ > scanOldZ) != flagZero)
			{
				scanDeltaZ += tempint;
			//	if Scan_DeltaZ overflows, end do loop and mark scan line failure
				if(scanDeltaZ < tempint)
				{
					scanSuccessful = false;
					break;
				}
			}
			else if (absin(scanNewZ - scanOldZ) < scanDeltaZ)
			{
				//	switch Scan_flags#0
				flagZero = !flagZero;
				
				//	Scan_DeltaZ = ||new z-feedback - old z-feedback| - Scan_DeltaZ|
				scanDeltaZ = absin(absin(scanNewZ - scanOldZ) - scanDeltaZ);
			}
		}
		//Scan Protection Done
		
		//ReadSeq at this new position
		readSeq();
	}
	
	
	//output data with XDSO
	//ReadSeq does not output any data to serial unless the memory storage becomes full.
	//We output the readings to the computer here.
	serialOut_xdso();
	
	//if line scanned successfully
	if(scanSuccessful)
	{
	//	send scanline command to serial to signal completion
		serialOut_char(DSPCMD_SCANLINE);
	}
	//if scanLine failed
	else
	{
	//	send stop command to serial to signal failure
	    serialOut_char(DSPCMD_STOP);
	}

	if (scanProtection)
	{
	//	send z offset value (Serial_Two) and final output value (Serial_Two) to serial
		serialOut_two(currentZOffset);
		serialOut_two(currentOutput);
	}

}

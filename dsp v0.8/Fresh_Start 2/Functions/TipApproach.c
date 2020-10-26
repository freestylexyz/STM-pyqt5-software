#include "DSP_FUNCTIONS.h"  //DSP specific functions

unsigned long zOuterParabMult = 0;
unsigned int zOuterParabSize = 0;
unsigned long xOuterParabMult = 0;
unsigned int xOuterParabSize = 0;
unsigned int xOuterParabZeroCycles = 0;
unsigned long tipApproachDelay = 0;

unsigned int tipNumGiant;
unsigned int tipBabySize;
unsigned int tipMinI;
unsigned long tipBabyMult;

void tipSetup(char * inData, char * outData)
{
	//Store parameters for TipApproach function
	
	//zOuterParabMult is 3 bytes.		Z outer parabola multiplier
	//zOuterParabSize is 2.			Z outer parabola step size
	//xOuterParabMult is 3.			X parabola multiplier
	//xOuterParabSize is 2.			X parabola step size
	//xOuterParabZeroCycles is 2.		X zeroing cycles
	//tipApproachDelay is 3.			delay (in microseconds)


	zOuterParabMult = 0;
	zOuterParabSize = 0;

	xOuterParabMult = 0;
	xOuterParabSize = 0;
	xOuterParabZeroCycles = 0;

	tipApproachDelay = 0;

	zOuterParabMult = inData[0];
	zOuterParabMult = zOuterParabMult << 8;
	zOuterParabMult += inData[1];
    zOuterParabMult = zOuterParabMult << 8;
    zOuterParabMult += inData[2];
	
	zOuterParabSize = inData[3];
	zOuterParabSize = zOuterParabSize << 8;
	zOuterParabSize += inData[4];

	xOuterParabMult = inData[5];
	xOuterParabMult = xOuterParabMult << 8;
	xOuterParabMult += inData[6];
	xOuterParabMult = xOuterParabMult << 8;
	xOuterParabMult += inData[7];

	xOuterParabSize = inData[8];
	xOuterParabSize = xOuterParabSize << 8;
	xOuterParabSize += inData[9];
	
	xOuterParabZeroCycles = inData[10];
	xOuterParabZeroCycles = xOuterParabZeroCycles << 8;
	xOuterParabZeroCycles += inData[11];

	tipApproachDelay = inData[12];
	tipApproachDelay = tipApproachDelay << 8;
	tipApproachDelay += inData[13];
	tipApproachDelay = tipApproachDelay << 8;
	tipApproachDelay += inData[14];
}

void tipApproach(char * inData, char * outData)
{
	bool done = false;
	char checkCmd;
	unsigned int finalCurrent, i;
	//Receive more parameters for Tip Approach
	
	//tipNumGiant is 1 byte		Number of giant steps per pass
	//tipBabySize is 1 byte		Baby step size
	//tipMinI is 2 bytes			Minimum Tunneling Current
	//tipBabyMult is 3 bytes		Baby zo parabola multiplier
	tipNumGiant = inData[0];

	tipBabySize = inData[1];
	
	tipMinI = inData[2];
	tipMinI = tipMinI << 8;
	tipMinI += inData[3];

	tipBabyMult = inData[4];
	tipBabyMult = tipBabyMult << 8;
	tipBabyMult += inData[5];
	tipBabyMult = tipBabyMult << 8;
	tipBabyMult += inData[6];
	
	while(true)
	{
		done = babyStep(tipBabySize);
		
		if (done)
		{
			break;
		}
		
		for(i = 0; i < tipNumGiant; i++)
		{
			oneGiant(0); //Direction is down for tip approach.
		}
		
		//Check serial for stop command and break loop if received.
		//if(FIFO_IN_DATA_AVAILABLE)
		//{
		checkCmd = FIFO_IN;
		if(checkCmd == DSPCMD_STOP)
		{
			break;
		}
		//}
		
		//Delay 1us
		DELAY_US(1);
		
		//Send latest tip current with approach command to serial (0xDD DD CM)
		finalCurrent = analogIn(I_IN_CHANNEL);
		serialOut_char(DSPCMD_TIPAPPROACH);
		serialOut_two(finalCurrent);
	}
		
	if (done)
	{
		//Send z outer position that triggers tunneling current with stop command to serial (same format)
		finalCurrent = analogIn(I_IN_CHANNEL);
        serialOut_char(DSPCMD_STOP);
        serialOut_two(zOuterTunnel);
	}
	
}

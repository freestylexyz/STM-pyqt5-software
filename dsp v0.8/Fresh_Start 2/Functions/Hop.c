#include "DSP_FUNCTIONS.h"  //DSP specific functions
#include <time.h>

unsigned int hopAvg;
unsigned long hopStepDelay;
unsigned long hopIStepDelay;
unsigned long hopCircDelay;
unsigned int hopR;
unsigned int hopRSqrt;
unsigned long hopPlaneA;
unsigned long hopPlaneB;

void sendTip(unsigned int, unsigned int, unsigned long);
void hopSlope(unsigned int, unsigned int);
unsigned long hopRead();

//Store parameters for Hop function
void hopSetup(char * inData, char * outData)
{
	//Hop_Avg is 1 byte (Max 256)
	//Hop_Step_Delay is 3 bytes
	//Hop_IStep_Delay is 3 bytes
	//Hop_Circ_Delay is 3 bytes.  
	//Hop_R is 2 bytes.
	//Hop_R_Sqrt is 2 bytes.
	//Hop_Plane_a is 3 bytes.
	//Hop_Plane_b is 3 bytes.

	hopAvg = 0;
	hopStepDelay = 0;
	hopIStepDelay = 0;
	hopCircDelay = 0;
	hopR = 0;
	hopRSqrt = 0;
	hopPlaneA = 0;
	hopPlaneB = 0;
	
	hopAvg = inData[0];

	hopStepDelay = inData[1];
	hopStepDelay = hopStepDelay << 8;
	hopStepDelay += inData[2];
	hopStepDelay = hopStepDelay << 8;
	hopStepDelay += inData[3];

	hopIStepDelay = inData[4];
	hopIStepDelay = hopIStepDelay << 8;
	hopIStepDelay += inData[5];
	hopIStepDelay = hopIStepDelay << 8;
	hopIStepDelay += inData[6];

	hopCircDelay = inData[7];
	hopCircDelay = hopCircDelay << 8;
	hopCircDelay += inData[8];
	hopCircDelay = hopCircDelay << 8;
	hopCircDelay += inData[9];

	hopR = inData[10];
	hopR = hopR << 8;
	hopR += inData[11];

	hopRSqrt = inData[12];
	hopRSqrt = hopRSqrt << 8;
	hopRSqrt += inData[13];

	hopPlaneA = inData[14];
	hopPlaneA = hopPlaneA << 8;
	hopPlaneA += inData[15];
	hopPlaneA = hopPlaneA << 8;
	hopPlaneA += inData[16];

	hopPlaneB = inData[17];
	hopPlaneB = hopPlaneB << 8;
	hopPlaneB += inData[18];
	hopPlaneB = hopPlaneB << 8;
	hopPlaneB += inData[19];
	
	return;
}
	
	
void hop(char * inData, char * outData)
{
	char command;
	unsigned int xCoord, yCoord;
	clock_t startingTime, timeOut;
	unsigned long timeOutUS;
	
	//setup timers
	//time.h functions?  clock()
	//Is the time reported the time since the last tip movement?  Or is it the time since the hop loop began?
	startingTime = clock();
	
	do
	{
		//Step 1: Receive initial x and y coordinates from serial
		//Receive x coordinate (receive two bytes from serial, MSB first)
		xCoord = 0;
		xCoord = serialIn();
		xCoord = xCoord << 8;
		xCoord += serialIn();
		
		//Receive y coordinate (receive two bytes from serial, MSB first)
		yCoord = 0;
		yCoord = serialIn();
		yCoord = yCoord << 8;
		yCoord += serialIn();
		
		//Step 2: Move tip to starting position
		//setup delay for RampLoop.  Use Hop_IStep_Delay
		//sendDelay = hopStepDelay;  Artifact from assembly code.
		sendTip(xCoord, yCoord, hopStepDelay);
		
		//read timer and save value to memory
		//Do not reset timer on read
		timeOut = clock() - startingTime;
		
		//Step 3: Calculate and send slopes to serial
		//Send x slope to serial (Serial_Word)
		//Send y slope to serial (Serial_Word)
		hopSlope(xCoord, yCoord);
		
		
		//Send time value read in step 2 to serial (Serial_Word)
		timeOutUS = timeOut * 1000000 / CLOCKS_PER_SEC;
		serialOut_word(timeOutUS);
		
		//Step 4: Check serial to continue or stop
		//If serial command is TrackCont ('c'), go back to step 1
		//while(FIFO_IN_DATA_AVAILABLE_SIZE == 0) {}
		//command = FIFO_IN;
		command = serialIn();
	}	while(command == DSPCMD_TRACKCONT);
	
	//stop timers  (Assembly artifact)
		
	//move back to center of last circle
	sendTip(xCoord, yCoord, hopStepDelay);
}
	
void sendTip(unsigned int xTarget, unsigned int yTarget, unsigned long delayTime)
{
	unsigned int xInitial, yInitial, steps;
	//Save x and y to memory (assembly artifact)
	
	//Step 1: Ramp X
	//Read last value sent to x channel and use as current x
	xInitial = lastOutput[X_OUT_CHANNEL>>4];
	
	//initialX = current x from memory	
	//finalX = target x from arguments
		
	steps = rampDir(xInitial, xTarget);
	outputChannel = X_OUT_CHANNEL;
	rampLoop(false, steps);
	
	
	//Step 2: Ramp Y
	//Read last value sent to y channel and use as current y
	yInitial = lastOutput[Y_OUT_CHANNEL>>4];
	
	//initialY = current y from memory	
	//finalY = target y from arguments
		
	steps = rampDir(yInitial, yTarget);
	outputChannel = Y_OUT_CHANNEL;
	rampLoop(false, steps);
	
	delay(0, 0, delayTime);
}
	
void hopSlope(unsigned int xPos, unsigned int yPos)
{
	unsigned long readSlope, xSlope, ySlope;
	
	//initialize slope values to zero (0x800000)
	xSlope = 0x800000;
	ySlope = 0x800000;
	
	//Point 1:
	sendTip(xPos, yPos + hopR, hopStepDelay);
	readSlope = hopRead();
	ySlope += readSlope/2;
	
	//Point 5:
	sendTip(xPos, yPos - hopR, hopStepDelay);
	readSlope = hopRead();
	ySlope -= readSlope/2;
	
	//Point 3:
	sendTip(xPos + hopR, yPos, hopStepDelay);
	readSlope = hopRead();
	xSlope += readSlope/2;
	
	//Point 7:
	sendTip(xPos - hopR, yPos, hopStepDelay);
	readSlope = hopRead();
	xSlope -= readSlope/2;
	
	//Point 8:
	sendTip(xPos - hopRSqrt, yPos + hopRSqrt, hopStepDelay);
	readSlope = hopRead();
	xSlope -= readSlope/4;
	ySlope += readSlope/4;
	
	//Point 2:
	sendTip(xPos + hopRSqrt, yPos + hopRSqrt, hopStepDelay);
	readSlope = hopRead();
	xSlope += readSlope/4;
	ySlope += readSlope/4;
	
	//Point 4:
	sendTip(xPos + hopRSqrt, yPos - hopRSqrt, hopStepDelay);
	readSlope = hopRead();
	xSlope += readSlope/4;
	ySlope -= readSlope/4;
	
	//Point 6:
	sendTip(xPos - hopRSqrt, yPos - hopRSqrt, hopStepDelay);
	readSlope = hopRead();
	xSlope -= readSlope/4;
	ySlope -= readSlope/4;
	
	serialOut_word(xSlope);
	serialOut_word(ySlope);
}
	
unsigned long hopRead()
{
	unsigned int i;
	unsigned long total;
	
	//initialize running total
	total = 0;
	
	for (i = 0; i < hopAvg; i++)
	{
		total += analogIn(ZFB_IN_CHANNEL);
	}
	
	return(total);
}

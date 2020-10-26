#include "DSP_FUNCTIONS.h"  //DSP specific functions

unsigned int trackAverage;
unsigned long trackSampleEvery;
unsigned long trackDelay;
unsigned long trackPlaneA;
unsigned long trackPlaneB;
bool trackMin;

void moveReadComp(unsigned int, unsigned int, unsigned int *);

//Store parameters for Track function
void trackSetup(char *inData, char *outData)
{
	//Track_Average is 1 byte (Max 256)
	//Track_Sample_Every is 3 bytes
	//Track_Delay is 3 bytes
	//Track_Plane_a is 3 bytes.  Format is set for bit 23 is 2^5, bit 0 is 2^-18
	//Track_Plane_b is 3 bytes.  Format is set for bit 23 is 2^5, bit 0 is 2^-18
	
	trackAverage = 0;
	trackSampleEvery = 0;
	trackDelay = 0;
	trackPlaneA = 0;
	trackPlaneB = 0;
	trackMin = 0;
	
	trackAverage = inData[0];

	trackSampleEvery = inData[1];
	trackSampleEvery = trackSampleEvery << 8;
	trackSampleEvery += inData[2];
	trackSampleEvery = trackSampleEvery << 8;
	trackSampleEvery += inData[3];

	trackDelay = inData[4];
	trackDelay = trackDelay << 8;
	trackDelay += inData[5];
	trackDelay = trackDelay << 8;
	trackDelay += inData[6];

	trackPlaneA = inData[7];
	trackPlaneA = trackPlaneA << 8;
	trackPlaneA += inData[8];
	trackPlaneA = trackPlaneA << 8;
	trackPlaneA += inData[9];

	trackPlaneB = inData[10];
	trackPlaneB = trackPlaneB << 8;
    trackPlaneB += inData[11];
    trackPlaneB = trackPlaneB << 8;
    trackPlaneB += inData[12];
}
	
void track(char *inData, char *outData)
{
	int i;
	char command;
	unsigned int xPos, yPos;
	unsigned int bestPos[3];

	//Receive initial x position from serial (2 bytes) MSB first
	xPos = serialIn();
    xPos = xPos << 8;
    xPos += serialIn();
	
	//Receive initial y position from serial (2 bytes) MSB first
    yPos = serialIn();
    yPos = yPos << 8;
    yPos += serialIn();
	
	//MSB of initial x position is track min/max (set/clear)
	trackMin = (xPos & 0b1000000000000000);
	xPos = (xPos & 0b0111111111111111);
	
	do	{	
	//initialize best x,y,z coordinate values.
	bestPos[0] = xPos;
	bestPos[1] = yPos;
	if (trackMin)
	{
		bestPos[2] = INPUT_MAX;
	}
	else
	{
		bestPos[2] = 0b0000000000000000;
	}
	
	for(i = 0; i < 10; i++)
	{
		moveReadComp(xPos, yPos, bestPos);
		
		//move one bit to the right
		xPos += 1;
		moveReadComp(xPos, yPos, bestPos);
		
		//move one bit up
		yPos += 1;
		moveReadComp(xPos, yPos, bestPos);
		
		//move one bit left
		xPos -= 1;
		moveReadComp(xPos, yPos, bestPos);
		
		//move one bit left
		xPos -= 1;
		moveReadComp(xPos, yPos, bestPos);
		
		//move one bit down
		yPos -= 1;
		moveReadComp(xPos, yPos, bestPos);
		
		//move one bit down
		yPos -= 1;
		moveReadComp(xPos, yPos, bestPos);
		
		//move one bit right
		xPos += 1;
		moveReadComp(xPos, yPos, bestPos);
		
		//move one bit right
		xPos += 1;
		moveReadComp(xPos, yPos, bestPos);
		
		//move one bit up
		yPos += 1;
		moveReadComp(xPos, yPos, bestPos);
		
		//move one bit left
		xPos -= 1;
		moveReadComp(xPos, yPos, bestPos);
		
		//output best x value to x channel
		analogOut(X_OUT_CHANNEL, bestPos[0]);
		delay(0, 0, trackDelay);
		
		//output best y value to y channel
		analogOut(Y_OUT_CHANNEL, bestPos[1]);
		delay(0,0, trackSampleEvery);
		
	}
	
	//send best x coordinate to serial LSB first
	serialOut_two(bestPos[0]);
	
	//send best y coordinate to serial LSB first
	serialOut_two(bestPos[1]);
	
	//check serial for next command.
	command = serialIn();
}
	while (command == DSPCMD_TRACKCONT); //repeat the track cycle

	return;
}

//subroutine of Track.  Given a x,y coordinate, will move to that position, check the height and compare to the current "best"
void moveReadComp(unsigned int xPosition, unsigned int yPosition, unsigned int * bestPositions) 
{
	unsigned int i, accumulation, zFinal;
	//output received x coordinate to x channel
	analogOut(X_OUT_CHANNEL, xPosition);
	delay(0, 0, trackDelay);
	
	//output received y coordinate to y channel
	analogOut(Y_OUT_CHANNEL, yPosition);
	delay(0, 0, trackDelay);
	
	//initialize average values
	accumulation = 0;
	
	for (i = 0; i < trackAverage; i++)
	{
		//read z channel and accumulate value
		accumulation = accumulation + analogIn(ZFB_IN_CHANNEL);
	}
	accumulation = accumulation / trackAverage;
	
	//adjust value according to plane fit
	zFinal = accumulation - (trackPlaneA * xPosition + trackPlaneB * yPosition);
	
	if (((zFinal < bestPositions[2]) && trackMin) || ((zFinal > bestPositions[2]) && !trackMin))
	{
		bestPositions[0] = xPosition;
		bestPositions[1] = yPosition;
		bestPositions[2] = zFinal;
	}
	
	return;
}

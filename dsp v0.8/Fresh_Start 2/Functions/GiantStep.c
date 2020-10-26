#include "DSP_FUNCTIONS.h"  //DSP specific functions

//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!
//Make a number of coarse approach steps without checking tunneling current in between steps.
//Needs parameters set by tipSetup.
void giantStep(char * inData, char * outData)
{
	unsigned int direction = 2; //start with no direction
	unsigned int i, numSteps = 0;
	char checkCmd;

	//inData format: Dbbbbbbb bbbbbbbb
	direction = inData[0] >> 7;

	numSteps = inData[0] & 0b01111111;
	numSteps = numSteps << 8;
	numSteps += inData[1];

	for(i = 0; i < numSteps; i++)
	{
	    //Make one step at a time
		oneGiant(direction);

		//Check for stop command between each step.
		//if(FIFO_IN_DATA_AVAILABLE)
		//{
		checkCmd = SciaRegs.SCIRXBUF.all;
		if(checkCmd == DSPCMD_STOP)
		{
			break;
		}
		//}
		DELAY_US(1);

		//Send command character to confirm successful step.
		FIFO_OUT = DSPCMD_GIANTSTEP;
	}
}	


void translateSample(char * inData, char * outData)
{
    unsigned int direction = 2; //start with no direction
        unsigned int i, numSteps = 0;
        char checkCmd;

        //inData format: Dbbbbbbb bbbbbbbb
        direction = inData[0] >> 7;

        numSteps = inData[0] & 0b01111111;
        numSteps = numSteps << 8;
        numSteps += inData[1];

        for(i = 0; i < numSteps; i++)
        {
            //Make one step at a time
            oneTranslate(direction);

            //Check for stop command between each step.
            //if(FIFO_IN_DATA_AVAILABLE)
            //{
            checkCmd = FIFO_IN;
            if(checkCmd == DSPCMD_STOP)
            {
                break;
            }
            //}
            DELAY_US(1);

            //Send command character to confirm successful step.
            FIFO_OUT = DSPCMD_TRANSLATE_SAMPLE;
        }
}
	
//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!//FUNCTION NOT TESTED!
//A single coarse approach step.  Used by TipApproach and GiantStep
void oneGiant(unsigned int direction)
{
	unsigned int xChannelTarget = 0;
	unsigned int xChannelCurrent = 0;
	
	unsigned int zChannelTarget = 0;
	unsigned int zChannelCurrent = 0;
	
	//output 0V to x channel (0x0800 to channel 2)
	analogOut(X_OUT_CHANNEL, OUTPUT_ZERO);
	xChannelCurrent = OUTPUT_ZERO;
	
	//delay(3 us * Tip_X_Zero)
	DELAY_US(3 * xOuterParabZeroCycles);
	
	//output normal parabola to x channel
	//increasing parabola if direction is up, decreasing parabola if down
	//target value given by xOuterParabSize, multiplier given by xOuterParabMult
	if (direction)
	{
		xChannelTarget = OUTPUT_ZERO + xOuterParabSize;
	}
	else
	{
		xChannelTarget = OUTPUT_ZERO - xOuterParabSize;
	}
	
	xChannelCurrent = parabola(X_OUT_CHANNEL, xChannelCurrent, xChannelTarget, xOuterParabMult, false);
	analogOut(X_OUT_CHANNEL, xChannelTarget);
	
	xChannelCurrent = xChannelTarget;
	
	//Step z outer to 0V + zOuterParabSize.
	zChannelTarget = OUTPUT_ZERO + zOuterParabSize;
	
	analogOut(Z_OUTER_CHANNEL, zChannelTarget);
	zChannelCurrent = zChannelTarget;
	
	//Delay(15 us)
	DELAY_US(15);
	
	//Step x channel to 0V.
	analogOut(X_OUT_CHANNEL, OUTPUT_ZERO);
	
	//Delay before restoring z outer
	delay(0, 0, tipApproachDelay);
	
	//Restoring Z Outer Parabola
	//1st Half Parabola
	//decreasing parabola, target is 0V + zOuterParabSize/2, multiplier given by zOuterParabMult
	zChannelTarget = OUTPUT_ZERO + (zOuterParabSize>>1);
	zChannelCurrent = parabola(Z_OUTER_CHANNEL, zChannelCurrent, zChannelTarget, zOuterParabMult, false);
		
	//2nd Half Parabola
	//output inverted parabola to z outer
	//decreasing parabola, target is 0V
	//multiplier given by zOuterParabMult
	//Use same number of steps as first half.
	
	zChannelTarget = OUTPUT_ZERO;
	zChannelCurrent = parabola(Z_OUTER_CHANNEL, zChannelCurrent, zChannelTarget, zOuterParabMult, true);
	
	//Output 0V to z outer (Just to be sure)
	analogOut(Z_OUTER_CHANNEL, OUTPUT_ZERO);
}

void oneTranslate(unsigned int direction)
{
    unsigned int xChannelTarget = 0;
    unsigned int xChannelCurrent = 0;

    unsigned int zChannelTarget = 0;
    unsigned int zChannelCurrent = 0;

    //output 0V to x channel (0x0800 to channel 2)
    analogOut(Y_OUT_CHANNEL, OUTPUT_ZERO);
    xChannelCurrent = OUTPUT_ZERO;

    //delay(3 us * Tip_X_Zero)
    DELAY_US(3 * xOuterParabZeroCycles);

    //output normal parabola to x channel
    //increasing parabola if direction is up, decreasing parabola if down
    //target value given by xOuterParabSize, multiplier given by xOuterParabMult
    if (direction)
    {
        xChannelTarget = OUTPUT_ZERO + xOuterParabSize;
    }
    else
    {
        xChannelTarget = OUTPUT_ZERO - xOuterParabSize;
    }

    xChannelCurrent = parabola(Y_OUT_CHANNEL, xChannelCurrent, xChannelTarget, xOuterParabMult, false);
    analogOut(Y_OUT_CHANNEL, xChannelTarget);

    xChannelCurrent = xChannelTarget;

    //Step z outer to 0V + zOuterParabSize.
    zChannelTarget = OUTPUT_ZERO + zOuterParabSize;

    analogOut(Z_OUTER_CHANNEL, zChannelTarget);
    zChannelCurrent = zChannelTarget;

    //Delay(15 us)
    DELAY_US(15);

    //Step x channel to 0V.
    analogOut(Y_OUT_CHANNEL, OUTPUT_ZERO);

    //Delay before restoring z outer
    delay(0, 0, tipApproachDelay);

    //Restoring Z Outer Parabola
    //1st Half Parabola
    //decreasing parabola, target is 0V + zOuterParabSize/2, multiplier given by zOuterParabMult
    zChannelTarget = OUTPUT_ZERO + (zOuterParabSize>>1);
    zChannelCurrent = parabola(Z_OUTER_CHANNEL, zChannelCurrent, zChannelTarget, zOuterParabMult, false);

    //2nd Half Parabola
    //output inverted parabola to z outer
    //decreasing parabola, target is 0V
    //multiplier given by zOuterParabMult
    //Use same number of steps as first half.

    zChannelTarget = OUTPUT_ZERO;
    zChannelCurrent = parabola(Z_OUTER_CHANNEL, zChannelCurrent, zChannelTarget, zOuterParabMult, true);

    //Output 0V to z outer (Just to be sure)
    analogOut(Z_OUTER_CHANNEL, OUTPUT_ZERO);
}

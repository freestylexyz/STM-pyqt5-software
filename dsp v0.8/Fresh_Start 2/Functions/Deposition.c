#include "DSP_FUNCTIONS.h"  //DSP specific functions


void depBiasAndI(unsigned int targetBias, unsigned int targetCurrent, bool step);
void depBiasAndI20(unsigned long targetBias, unsigned int targetCurrent, bool step);

unsigned long depBiasM;
unsigned long depBiasW;
unsigned int depIM;
unsigned int depIW;
unsigned int depZOffset;
unsigned long depWait;
unsigned int depNumPts;
unsigned long depDelay;
unsigned int depAvgPts;
unsigned int depIgnoreInit;
unsigned int depChngT;
unsigned int depAfterT;
unsigned int depChngRatio;
unsigned int depAvgEvery;
unsigned long depAvgInv;
unsigned int depFlags;

//Store parameters for Deposition function
void depSetup(char *inData, char *outData)
{
	//Dep_BiasM is 2 bytes.		This is not used in Deposition.
	//Dep_BiasW is 2.			Writing Bias
	//Dep_IM is 2.				This is not used in Deposition.
	//Dep_IW is 2.				Writing Current
	//Dep_Z_Offset is 2.		Self-explanatory
	//Dep_Wait is 3.			delay during ramp.  0 = Max
	//Dep_Num_Pts is 2.			total number of data points to store
	//Dep_Delay is 3.			Delay between stored data points.  0 = Max
	//Dep_Aver_Pts is 2.		This is not used in Deposition.
	//Dep_Ignore_Init is 2.		This is not used in Deposition.
	//Dep_Chng_T is 2.			This is not used in Deposition.
	//Dep_After_T is 2.			This is not used in Deposition.
	//Dep_Chng_Ratio is 2.		This is not used in Deposition.
	//Dep_Aver_Every is 2.		"Average number per stored data points?"  Recommended >64
	//Dep_Aver_Inv is 3.		1/Dep_Aver_Every
	//Dep_Flags is 1.			Flags

	depBiasM = 0;
	depBiasW = 0;
	depIM = 0;
	depIW = 0;
	depZOffset = 0;
	depWait = 0;
	depNumPts = 0;
	depDelay = 0;
	depAvgPts = 0;
	depIgnoreInit = 0;
	depChngT = 0;
	depAfterT = 0;
	depChngRatio = 0;
	depAvgEvery = 0;
	depAvgInv = 0;
	depFlags = 0;
	
	depBiasM = inData[0];
	depBiasM = depBiasM << 8;
	depBiasM += inData[1];

	depBiasW = inData[2];
	depBiasW = depBiasW << 8;
	depBiasW += inData[3];

	depIM = inData[4];
	depIM = depIM << 8;
	depIM += inData[5];

	depIW = inData[6];
	depIW = depIW << 8;
	depIW += inData[7];

	depZOffset = inData[8];
	depZOffset = depZOffset << 8;
	depZOffset += inData[9];

	depWait = inData[10];
	depWait = depWait << 8;
	depWait += inData[11];
	depWait = depWait << 8;
	depWait += inData[12];

    depNumPts = inData[13];
    depNumPts = depNumPts << 8;
    depNumPts += inData[14];

    depDelay = inData[15];
    depDelay = depDelay << 8;
    depDelay += inData[16];
    depDelay = depDelay << 8;
    depDelay += inData[17];

    depAvgPts = inData[18];
    depAvgPts = depAvgPts << 8;
    depAvgPts += inData[19];

    depIgnoreInit = inData[20];
    depIgnoreInit = depIgnoreInit << 8;
    depIgnoreInit += inData[21];

    depChngT = inData[22];
    depChngT = depChngT << 8;
    depChngT += inData[23];

    depAfterT = inData[24];
    depAfterT = depAfterT << 8;
    depAfterT += inData[25];

    depChngRatio = inData[26];
    depChngRatio = depChngRatio << 8;
    depChngRatio += inData[27];

    depAvgEvery = inData[28];
    depAvgEvery = depAvgEvery << 8;
    depAvgEvery += inData[29];

    depAvgInv = inData[30];
    depAvgInv = depAvgInv << 8;
    depAvgInv += inData[31];
    depAvgInv = depAvgInv << 8;
    depAvgInv += inData[32];

	depFlags = inData[33];
}

//Store parameters for Deposition function
void dep20Setup(char *inData, char *outData)
{
    //Dep_BiasM is 3 bytes.     This is not used in Deposition.
    //Dep_BiasW is 3.           Writing Bias
    //Dep_IM is 2.              This is not used in Deposition.
    //Dep_IW is 2.              Writing Current
    //Dep_Z_Offset is 2.        Self-explanatory
    //Dep_Wait is 3.            delay during ramp.  0 = Max
    //Dep_Num_Pts is 2.         total number of data points to store
    //Dep_Delay is 3.           Delay between stored data points.  0 = Max
    //Dep_Aver_Pts is 2.        This is not used in Deposition.
    //Dep_Ignore_Init is 2.     This is not used in Deposition.
    //Dep_Chng_T is 2.          This is not used in Deposition.
    //Dep_After_T is 2.         This is not used in Deposition.
    //Dep_Chng_Ratio is 2.      This is not used in Deposition.
    //Dep_Aver_Every is 2.      "Average number per stored data points?"  Recommended >64
    //Dep_Aver_Inv is 3.        1/Dep_Aver_Every
    //Dep_Flags is 1.           Flags

    depBiasM = 0;
    depBiasW = 0;
    depIM = 0;
    depIW = 0;
    depZOffset = 0;
    depWait = 0;
    depNumPts = 0;
    depDelay = 0;
    depAvgPts = 0;
    depIgnoreInit = 0;
    depChngT = 0;
    depAfterT = 0;
    depChngRatio = 0;
    depAvgEvery = 0;
    depAvgInv = 0;
    depFlags = 0;

    depBiasM = inData[0];
    depBiasM = depBiasM << 8;
    depBiasM += inData[1];
    depBiasM = depBiasM << 8;
    depBiasM += inData[2];

    depBiasW = inData[3];
    depBiasW = depBiasW << 8;
    depBiasW += inData[4];
    depBiasW = depBiasW << 8;
    depBiasW += inData[5];

    depIM = inData[6];
    depIM = depIM << 8;
    depIM += inData[7];

    depIW = inData[8];
    depIW = depIW << 8;
    depIW += inData[9];

    depZOffset = inData[10];
    depZOffset = depZOffset << 8;
    depZOffset += inData[11];

    depWait = inData[12];
    depWait = depWait << 8;
    depWait += inData[13];
    depWait = depWait << 8;
    depWait += inData[14];

    depNumPts = inData[15];
    depNumPts = depNumPts << 8;
    depNumPts += inData[16];

    depDelay = inData[17];
    depDelay = depDelay << 8;
    depDelay += inData[18];
    depDelay = depDelay << 8;
    depDelay += inData[19];

    depAvgPts = inData[20];
    depAvgPts = depAvgPts << 8;
    depAvgPts += inData[21];

    depIgnoreInit = inData[22];
    depIgnoreInit = depIgnoreInit << 8;
    depIgnoreInit += inData[23];

    depChngT = inData[24];
    depChngT = depChngT << 8;
    depChngT += inData[25];

    depAfterT = inData[26];
    depAfterT = depAfterT << 8;
    depAfterT += inData[27];

    depChngRatio = inData[28];
    depChngRatio = depChngRatio << 8;
    depChngRatio += inData[29];

    depAvgEvery = inData[30];
    depAvgEvery = depAvgEvery << 8;
    depAvgEvery += inData[31];

    depAvgInv = inData[32];
    depAvgInv = depAvgInv << 8;
    depAvgInv += inData[33];
    depAvgInv = depAvgInv << 8;
    depAvgInv += inData[34];

    depFlags = inData[35];
}

void deposition(char *inData, char *outData)
{
	bool readDuringDeposition, readTunnelingCurrent, stepBiasCurrent, highResBias;
	unsigned int i, j, inputChannel, runningTotal, zFeedbackOn, savedFeedback, newFeedback, savedZOffset, savedBias, savedCurrent;
	unsigned int *depReadData;
	
	readDuringDeposition = depFlags & 0b00000001;
	readTunnelingCurrent = depFlags & 0b00000010;
	zFeedbackOn = depFlags & 0b00000100;
	stepBiasCurrent = depFlags & 0b00001000;
	highResBias = depFlags & 0b00100000;
	//Other flags not used in current implementation
	
	//allocate memory for depReadData.  Use depSetup values.
	//depReadData = new unsigned int[depNumPts];
	depReadData = (unsigned int *) malloc(depNumPts * sizeof(unsigned int));
	
	//Step 1: Feedback Setup
	//save current setting for feedback to restore later
	//reads last sent setting for feedback
	savedFeedback = lastOutput[(FB_OUT_CHANNEL>>4)];

	newFeedback = (savedFeedback & 0xFFFB) + (zFeedbackOn);
	analogOut(FB_OUT_CHANNEL, newFeedback);

	//Step 2: Ramp to Moving Values *Not Implemented*
		
	//Step 3: Delta Z Offset
	//Save current z offset value to memory
	savedZOffset = lastOutput[(ZOFF_OUT_CHANNEL>>4)];
	
	//output deposition z offset value (from DepSetup) to z offset channel
	analogOut(ZOFF_OUT_CHANNEL, depZOffset);
	
	//Step 4: Ramp to Writing Values
	//Use DepBiasAndI to move to writing values (Dep_BiasW, Dep_IW from DepSetup)
	savedBias = lastOutput[(BIAS_OUT_CHANNEL>>4)];
	savedCurrent = lastOutput[(I_OUT_CHANNEL>>4)];
		
	depBiasAndI(depBiasW, depIW, stepBiasCurrent);
	delay(0, 0, depWait);
	
	//Step 5: Read or Wait
	if (readDuringDeposition)
	{
		if (readTunnelingCurrent)
		{
			//set input channel to 0 (current)
			inputChannel = I_IN_CHANNEL;
		}
		else
		{
			//set input channel to 1 (z-feedback)
			inputChannel = ZFB_IN_CHANNEL;
		}
		
		for(i = 0; i < depNumPts; i++)
		{
			//initialize running total to 0
			runningTotal = 0;
			
			for(j = 0; j < depAvgEvery; j++)
			{
				//read input channel
				//add input value to running total
				runningTotal += analogIn(inputChannel);
			}
			//divide running total by Dep_Aver_Every to get average value.
			runningTotal /= depAvgEvery;
			
			//save average value to memory
			depReadData[i] = runningTotal;
			
			//This is not what was originally used.  Make sure manipulation does not depend on this function
			
			delay(0, 0, depDelay);
		}
	}
	else
	{
		delay(0, 0, depNumPts * (depDelay + depAvgEvery * 2276 + 22));  //number is in clock cycles.  Matches reading time
	}
	
	//Step 6: Ramp Back to Initial Values
	//Use DepBiasAndI to go back to initial values
	depBiasAndI(savedBias, savedCurrent, stepBiasCurrent);
	
	//Step 7: Undo Delta Z Offset
	//Output saved z offset value to z offset channel.
	analogOut(ZOFF_OUT_CHANNEL, savedZOffset);
	
	//Step 8: Ramp to Moving Values *Not Implemented*
	
	//Step 9: Feedback Return
	//Restore feedback to the setting it was at before step 1.
	analogOut(FB_OUT_CHANNEL, savedFeedback);
	
	//Step 10: Output Read Data to Serial
	
	//Send number of data points to serial (Serial_Two)
	serialOut_two(depNumPts);
	
	for(i = 0; i < depNumPts; i++)
	{
		//send data stored using Serial_Two
		serialOut_two(depReadData[i]);
	
		//delay 1us between sending data points
		DELAY_US(1);
	}
	//free memory used for depReadData
	//delete[] depReadData;
	free(depReadData);
}

void deposition20(char *inData, char *outData)
{
    bool readDuringDeposition, readTunnelingCurrent, stepBiasCurrent, highResBias;
    unsigned int i, j, inputChannel, runningTotal, zFeedbackOn, savedFeedback, newFeedback, savedZOffset, savedBias, savedCurrent;
    unsigned int *depReadData;

    readDuringDeposition = depFlags & 0b00000001;
    readTunnelingCurrent = depFlags & 0b00000010;
    zFeedbackOn = depFlags & 0b00000100;
    stepBiasCurrent = depFlags & 0b00001000;
    highResBias = depFlags & 0b00100000;
    //Other flags not used in current implementation

    //allocate memory for depReadData.  Use depSetup values.
    //depReadData = new unsigned int[depNumPts];
    depReadData = (unsigned int *) malloc(depNumPts * sizeof(unsigned int));


    //Step 1: Feedback Setup
    //save current setting for feedback to restore later
    //reads last sent setting for feedback
    savedFeedback = lastOutput[(FB_OUT_CHANNEL>>4)];

    newFeedback = (savedFeedback & 0xFFFB) + (zFeedbackOn);
    analogOut(FB_OUT_CHANNEL, newFeedback);

    //Step 2: Ramp to Moving Values *Not Implemented*

    //Step 3: Delta Z Offset
    //Save current z offset value to memory
    savedZOffset = lastOutput[(ZOFF_OUT_CHANNEL>>4)];

    //output deposition z offset value (from DepSetup) to z offset channel
    analogOut(ZOFF_OUT_CHANNEL, depZOffset);

    //Step 4: Ramp to Writing Values
    //Use DepBiasAndI to move to writing values (Dep_BiasW, Dep_IW from DepSetup)
    savedBias = lastOutput[(BIAS_OUT_CHANNEL>>4)];
    savedCurrent = lastOutput[(I_OUT_CHANNEL>>4)];

    depBiasAndI20(depBiasW, depIW, stepBiasCurrent);
    delay(0, 0, depWait);

    //Step 5: Read or Wait
    if (readDuringDeposition)
    {
        if (readTunnelingCurrent)
        {
            //set input channel to 0 (current)
            inputChannel = I_IN_CHANNEL;
        }
        else
        {
            //set input channel to 1 (z-feedback)
            inputChannel = ZFB_IN_CHANNEL;
        }

        for(i = 0; i < depNumPts; i++)
        {
            //initialize running total to 0
            runningTotal = 0;

            for(j = 0; j < depAvgEvery; j++)
            {
                //read input channel
                //add input value to running total
                runningTotal += analogIn(inputChannel);
            }
            //divide running total by Dep_Aver_Every to get average value.
            runningTotal /= depAvgEvery;

            //save average value to memory
            depReadData[i] = runningTotal;

            //This is not what was originally used.  Make sure manipulation does not depend on this function

            delay(0, 0, depDelay);
        }
    }
    else
    {
        delay(0, 0, depNumPts * (depDelay + depAvgEvery * 2276 + 22));  //number is in clock cycles.  Matches reading time
    }

    //Step 6: Ramp Back to Initial Values
    //Use DepBiasAndI to go back to initial values
    depBiasAndI20(savedBias, savedCurrent, stepBiasCurrent);

    //Step 7: Undo Delta Z Offset
    //Output saved z offset value to z offset channel.
    analogOut(ZOFF_OUT_CHANNEL, savedZOffset);

    //Step 8: Ramp to Moving Values *Not Implemented*

    //Step 9: Feedback Return
    //Restore feedback to the setting it was at before step 1.
    analogOut(FB_OUT_CHANNEL, savedFeedback);

    //Step 10: Output Read Data to Serial

    //Send number of data points to serial (Serial_Two)
    serialOut_two(depNumPts);

    for(i = 0; i < depNumPts; i++)
    {
        //send data stored using Serial_Two
        serialOut_two(depReadData[i]);

        //delay 1us between sending data points
        DELAY_US(1);
    }
    //free memory used for depReadData
    //delete[] depReadData;
    free(depReadData);
}

void depBiasAndI(unsigned int targetBias, unsigned int targetCurrent, bool step)
{
	unsigned int initialBias, initialCurrent, steps;
	
	//if step==true, step directly to target values
	//else gradual ramp of bias and current to targets
	if(step)
	{
		analogOut(BIAS_OUT_CHANNEL, targetBias);
		analogOut(I_OUT_CHANNEL, targetCurrent);
	}
	else
	{
		//read the initial bias value from memory (normal or hi-res)
		initialBias = lastOutput[(BIAS_OUT_CHANNEL>>4)];
	
		//ramp bias to target value.
		steps = rampDir(initialBias, targetBias);
		
		//setup variables for rampLoop
		outputChannel = BIAS_OUT_CHANNEL;
		currentOutputValue = initialBias;
		crashProtectionEnabled = false;
		
		rampLoop(false, steps);
	
		//read the initial current value from memory (channel 8 for current setpoint)
		initialCurrent = lastOutput[(I_OUT_CHANNEL>>4)];
		
		//ramp current setpoint to target value.
		steps = rampDir(initialCurrent, targetCurrent);
		
		//setup variables for rampLoop
		outputChannel = I_OUT_CHANNEL;
		currentOutputValue = initialCurrent;
		crashProtectionEnabled = false;
		
		rampLoop(false, steps);
	}
}

void depBiasAndI20(unsigned long targetBias, unsigned int targetCurrent, bool step)
{
    unsigned long initialBias;
    unsigned int initialCurrent, steps;

    //if step==true, step directly to target values
    //else gradual ramp of bias and current to targets
    if(step)
    {
        higherResOutput(targetBias);
        analogOut(I_OUT_CHANNEL, targetCurrent);
    }
    else
    {
        //read the initial bias value from memory (normal or hi-res)
        initialBias = lastOutput[(BIAS_OUT_CHANNEL>>4)];

        //ramp bias to target value.
        steps = rampDir(initialBias, targetBias);

        //setup variables for rampLoop
        outputChannel = BIAS_OUT_CHANNEL;
        currentOutputValue = initialBias;
        crashProtectionEnabled = false;

        rampLoop(false, steps);

        //read the initial current value from memory (channel 8 for current setpoint)
        initialCurrent = lastOutput[(I_OUT_CHANNEL>>4)];

        //ramp current setpoint to target value.
        steps = rampDir(initialCurrent, targetCurrent);

        //setup variables for rampLoop
        outputChannel = I_OUT_CHANNEL;
        currentOutputValue = initialCurrent;
        crashProtectionEnabled = false;

        rampLoop(false, steps);
    }
}

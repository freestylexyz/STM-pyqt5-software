#include "DSP_FUNCTIONS.h"  //DSP specific functions
bool babyStep(unsigned int);


unsigned int zOuterTunnel = 0;

bool babyStep(unsigned int stepSize)
{
	
	unsigned int current;
	unsigned int old_current;
	unsigned int zo;
	
	zo = OUTPUT_ZERO;

	//Wait for 8 ms
	DELAY_US(8000);
	
	//Read value from current
	current = analogIn(I_IN_CHANNEL);
	
	if (absin(current) > tipMinI) //ABSIN defined in the macros.  Only to be used with unsigned int type.
	{
		//Wait for 5 ms
		DELAY_US(5000);
		
		//Save the value of the current just read.
		old_current = current;
		
		//Read current again.
		current = analogIn(I_IN_CHANNEL);
	}

	do 
	{
		if (absin(current) > tipMinI)
		{
			break;
		}
			
		//if we've maxed z outer, stop the loop.
		if (zo > OUTPUT_MAX - stepSize)
		{
		    break;
		}

		//increment zo by tipBabyStep (set by TipApproach)
		zo += stepSize;
			
		

		// output new zo value
		analogOut(Z_OUTER_CHANNEL, zo);
		
		// wait 20 us
		DELAY_US(20);
		
		// read new current after zo increase.
		current = analogIn(I_IN_CHANNEL);
		
		if (absin(current) > tipMinI)
		{
			//Wait for 5 ms
			DELAY_US(5000);
			
			//Save the value of the current just read.
			old_current = current;
			
			//Read current again.
			current = analogIn(I_IN_CHANNEL);
		}
		
	}
	while(zo < OUTPUT_MAX); //This makes sure there are enough steps to max out z outer at some point during the loop.
	
	if (absin(current) > tipMinI)
	{
		//save the zo value where we saw tunneling current (tip_update).  This is not used later?
		zOuterTunnel = zo; 
		
		//If tip approach was complete, return true.
		return(true); 
	}
	
	//Restore zo to zero using parabola function
	//start with normal decreasing parabola for half of the restoration
	zo = parabola(Z_OUTER_CHANNEL, zo, (zo>>1) + OUTPUT_ZERO, tipBabyMult, false);
	
	//end with inverted decreasing parabola
	zo = parabola(Z_OUTER_CHANNEL, zo, OUTPUT_ZERO, tipBabyMult, true);
	
	//output zero to zo to be sure it returns to the proper value.
	analogOut(Z_OUTER_CHANNEL, OUTPUT_ZERO);
	zo = OUTPUT_ZERO;
	
	return(false); //If tip approach was incomplete, return false.
}

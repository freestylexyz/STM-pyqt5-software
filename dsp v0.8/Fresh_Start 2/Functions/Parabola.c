#include "DSP_FUNCTIONS.h"  //DSP specific functions

unsigned int stepNumber = 0;

unsigned int parabola(unsigned int outChannel, unsigned int initialValue, unsigned int targetValue, unsigned long multiplier, bool invert) //Used by TipApproach to return to original values carefully.
{
	unsigned int currentValue;
	unsigned int slope;
	unsigned long temp1;

	//output initialValue to outChannel
	analogOut(outChannel, initialValue);
	currentValue = initialValue;
	
	if (invert)
	{
	    //Use previous stepNumber
	}
	else
	{
	    //initialize stepNumber to 0
	    stepNumber = 0;
	}
	
	//stepNumber = (Sqrt(1+4(final-initial)/mult)-1)/2  Need this for invert parabola.  We shouldn't depend on external source for step numbers.
/*
	{
		//need to be careful about data types here.
		//In C++, arithmetic operations promote smaller types like short to int
		//multiplier was originally a 3 byte object, so now it is an long
		//multiplication operation was originally between two unsigned numbers, this is why most of the variables are unsigned.
		//Simple arithmetic is straightforward and doesn't cause a problem.  Need to remember that all the short variables are promoted to int.
		//Problem is introduced by sqrt.  This only works with float, doubles, etc.  Change everything to double, sqrt it, then back to int.  This rounds down.
		stepNumber = (unsigned int)((sqrt((double)((4 * (targetValue - initialValue)) * multiplier + 1)) - 1) / 2);
	}
*/
	
	do{
		//calculate slope of parabola.  ****Careful about data types here too.****
		temp1 = (unsigned long)((2 * stepNumber + 1) * multiplier);
		
		slope = (unsigned int)(temp1 >> 16 & 0xFFFF);
		
		//add/sub slope to running total (starts at initialValue)
		//if increasing parabola, add.  If decreasing parabola, subtract.
		//Compare new value to targetValue.
		//If it isn't there yet, output the new value to outChannel and continue the loop
		//if it is, output the targetValue to outChannel and stop the loop
		if (targetValue > currentValue)
		{
			if(targetValue > (currentValue + slope))
			{
			    currentValue += slope; //Slope is limited here.  Upper bits are truncated due to typecast.  If stepNumber or multiplier too big, can cause error.
			    analogOut(outChannel, currentValue);
			}
			else
			{
				analogOut(outChannel, targetValue);
				break;
			}
		}
		else
		{
			if(currentValue > (targetValue + slope))
			{
			    currentValue -= slope;
				analogOut(outChannel, currentValue);
			}
			else
			{
				analogOut(outChannel, targetValue);
				break;
			}
		}


		DELAY_US(2);
	
		//increment/decrement stepNumber
		//if inverted parabola, decrement.  Else increment.
		if(invert)
		{
			stepNumber--;
		}
		else
		{
			stepNumber++;
		}
		
		//don't let stepNumber go negative.  Break loop at stepNumber==0
		if(stepNumber <= 0)
		{
			break;
		}
	}while(true);
	
	

	return currentValue;
}

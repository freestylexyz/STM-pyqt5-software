#include "DSP_FUNCTIONS.h"  //DSP specific functions
void analogOut(unsigned int, unsigned int);
void dspOutput(char * ,char *, char , char , char );

unsigned int lastOutput[NUM_OUT_CHANNEL];

//Outputs the 12 LSB of data along with the 4 bits of channel.
void analogOut(unsigned int channel, unsigned int data)
{
	char byteA, byteB;

	byteA = (channel) + ((data & 0x0F00) >> 8);
	byteB = data & 0x00FF;

	dspOutput(0, 0, 'r', byteA, byteB);
}

//Called externally.  Converts inData to channel to output to and data to output.
//Data organized as 0xCCCC dddd dddd dddd.  C is channel bits, d is data bits.
void dspOutput(char * inData, char * outData, char source, char byteA, char byteB)
{
	char maskTargetA , maskTargetB;
	int channel, data;

	switch (source)
	{
	case 'm': //package format: 'ccccdddd dddddddd'
		maskTargetA = inData[0];
		maskTargetB = inData[1];
		break;

		//				cp+channel + initial value + final value
	case 'r': //package format: 'p000cccc' + 'dddddddd dddddddd' + 'dddddddd dddddddd'
		maskTargetA = byteA;
		maskTargetB = byteB;
		break;
	}


//  Initial state: CH0 = 0, CH1 = 0
	GPIO_WritePin(CH_0, 1);
	DELAY_US(1);

	GPIO_WritePin(OUT_DATA_BIT_7, (maskTargetA & 0b10000000) >> 7);
    GPIO_WritePin(OUT_DATA_BIT_6, (maskTargetA & 0b01000000) >> 6);
    GPIO_WritePin(OUT_DATA_BIT_5, (maskTargetA & 0b00100000) >> 5);
    GPIO_WritePin(OUT_DATA_BIT_4, (maskTargetA & 0b00010000) >> 4);
    GPIO_WritePin(OUT_DATA_BIT_3, (maskTargetA & 0b00001000) >> 3);
    GPIO_WritePin(OUT_DATA_BIT_2, (maskTargetA & 0b00000100) >> 2);
    GPIO_WritePin(OUT_DATA_BIT_1, (maskTargetA & 0b00000010) >> 1);
    GPIO_WritePin(OUT_DATA_BIT_0, (maskTargetA & 0b00000001) >> 0);
	DELAY_US(1);

	GPIO_WritePin(CH_0, 0);
	DELAY_US(1);

	GPIO_WritePin(OUT_DATA_BIT_7, (maskTargetB & 0b10000000) >> 7);
	GPIO_WritePin(OUT_DATA_BIT_6, (maskTargetB & 0b01000000) >> 6);
	GPIO_WritePin(OUT_DATA_BIT_5, (maskTargetB & 0b00100000) >> 5);
	GPIO_WritePin(OUT_DATA_BIT_4, (maskTargetB & 0b00010000) >> 4);
	GPIO_WritePin(OUT_DATA_BIT_3, (maskTargetB & 0b00001000) >> 3);
	GPIO_WritePin(OUT_DATA_BIT_2, (maskTargetB & 0b00000100) >> 2);
	GPIO_WritePin(OUT_DATA_BIT_1, (maskTargetB & 0b00000010) >> 1);
	GPIO_WritePin(OUT_DATA_BIT_0, (maskTargetB & 0b00000001) >> 0);
    DELAY_US(1);

	//=========ACKNOWLEDGMENT BLOCK===========
	DELAY_US(1);
	GPIO_WritePin(CH_1, 1);  //acknowledgment bit set to 1 indicates sending completed
	DELAY_US(1);
	GPIO_WritePin(CH_1, 0);  //back to the NULL stage
	//=========END OF ACKNOWLEDGMENT BLOCK===========

	//Save upper bits if output channel == 13.
	if((((unsigned int)(maskTargetA))>>4) == 13)
	{
	    ch13Bits = maskTargetA;
	}

	//Save output value.
	channel = (maskTargetA & 0b11110000) >> 4;
	data = maskTargetA & 0b00001111;
	data = data << 8;
	data += maskTargetB;
	lastOutput[channel] = data;
}

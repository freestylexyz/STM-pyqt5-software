#include "DSP_FUNCTIONS.h"  //DSP specific functions
void dspInput(char * ,char *, char );
unsigned int analogIn(unsigned int);
void outputControlBits(char);

char lastControlBits = 0;
char ch13Bits = 0b11010000;

//Given a channel to read from, this reads two bytes of data, stitches them together,
//and returns them as an unsigned int.
unsigned int analogIn(unsigned int channel)
{
    Uint16 inputMSB;
    Uint16 inputLSB;
    char controlBits;
    char strobe1;
    char strobe2;
	unsigned int inputValue;


	//Step 1: Format input channel data with saved control bits
    controlBits = (lastControlBits & 0b00000011) + (channel<<2);
    lastControlBits = controlBits;

    //Start strobe by outputting channel information to control bits
    GPIO_WritePin(CH_2, (channel & 0b00000001) >> 0);
    GPIO_WritePin(CH_3, (channel & 0b00000010) >> 1);

    //Step 2: Strobe A/D Conversion and Delay
    strobe1 = 0b11011111 & ch13Bits;  //saved CH13 bits

    //strobe2 = 0b00000001 << ((((unsigned int)channel) * 2) + 1);
    switch (channel)
    {
    case 0: strobe2 = 0b00000010;
        break;
    case 1: strobe2 = 0b00001000;
        break;
    case 2: strobe2 = 0b00100000;
        break;
    case 3: strobe2 = 0b10000000;
        break;
    default: strobe2 = 0b00000000;
        break;
    }

    dspOutput(0, 0, 'r', strobe1, strobe2);
    DELAY_US(1);

    strobe2 = 0b00000000;
    dspOutput(0, 0, 'r', strobe1, strobe2);
    DELAY_US(20);


    //Step 3: Read LSBs
    outputControlBits(controlBits);
    GPIO_WritePin(CH_4, 1);//set Channel Bit 4 to 1 in order to get the lower 8 bits
    DELAY_US(1);

    inputLSB = 0;
    inputLSB += GPIO_ReadPin(IN_DATA_BIT_7) << 7;
    inputLSB += GPIO_ReadPin(IN_DATA_BIT_6) << 6;
    inputLSB += GPIO_ReadPin(IN_DATA_BIT_5) << 5;
    inputLSB += GPIO_ReadPin(IN_DATA_BIT_4) << 4;
    inputLSB += GPIO_ReadPin(IN_DATA_BIT_3) << 3;
    inputLSB += GPIO_ReadPin(IN_DATA_BIT_2) << 2;
    inputLSB += GPIO_ReadPin(IN_DATA_BIT_1) << 1;
    inputLSB += GPIO_ReadPin(IN_DATA_BIT_0) << 0;//GET 8 LSB
    DELAY_US(1);

    //Step 4: Read MSBs
    GPIO_WritePin(CH_4, 0);//then set Channel Bit 4 to 0 in order to get the upper 8 bits
    DELAY_US(1);

    inputMSB = 0;
    inputMSB += GPIO_ReadPin(IN_DATA_BIT_7) << 7;
    inputMSB += GPIO_ReadPin(IN_DATA_BIT_6) << 6;
    inputMSB += GPIO_ReadPin(IN_DATA_BIT_5) << 5;
    inputMSB += GPIO_ReadPin(IN_DATA_BIT_4) << 4;
    inputMSB += GPIO_ReadPin(IN_DATA_BIT_3) << 3;
    inputMSB += GPIO_ReadPin(IN_DATA_BIT_2) << 2;
    inputMSB += GPIO_ReadPin(IN_DATA_BIT_1) << 1;
    inputMSB += GPIO_ReadPin(IN_DATA_BIT_0) << 0;//GET 8 MSB

    DELAY_US(1);

    lastControlBits = controlBits;

    //Organize input value for serial output
    inputValue = (inputMSB << 8) + inputLSB;

	return(inputValue);
}

//Called externally.  Converts inData to channel to read from.
//Returns input data to serial.
void dspInput(char * inData, char * outData, char source)
{
	unsigned int i = 0;
	Uint16 inputMSB;
	Uint16 inputLSB;
	unsigned int inputValue;
	char convertedInput[2];
	char channel;

	channel = inData[0];

	inputValue = analogIn(channel);

	//Organize input value for serial output
	inputMSB = (inputValue & 0xFF00) >> 8;
	inputLSB = inputValue & 0x00FF;

	convertedInput[0] = ~inputMSB;
	convertedInput[1] = ~inputLSB;

	memcpy(outData,convertedInput,OUT_INPUT_CH_SIZE); //copy data to outData vector

	for(i=0;i<OUT_INPUT_CH_SIZE;i++)
	{
	    serialOut_char(outData[i]); //send outData vector via serial
	}
}

//Update all control bits with new data given by bits
void outputControlBits(char bits)
{
    GPIO_WritePin(CH_0, (bits & 0b00000001) >> 0);
    GPIO_WritePin(CH_1, (bits & 0b00000010) >> 1);
    GPIO_WritePin(CH_2, (bits & 0b00000100) >> 2);
    GPIO_WritePin(CH_3, (bits & 0b00001000) >> 3);
    GPIO_WritePin(CH_4, (bits & 0b00010000) >> 4);
    GPIO_WritePin(CH_5, (bits & 0b00100000) >> 5);
}

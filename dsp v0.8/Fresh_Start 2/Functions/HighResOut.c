#include "DSP_FUNCTIONS.h"  //DSP specific functions
void highRes (char * ,char * );
void higherRes (char * ,char * );
void highResOutput (unsigned int , unsigned int );
void higherResOutput (unsigned long);
void output20BitControlBits(char);
void higherResSerialOut(unsigned int, unsigned int);

unsigned int savedHighResBits;

//###########################################################################
// FUNCTION NAME: HIGH RES
// DESCRIPTION: Receives 16 bits of data and 4 bits for the channel and calls highResOutput subroutine for
// 16 bit output.
// EXPECTED PARAMETERS:
// RETURN:
//###########################################################################

void highRes(char * inData, char * outData)
{
    unsigned int dataBits;
    unsigned int channelBits;

    dataBits = (((unsigned int)inData[0]) << 8) + (unsigned int)inData[1];
    channelBits = inData[2];

    highResOutput(dataBits, channelBits);
}

void highResOutput(unsigned int data, unsigned int channel)
{
    unsigned int upperBits;
    unsigned int lowerBits;
    char dataMSB;
    char dataLSB;

    lowerBits = (data & 0b0000111111111111);
    upperBits = (data & 0b1111000000000000);

    if(channel == 10)
    {
        dataMSB = 0b11110000 + ((char)(upperBits >> 12));
        dataLSB = (char)(savedHighResBits & 0b0000000011110000);

        savedHighResBits = (((unsigned int)(dataMSB & 0b00001111)) << 8) + (unsigned int)dataLSB;

        dspOutput(0, 0, 'r', dataMSB, dataLSB);

        dataMSB = (char)(0b10100000 + ((lowerBits & 0b0000111100000000) >> 8));
        dataLSB = (char)(lowerBits & 0b0000000011111111);
        dspOutput(0, 0, 'r', dataMSB, dataLSB);
    }
    else if (channel == 11)
    {
        dataLSB = (char)(upperBits >> 8);
        dataMSB = 0b11110000 + (char)((savedHighResBits & 0b0000111100000000) >> 8);

        savedHighResBits = (((unsigned int)(dataMSB & 0b00001111)) << 8) + (unsigned int)dataLSB;

        dspOutput(0, 0, 'r', dataMSB, dataLSB);

        dataMSB = (char)(0b10110000 + ((lowerBits & 0b0000111100000000) >> 8));
        dataLSB = (char)(lowerBits & 0b0000000011111111);
        dspOutput(0, 0, 'r', dataMSB, dataLSB);
    }
    else
    {
        dataMSB = (char)((channel << 4) + ((lowerBits & 0b0000111100000000) >> 8));
        dataLSB = (char)(lowerBits & 0b0000000011111111);
        dspOutput(0, 0, 'r', dataMSB, dataLSB);
    }
}

//###########################################################################
// FUNCTION NAME: HIGHER RES
// DESCRIPTION: Receives 20 bits of data and calls higherResOutput subroutine for
// 20 bit bias output.
// EXPECTED PARAMETERS:
// RETURN:
//###########################################################################

void higherRes(char * inData, char * outData)
{
    unsigned long dataBits;

    dataBits = inData[0];
    dataBits = dataBits << 8;
    dataBits += inData[1];
    dataBits = dataBits << 8;
    dataBits += inData[2];

    higherResOutput(dataBits);
}

void higherResOutput(unsigned long data)
{
    unsigned long invertedData;
    unsigned int lowerBits;
    unsigned int upperBits;
    unsigned int firstBit;
    unsigned int secondBit;
    int i;
    int shift;

    invertedData = 0b000011111111111111111111 - data + 1;

    lowerBits = ((invertedData & 0b000000000000000000111111) << 5);
    upperBits = ((invertedData & 0b000011111111111111000000) >> 4);

    output20BitControlBits(0b00001000);
    //DELAY_US(1);
    output20BitControlBits(0b00000000);
    //DELAY_US(1);

    for(i = 0; i < 16; i++)
    {
        shift = 15 - i;
        firstBit = upperBits >> shift;
        firstBit = firstBit & 0x00000001;
        secondBit = lowerBits >> shift;
        secondBit = secondBit & 0x00000001;
        higherResSerialOut(firstBit, secondBit);
    }

    output20BitControlBits(0b00000000);
    //DELAY_US(1);
    output20BitControlBits(0b00001000);
    //DELAY_US(1);
}

void output20BitControlBits(char bits)
{
    GPIO_WritePin(HIGH_RES_DAC_0, (bits & 0b00000001) >> 0);
    GPIO_WritePin(HIGH_RES_DAC_1, (bits & 0b00000010) >> 1);
    GPIO_WritePin(HIGH_RES_DAC_2, (bits & 0b00000100) >> 2);
    GPIO_WritePin(HIGH_RES_DAC_3, (bits & 0b00001000) >> 3);
    GPIO_WritePin(HIGH_RES_DAC_4, (bits & 0b00010000) >> 4);
    //GPIO_WritePin(HIGH_RES_DAC_5, (bits & 0b00100000) >> 5);

    /*
    HIGH_RES_DAC_0 = (bits & 0b00000001);
    HIGH_RES_DAC_1 = ((bits & 0b00000010) >> 1);
    HIGH_RES_DAC_2 = ((bits & 0b00000100) >> 2);
    HIGH_RES_DAC_3 = ((bits & 0b00001000) >> 3);
    HIGH_RES_DAC_4 = ((bits & 0b00010000) >> 4);
    //HIGH_RES_DAC_5 = ((bits & 0b00100000) >> 5);

    */
}

void higherResSerialOut(unsigned int upper, unsigned int lower)
{
    char output;

    output = (char)((upper << 2) | (lower << 1));
    output = output & 0b00000110;

    output20BitControlBits(output);
    //DELAY_US(1);

    output = output | 0b00000001;

    output20BitControlBits(output);
    //DELAY_US(1);
}

#include "DSP_FUNCTIONS.h"  //DSP specific functions

extern data readData;

void serialOut_char(char byte)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    SciaRegs.SCITXBUF.all = byte;
}

//Functions like SERIAL_TWO
void serialOut_two(unsigned int bytesOut)
{
	char byteA, byteB;

	byteA = bytesOut & 0x00FF;
	byteB = (bytesOut & 0xFF00)>>8;
	
	serialOut_char(byteA);
	serialOut_char(byteB);
}

//Functions like SERIAL_WORD
void serialOut_word(unsigned long bytesOut)
{
	char byteA, byteB, byteC;
	
	byteA = (char)(bytesOut & 0x000000FF);
	byteB = (char)((bytesOut & 0x0000FF00)>>8);
	byteC = (char)((bytesOut & 0x00FF0000)>>16);
	
	serialOut_char(byteC);
	serialOut_char(byteB);
	serialOut_char(byteA);
}
	
//This function acts like XDSO.  Everything in bytesOut is sent via serial 3 bytes at a time, LSB first
void serialOut_xdso()
{

    unsigned int i;
    unsigned int end = readData.numPoints;
    unsigned long firstWord;

    firstWord = ((unsigned long)DSPCMD_RAMPREAD);
    firstWord = firstWord << 8;
    firstWord += ((readData.numPoints) & 0x0000FF00) >> 8;
    firstWord = firstWord << 8;
    firstWord += (readData.numPoints) & 0x000000FF;

    serialOut_word(firstWord);

    for (i = 0; i < end; i++)
    {
        serialOut_word(readData.data[i]);
    }

    readData.numPoints = 0;
}

void serialOut_array(char * bytesToSend, unsigned int numBytes)
{
    unsigned int i = 0;
    for(i = 0; i < numBytes; i++)
    {
        serialOut_char(bytesToSend[i]);
    }
}

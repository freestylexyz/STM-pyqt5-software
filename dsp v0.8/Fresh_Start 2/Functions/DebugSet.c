#include "DSP_FUNCTIONS.h"  //DSP specific functions
void debugSet(char *, char *);


void debugSet(char * inData, char * outData)
{
	unsigned int address;
	unsigned long memoryData;
	
	//receive two bytes from serial (RECEIVETWO) to use as address
    address = inData[0];
    address = address << 8;
    address += inData[1];
	
	//receive three more bytes from serial (RECEIVETHREE) to use as value
    memoryData = inData[2];
    memoryData = memoryData << 8;
    memoryData += inData[3];
    memoryData = memoryData << 8;
    memoryData += inData[4];
	
	//Store the value in the memory address given by address.

}

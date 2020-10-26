#include "DSP_FUNCTIONS.h"  //DSP specific functions

void debugGet(char * inData, char * outData)
{
	unsigned int address;
	unsigned long memoryData;
	
	//get two bytes from serial.
	address = inData[0];
	address = address << 8;
	address += inData[1];
	
	//use these bytes as address to pull memory
	//memoryData = address;
	
	//output the value using SERIAL_WORD
	serialOut_word(memoryData);
}

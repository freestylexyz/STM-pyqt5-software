#include "DSP_FUNCTIONS.h"  //DSP specific functions

//Receive one bit from serial.
//This function is not needed much since most of the serial input is done before function call.

Uint16 serialIn()
{
    // Wait for inc character
    while(SciaRegs.SCIFFRX.bit.RXFFST == 0) { } // wait for empty state

    return(SciaRegs.SCIRXBUF.all); //Read first character received to get command character
}

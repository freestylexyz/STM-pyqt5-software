/*
 * Serial.c
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#include "Serial.h"

//
//  scia_echoback_init - Test 1,SCIA  DLB, 8-bit word, baud rate 0x000F,
//                       default, 1 STOP bit, no parity
//
void scia_echoback_init()
{
    //
    // Note: Clocks were turned on to the SCIC peripheral
    // in the InitSysCtrl() function
    //

    SciaRegs.SCICCR.all = 0x0007;   // 1 stop bit,  No loopback
                                    // No parity,8 char bits,
                                    // async mode, idle-line protocol
    SciaRegs.SCICTL1.all = 0x0003;  // enable TX, RX, internal SCICLK,
                                    // Disable RX ERR, SLEEP, TXWAKE
    SciaRegs.SCICTL2.all = 0x0003;
    SciaRegs.SCICTL2.bit.TXINTENA = 1;
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;

    //
    // SCIC at 38400 baud
    // @LSPCLK = 50 MHz (200 MHz SYSCLK) HBAUD = 0x00 and LBAUD = 0xA2.
    //
    SciaRegs.SCIHBAUD.all = 0x0000;
    SciaRegs.SCILBAUD.all = 0x00A2;

    SciaRegs.SCICTL1.all = 0x0023;  // Relinquish SCI from Reset
}

//
// scia_fifo_init - Initialize the SCI FIFO
//
void scia_fifo_init()
{
    SciaRegs.SCIFFTX.all = 0xE040;
    SciaRegs.SCIFFRX.all = 0x2044;
    SciaRegs.SCIFFCT.all = 0x0;
}

//
// serialIn - Read data through serial
//
char serialIn()
{
    // Wait for inc character
    while(SciaRegs.SCIFFRX.bit.RXFFST == 0) { } // wait for empty state
    return(SciaRegs.SCIRXBUF.all);              //Read first character received to get command character
}


//
// serialOut_char - Output a one-byte data through serial
//
void serialOut_char(char byte)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    SciaRegs.SCITXBUF.all = byte;
}

//
// serialOut_two - Output a two-byte data through serial
//
void serialOut_two(Uint16 bytesOut)
{
	char byteA, byteB;

	byteA = (bytesOut & 0xFF00)>>8;
	byteB = bytesOut & 0x00FF;
	
	serialOut_char(byteA);
	serialOut_char(byteB);
}

//
// serialOut_word - Output a three-byte data through serial
//
void serialOut_word(Uint32 bytesOut)
{
	char byteA, byteB, byteC;
	
	byteA = (char)(bytesOut & 0x000000FF);
	byteB = (char)((bytesOut & 0x0000FF00)>>8);
	byteC = (char)((bytesOut & 0x00FF0000)>>16);
	
	serialOut_char(byteA);
	serialOut_char(byteB);
	serialOut_char(byteC);
}


//
// End of file
//

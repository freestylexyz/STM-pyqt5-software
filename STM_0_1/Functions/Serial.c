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
    // @LSPCLK = 50 MHz (200 MHz SYSCLK) HBAUD = 0x00 and LBAUD = 0x50. RAM
    // @LSPCLK = 50 MHz (200 MHz SYSCLK) HBAUD = 0x00 and LBAUD = 0xA2. FLASH
    // @LSPCLK = 200 MHz (200 MHz SYSCLK) HBAUD = 0x01 and LBAUD = 0x45. RAM
    // @LSPCLK = 200 MHz (200 MHz SYSCLK) HBAUD = 0x02 and LBAUD = 0x8A. FLASH
    //
    SciaRegs.SCIHBAUD.all = 0x0002;
    SciaRegs.SCILBAUD.all = 0x008A;

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
// serialIn - Read 'byte_num' number of byte through serial. 4 byte maximum, MSB first
//
byte serialIn(Uint16 byte_num)
{
    Uint16 i;
    byte bytearray;

    if(byte_num > 4)
    {
        byte_num = 4;                                       // Limit the max reading byte number to 4
    }

    bytearray.num = byte_num;

    // Read  bytes, MSB first
    for(i = 0; i < byte_num; i++)
    {
        while(SciaRegs.SCIFFRX.bit.RXFFST == 0) { }         // Wait for empty state
        bytearray.arr[i] = SciaRegs.SCIRXBUF.all;           // Read 1 byte data
    }

    return bytearray;
}


//
// serialOut - Output byte array through serial. MSB first
//
void serialOut(byte bytearray)
{
    Uint16 i;
    // Send  bytes, MSB first
    for(i = 0; i < bytearray.num; i++)
    {
        while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
        SciaRegs.SCITXBUF.all = bytearray.arr[i];
    }
}

//
// serialOutArray - Output an array of Uint16 data. 2048 data points maximum
//
void serialOutArray(Uint16 * array, Uint16 n)
{
    Uint16 i;
    for(i = 0; i < n; i++){serialOut(split(array[i], 2));}      // Transmit data
}

//
// serialOutCharArray - Output an array of Char data. 2048 data points maximum
//
void serialOutCharArray(char * array, Uint16 n)
{
    Uint16 i;
    for(i = 0; i < n; i++){serialOut(split(array[i], 1));}              // Transmit data
}

//
// serialCheck - Check if received any data. This function need to corporate with Start and Finish command
//
char serialCheck()
{
    char rdata = Null;                              // 0x00 is Null code (received nothing)
    if(SciaRegs.SCIFFRX.bit.RXFFST != 0)            // Check if SCI has received a character
    {
        rdata = SciaRegs.SCIRXBUF.all;              // Read the character
    }
    return rdata;
}

//
// serialEcho - Echo data back
//
void serialEcho()
{
    byte sdata;
    Uint16 number;
    serialOut(split(Start, 1));         // Send out start command
    while(true)
    {
        sdata = serialIn(1);
        number = combine(sdata);
        if(combine(sdata) == Stop){break;}
        else{serialOut(split(number, 1));}
    }
    serialOut(split(Finish, 1));                    // Send out finish command
}

//
// End of file
//

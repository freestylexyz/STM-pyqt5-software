//###########################################################################
//
// FILE:    Example_2837xDSci_Echoback.c
//
// TITLE:   SCI Echoback.
//
//! \addtogroup cpu01_example_list
//! <h1>SCI Echoback (sci_echoback)</h1>
//!
//!  This test receives and echo-backs data through the SCI-A port.
//!
//!  The PC application 'hyperterminal' or another terminal
//!  such as 'putty' can be used to view the data from the SCI and
//!  to send information to the SCI.  Characters received
//!  by the SCI port are sent back to the host.
//!
//!  \b Running \b the \b Application
//!  -# Configure hyperterminal or another terminal such as putty:
//!
//!  For hyperterminal you can use the included hyperterminal configuration
//!  file SCI_96.ht.
//!  To load this configuration in hyperterminal
//!    -# Open hyperterminal
//!    -# Go to file->open
//!    -# Browse to the location of the project and
//!       select the SCI_96.ht file.
//!  -# Check the COM port.
//!  The configuration file is currently setup for COM1.
//!  If this is not correct, disconnect (Call->Disconnect)
//!  Open the File-Properties dialogue and select the correct COM port.
//!  -# Connect hyperterminal Call->Call
//!  and then start the 2837xD SCI echoback program execution.
//!  -# The program will print out a greeting and then ask you to
//!  enter a character which it will echo back to hyperterminal.
//!
//!  \note If you are unable to open the .ht file, or you are using
//!  a different terminal, you can open a COM port with the following settings
//!  -  Find correct COM port
//!  -  Bits per second = 9600
//!  -  Date Bits = 8
//!  -  Parity = None
//!  -  Stop Bits = 1
//!  -  Hardware Control = None
//!
//!  \b Watch \b Variables \n
//!  - LoopCount - the number of characters sent
//!
//! \b External \b Connections \n
//!  Connect the SCI-A port to a PC via a transceiver and cable.
//!  - GPIO28 is SCI_A-RXD (Connect to Pin3, PC-TX, of serial DB9 cable)
//!  - GPIO29 is SCI_A-TXD (Connect to Pin2, PC-RX, of serial DB9 cable)
//!
//
//###########################################################################
// $TI Release: F2837xD Support Library v3.06.00.00 $
// $Release Date: Mon May 27 06:48:24 CDT 2019 $
// $Copyright:
// Copyright (C) 2013-2019 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//###########################################################################

//
// Included Files
//
#include "F28x_Project.h"
#include "DSP_FUNCTIONS.h"

//
// Globals
//
Uint16 LoopCount;

//
// Function Prototypes
//
void scia_echoback_init(void);
void scia_fifo_init(void);
void updateSerial(void);

//
// Main
//
void main(void)
{
    int i;
//
// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the F2837xD_SysCtrl.c file.
//
   InitSysCtrl();

//
// Step 2. Initialize GPIO:
// This example function is found in the F2837xD_Gpio.c file and
// illustrates how to set the GPIO to it's default state.
//
   InitGpio();

//
// For this version, initialize the pins for the SCI-A port first.
//  GPIO_SetupPinMux() - Sets the GPxMUX1/2 and GPyMUX1/2 register bits
//  GPIO_SetupPinOptions() - Sets the direction and configuration of the GPIOS
// These functions are found in the F2837xD_Gpio.c file.

   pinInit();

//
// Step 3. Clear all __interrupts and initialize PIE vector table:
// Disable CPU __interrupts
//
   DINT;

//
// Initialize PIE control registers to their default state.
// The default state is all PIE __interrupts disabled and flags
// are cleared.
// This function is found in the F2837xD_PieCtrl.c file.
//
   InitPieCtrl();

//
// Disable CPU __interrupts and clear all CPU __interrupt flags:
//
   IER = 0x0000;
   IFR = 0x0000;

//
// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the __interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in F2837xD_DefaultIsr.c.
// This function is found in F2837xD_PieVect.c.
//
   InitPieVectTable();

//
// Step 4. User specific code:
//
   LoopCount = 0;

   scia_fifo_init();       // Initialize the SCI FIFO
   scia_echoback_init();   // Initialize SCI for echoback

   for(i = 0; i < NUM_OUT_CHANNEL; i++)
   {
       lastOutput[i] = i;
   }


   while(1)
   {
       updateSerial();
   }
}

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
// updateSerial - Receive command character from SCIA and run paired function.
//

void updateSerial(void)
{
    Uint16 i = 0, j = 0, inFifoSize = 1, outFifoSize = 1;
    Uint16 inTypeTest = 'A';
    char *g_inData, *g_outData;
    bool cmdGood = 1;

    inTypeTest = serialIn(); //Read first character received to get command character

    switch(inTypeTest) //test first character to define how many extra characters the program should wait for
    {
    case DSPCMD_DELAY:
        inFifoSize = IN_DELAY_SIZE;
        outFifoSize = OUT_DELAY_SIZE;
        break;
    case DSPCMD_INPUT:
        inFifoSize = IN_INPUT_CH_SIZE;
        outFifoSize = OUT_INPUT_CH_SIZE;
        break;
    case DSPCMD_OUTPUT:
        inFifoSize = IN_OUTPUT_SIZE;
        outFifoSize = OUT_OUTPUT_SIZE;
        break;
    case DSPCMD_VERSION:
        inFifoSize = IN_VERSION_SIZE;
        outFifoSize = OUT_VERSION_SIZE;
        break;
    case DSPCMD_RAMP:
        inFifoSize = IN_RAMP_SIZE;
        outFifoSize = OUT_RAMP_SIZE;
        break;
    case DSPCMD_RAMPREAD:
        inFifoSize = IN_RAMP_READ_SIZE;
        outFifoSize = OUT_RAMP_READ_SIZE;
        break;
    case DSPCMD_SQUAREWAVE:
        inFifoSize = IN_SQUARE_SIZE;
        outFifoSize = OUT_SQUARE_SIZE;
        break;
    case DSPCMD_TIPSETUP:
        inFifoSize = IN_TIPSETUP_SIZE;
        outFifoSize = OUT_TIPSETUP_SIZE;
        break;
    case DSPCMD_GIANTSTEP:
        inFifoSize = IN_GIANT_SIZE;
        outFifoSize = OUT_GIANT_SIZE;
        break;
    case DSPCMD_TRANSLATE_SAMPLE:
        inFifoSize = IN_TRANSLATE_SAMPLE_SIZE;
        outFifoSize = OUT_TRANSLATE_SAMPLE_SIZE;
        break;
    case DSPCMD_TIPAPPROACH:
        inFifoSize = IN_APPROACH_SIZE;
        outFifoSize = OUT_APPROACH_SIZE;
        break;
    case DSPCMD_AUTOPLUS:
        inFifoSize = IN_AUTO_SIZE;
        outFifoSize = OUT_AUTO_SIZE;
        break;
    case DSPCMD_AUTOMINUS:
        inFifoSize = IN_AUTO_SIZE;
        outFifoSize = OUT_AUTO_SIZE;
        break;
    case DSPCMD_HIGHRES:
        inFifoSize = IN_HIGHRES_SIZE;
        outFifoSize = OUT_HIGHRES_SIZE;
        break;
    case DSPCMD_HIGHERRES:
        inFifoSize = IN_HIGHERRES_SIZE;
        outFifoSize = OUT_HIGHERRES_SIZE;
        break;
    case DSPCMD_SCANSETUP:
        inFifoSize = IN_SCANSETUP_SIZE;
        outFifoSize = OUT_SCANSETUP_SIZE;
        break;
    case DSPCMD_SCANLINE:
        inFifoSize = IN_SCANLINE_SIZE;
        //outFifoSize = OUT_SCANLINE_SIZE;
        break;
    case DSPCMD_TRACKSETUP:
        inFifoSize = IN_TRACKSETUP_SIZE;
        outFifoSize = OUT_TRACKSETUP_SIZE;
        break;
    case DSPCMD_TRACK:
        inFifoSize = IN_TRACK_SIZE;
        outFifoSize = OUT_TRACK_SIZE;
        break;
    case DSPCMD_RSSETUP:
        //inFifoSize = IN_RSSETUP_SIZE;
        inFifoSize = (3 * serialIn()) + 1;  //Size of incoming data is variable.
        outFifoSize = OUT_RSSETUP_SIZE;
        break;
    case DSPCMD_SPSETUP:
        inFifoSize = IN_SPSETUP_SIZE;
        outFifoSize = OUT_SPSETUP_SIZE;
        break;
    case DSPCMD_SPEC:
        inFifoSize = IN_SPEC_SIZE;
        outFifoSize = OUT_SPEC_SIZE;
        break;
    case DSPCMD_DIAGSETUP:
        inFifoSize = IN_DIAGSETUP_SIZE;
        outFifoSize = OUT_DIAGSETUP_SIZE;
        break;
    case DSPCMD_DIAG:
        inFifoSize = IN_DIAG_SIZE;
        outFifoSize = OUT_DIAG_SIZE;
        break;
    case DSPCMD_HOPSETUP:
        inFifoSize = IN_HOPSETUP_SIZE;
        outFifoSize = OUT_HOPSETUP_SIZE;
        break;
    case DSPCMD_HOP:
        inFifoSize = IN_HOP_SIZE;
        outFifoSize = OUT_HOP_SIZE;
        break;
    case DSPCMD_DEPSETUP:
        inFifoSize = IN_DEPSETUP_SIZE;
        outFifoSize = OUT_DEPSETUP_SIZE;
        break;
    case DSPCMD_DEP:
        inFifoSize = IN_DEP_SIZE;
        outFifoSize = OUT_DEP_SIZE;
        break;
    case DSPCMD_DEP20SETUP:
        inFifoSize = IN_DEP20SETUP_SIZE;
        outFifoSize = OUT_DEP20SETUP_SIZE;
        break;
    case DSPCMD_DEP20:
        inFifoSize = IN_DEP20_SIZE;
        outFifoSize = OUT_DEP20_SIZE;
        break;
    /*
    case DSPCMD_DAC:
        inFifoSize = IN_DAC_SIZE;
        outFifoSize = OUT_DAC_SIZE;
        break;
    */
    case DSPCMD_DEBUGGET:
        inFifoSize = IN_DEBUGGET_SIZE;
        outFifoSize = OUT_DEBUGGET_SIZE;
        break;
    case DSPCMD_DEBUGSET:
        inFifoSize = IN_DEBUGSET_SIZE;
        outFifoSize = OUT_DEBUGSET_SIZE;
        break;
    default:
        cmdGood = NULL;
    }

    if (cmdGood)
    {
    g_inData = (char*)calloc(inFifoSize, sizeof(char));
    g_outData = (char*)calloc(outFifoSize, sizeof(char));

    //wait until there is nothing else to be received (check if all the expected characters were received)
    //while(FIFO_IN_DATA_AVAILABLE_SIZE < inFifoSize) {}
    //wait to make sure there's no more data being sent to in fifo
    //while(FIFO_IN_DATA_AVAILABLE == 1) {}

    for(j=0;j<inFifoSize;j++) //loop reading data while index shorter than IN_FIFO size
    {
        g_inData[j] = serialIn(); //read in fifo
    }

    //INSERT MENU HERE
    switch(inTypeTest) //check what kind of command has been received
    {
    case DSPCMD_DELAY:
        delay(g_inData,g_outData,0);
        break;
    case DSPCMD_INPUT:
        dspInput(g_inData,g_outData,'m');
        break;
    case DSPCMD_OUTPUT:
        dspOutput(g_inData,g_outData,'m',0,0);
        break;
    case DSPCMD_VERSION:
        version(g_inData,g_outData);
        break;
    case DSPCMD_RAMP:
        ramp(g_inData,g_outData);
        break;
    case DSPCMD_RAMPREAD:
        rampRead(g_inData,g_outData);
        break;
    case DSPCMD_SQUAREWAVE:
        squareWave(g_inData,g_outData);
        break;
    case DSPCMD_TIPSETUP:
        tipSetup(g_inData,g_outData);
        break;
    case DSPCMD_GIANTSTEP:
        giantStep(g_inData,g_outData);
        break;
    case DSPCMD_TRANSLATE_SAMPLE:
        translateSample(g_inData,g_outData);
        break;
    case DSPCMD_TIPAPPROACH:
        tipApproach(g_inData,g_outData);
        break;
    case DSPCMD_AUTOPLUS:
        autoPlus(g_inData,g_outData);
        break;
    case DSPCMD_AUTOMINUS:
        //autoPlus(g_inData,g_outData);
        autoMinus(g_inData,g_outData);
        break;
    case DSPCMD_HIGHRES:
        highRes(g_inData,g_outData);
        break;
    case DSPCMD_HIGHERRES:
        higherRes(g_inData,g_outData);
        break;
    case DSPCMD_SCANSETUP:
        scanSetup(g_inData,g_outData);
        break;
    case DSPCMD_SCANLINE:
        scanLine(g_inData,g_outData);
        break;
    case DSPCMD_TRACKSETUP:
        trackSetup(g_inData,g_outData);
        break;
    case DSPCMD_TRACK:
        track(g_inData,g_outData);
        break;
    case DSPCMD_RSSETUP:
        readSeqSetup(g_inData, g_outData, ((inFifoSize - 1) / 3));
        break;
    case DSPCMD_SPSETUP:
        specSetup(g_inData,g_outData);
        break;
    case DSPCMD_SPEC:
        spec(g_inData,g_outData);
        break;
    case DSPCMD_DIAGSETUP:
        rampDiagSetup(g_inData,g_outData);
        break;
    case DSPCMD_DIAG:
        rampDiag(g_inData,g_outData);
        break;
    case DSPCMD_HOPSETUP:
        hopSetup(g_inData,g_outData);
        break;
    case DSPCMD_HOP:
        hop(g_inData,g_outData);
        break;
    case DSPCMD_DEPSETUP:
        depSetup(g_inData,g_outData);
        break;
    case DSPCMD_DEP:
        deposition(g_inData,g_outData);
        break;
    case DSPCMD_DEP20SETUP:
        dep20Setup(g_inData,g_outData);
        break;
    case DSPCMD_DEP20:
        deposition20(g_inData,g_outData);
        break;
    /*
    case DSPCMD_DAC:
        //dac(g_inData,g_outData);
        dac();
        break;
    */
    case DSPCMD_DEBUGGET:
        debugGet(g_inData,g_outData);
        break;
    case DSPCMD_DEBUGSET:
        debugSet(g_inData,g_outData);
        break;
    default:
        for(i=0;i<inFifoSize;i++) //clear inData vector
        {
            g_inData[i] = NULL_VALUE; //if we see a % mark, it is indicating an error or null value
        }
        for(i=0;i<outFifoSize;i++) //clear outData vector
        {
            g_outData[i] = NULL_VALUE;
        }
    }

    //clear data vectors after usage
    for(i=0;i<inFifoSize;i++) //clear inData vector
    {
        g_inData[i] = NULL_VALUE; //if we see a % mark, it is indicating an error or null value
    }
    for(i=0;i<outFifoSize;i++) //clear outData vector
    {
        g_outData[i] = NULL_VALUE;
    }

    free(g_inData);     //Free allocated memory now that values are cleared.
    free(g_outData);
    }
    SciaRegs.SCIFFRX.bit.RXFIFORESET = 0; //points RX FIFO to address 0 again and start overwriting values
    SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;
}


//
// End of file
//

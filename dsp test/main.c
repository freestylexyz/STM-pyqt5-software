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
// Main
//
void main(void)
{
    char inTypeTest;
    char byteA, byteB, byteC;
    char bytea, byteb, bytec;
    Uint16 sdata, rdata, command;
    char addr;
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

//   Enable 200MHz LSPCLK
//   EALLOW;
//   ClkCfgRegs.LOSPCP.bit.LSPCLKDIV=0;
//   EDIS;


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

   spi_fifo_init();         // Initialize SPI FIFO
   scia_fifo_init();        // Initialize the SCI FIFO
   scia_echoback_init();    // Initialize SCI for echo-back

   bit20_RST_H();           // Hardware reset 20bit DAC
   dac_RST_H();             // Hardware reset DAC
   adc_command(ADC_RST);    // Software reset ADC

   DELAY_US(100000);        // Wait 100ms

   bit20Init();             // Initialize 20bit DAC
   dacInit();               // Initialize DAC
   adcInit();               // Initialize ADC

   DELAY_US(100000);        // Wait 100ms
                            // Human reaction time is longer than 200ms:)

   dac_W(ISET_OFFSET, 0X8167);   // Write ISET offset (need to be measured for each electronics)
   dac_W(BIAS_OFFSET, 0X8004);   // Write BIAS offset (need to be measured for each electronics)


   for(;;)
   {
       inTypeTest = 0x00;
       inTypeTest = serialIn(); // Read first character received to get command character


//############################################################################################################

//       byteA = serialIn();
//       byteB = serialIn();
//       byteC = serialIn();

//       serialOut_char(inTypeTest);
//
//       byteA = serialIn();                         // Obtain most significant byte for channel address
//       command = ((byteA & 0x00FF) << 8);          // Patch command data
//
//       serialOut_two(command);

//       rdata = adc_CNV(command);                   // Return conversion result of corresponding channel
//
//       bytea = (char)((rdata & 0xFF00) >> 8);      // Most significant byte of return data
//       byteb = (char)(rdata & 0x00FF);             // Least significant byte of return data
//       serialOut_char(bytea);                      // Return most significant byte first
//       serialOut_char(byteb);                      // Return least significant byte second

//############################################################################################################

       switch(inTypeTest)       // Execute command
           {
       case DAC_W_DSP:
           dac_W_DSP();
           break;
       case DAC_R_DSP:
           dac_R_DSP();
           break;
       case DAC_RANGE:
           dacRange();
           break;
       case BIT20_W_DSP:
           bit20_W_DSP();
           break;
       case BIT20_R_DSP:
           bit20_R_DSP();
           break;
       case ADC_DSP:
           adc_DSP();
           break;
       case ADC_N_DSP:
           adc_N_DSP();
           break;
       case ADC_W_DSP:
           adc_W_DSP();
           break;
       case ADC_R_DSP:
           adc_R_DSP();
           break;
       case DIGITALO_DSP:
           digitalO_DSP();
           break;
       case GAIN_DSP:
           gain_DSP();
           break;
       case DIGITALSCAN_DSP:
           digitalScan_DSP();
           break;
       default:
           serialOut_char(inTypeTest);
           }
       LoopCount++;
   }
}




//
// End of file
//

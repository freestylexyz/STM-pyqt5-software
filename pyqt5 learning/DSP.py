# -*- coding: utf-8 -*-
"""
Created on Tue Oct 13 11:40:08 2020

@author: yaoji
"""

import serial

class dsp(serial.Serial):
    
    #
    # constats
    #
    
    # STM output channels
    XIN = 0x10
    XOFFSET = 0x11
    ZOFFSET_FINE = 0x12
    ZOFFSET = 0x13
    # ISET_OFFSET = 0x14
    ISET = 0x15
    ZOUTER = 0x1a
    # BIAS_OFFSET = 0x1c
    BIAS = 0x1d
    YOFFSET = 0x1e
    YIN = 0x1f
    
    # Auxiliary DAC channels
    DAC_6 = 0x16
    DAC_7 = 0x17
    DAC_8 = 0x18
    DAC_9 = 0x19
    DAC_11 = 0x1b
    
    # DAC range
    DAC_U5 = 0b0000          # Unipolar 5V span
    DAC_U10 = 0b0001         # Unipolar 10V span
    DAC_U20 = 0b0010         # Unipolar 20V span
    DAC_U40 = 0b0100         # Unipolar 40V span
    DAC_B10 = 0b1001         # Bipolar 10V span
    DAC_B20 = 0b1010         # Bipolar 20V span
    DAC_B5 = 0b1110          # Bipolar 5V span
    
    # 20bit-DAC DAC address
    BIAS_20 = 0x10
    
    # STM input channels
    ZOUT = 0xd8
    PREAMP = 0xdc
    
    # All ADC channels
    ADC_0 = 0xc0
    ADC_1 = 0xc4
    ADC_2 = 0xc8
    ADC_3 = 0xcc
    ADC_4 = 0xd0
    ADC_5 = 0xd4
    ADC_6 = 0xd8
    ADC_7 = 0xdc
    
    # ADC range register address
    ADC_RANGE0 = (0x05 << 1)
    ADC_RANGE1 = (0x06 << 1)
    ADC_RANGE2 = (0x07 << 1)
    ADC_RANGE3 = (0x08 << 1)
    ADC_RANGE4 = (0x09 << 1)
    ADC_RANGE5 = (0x0A << 1)
    ADC_RANGE6 = (0x0B << 1)
    ADC_RANGE7 = (0x0C << 1)
    
    # ADC range
    ADC_B20 = 0x00          # Input range +/-10.24V
    ADC_B10 = 0x01          # Input range +/-5.12V
    ADC_B5 = 0x02           # Input range +/-2.56V
    ADC_U10 = 0x05          # Input range 0V to 10.24V
    ADC_U5 = 0x06           # Input range 0V to 5.12V
    
    # Digital channel
    DITHER_0 = 0
    DITHER_1 = 1
    FEEDBACK = 2
    RETRACT = 3
    COARSE = 4
    TRANSLATION = 5
    
    # Digital command
    ON = 1
    OFF = 0
    
    # Gain channel
    XGAIN = 0
    YGAIN = 1
    Z1GAIN = 2
    Z2GAIN = 3

    # Gain command
    GIAN0_1 = 0         # gain 0.1
    GAIN1 = 2           # gain 1
    GAIN10 =3           # gain 10
    
    # Digital tip approach/scan command
    APPROACH = 1
    SCAN = 0
    
    #
    # __init__ - This function intial dsp class that inherit from serial.Serial
    # Default port is none, default baudrate is 9600
    #
    def __init__(self, port=None, baudrate=9600):
        serial.Serial.__init__(self, port, baudrate)
    
    #
    # echo - This function echo back the written 1 byte data 
    #
    def echo(self, sdata):
        self.write(sdata)
        return self.read()
    
    #
    # dac_W - This function command dsp to write data (16-bit) to a specific DAC register (6-bit address)
    # 
    def dac_W(self, addr, data):
        sdata = ((addr & 0x0000003f) << 16) | (data & 0xffff)      # patch tx data
        self.write(int(0xd0).to_bytes(1, byteorder="big"))         # 0xd0 for write operation
        self.write(int(sdata).to_bytes(3, byteorder="big"))        # send data
        
    #
    # dac_R - This function command dsp to read data (16-bit) form a specific DAC register (6-bit address)
    #
    def dac_R(self, addr):
        self.write(int(0xd1).to_bytes(1, byteorder="big"))            # 0xd1 for read operation
        self.write(int(addr & 0x3f).to_bytes(1, byteorder="big"))     # send adress 
        rdata = self.read(3)                                          # read data back, first data should be address with R/W flag
                                                                      # last 2 bytes are returned data
        return (rdata[0] & 0x3f), int.from_bytes(rdata[1:],"big")     # split received data into address and data
    
    #
    # dac_range - This function command dsp to change range of a specific DAC channel
    #
    def dac_range(self, channel, ran):
        sdata = ((channel & 0x0F) << 4) | (ran & 0x0F)
        self.write(int(0xd2).to_bytes(1, byteorder="big"))        # 0xd2 for changing range
        self.write(int(sdata).to_bytes(1, byteorder="big"))       # send data
    
    #
    # digital_o - This function command dsp to write a bool number to a specific digital port
    # channel: Dither0:0; Dither1:1; Hold:2; Retract:3; Coarse:4; ROT:5
    #
    def digitalIO(self, channel, data):
        sdata = ((channel & 0xf) << 4) | (data & 0x1)
        self.write(int(0xd3).to_bytes(1, byteorder="big"))        # 0xd3 for digital output
        self.write(int(sdata).to_bytes(1, byteorder="big"))       # send data
    
    #
    # gain - This function command dsp to change gain of a specific channel
    # channel: X:0, Y:1, Z1:2, Z2:3
    # gain command: 0->0.1; 2->1.0; 3->10.0
    # 
    def gain(self, channel, data):
        # Z1 gain is different from others, reverse data for gain 0.1 and gain 10
        if (channel == 2) and (data != 2):
            data = (~ data) & 0x3                                 
        sdata = ((channel & 0xf) << 4) | (data & 0x3)
        self.write(int(0xd4).to_bytes(1, byteorder="big"))        # 0xd4 for changing gain
        self.write(int(sdata).to_bytes(1, byteorder="big"))       # send data 
        
    #
    # digital_scan - This function command dsp to set up digital output for tip approach or scan
    # 1->tip approach; 0->scan
    #
    def digital_scan(self, data):
        self.write(int(0xd5).to_bytes(1, byteorder="big"))              # 0xd5 for set up digital scan
        self.write(int(data & 0x1).to_bytes(1, byteorder="big"))        # send data   
        
    #
    # bit20_W - This function command dsp to write data (20-bit) to a specific 20bit DAC register (3-bit address)
    #
    def bit20_W(self, addr, data):
        sdata = ((addr & 0x00000070) << 16) | (data & 0xfffff)      # patch tx data
        self.write(int(0x20).to_bytes(1, byteorder="big"))          # 0x20 for write operation
        self.write(int(sdata).to_bytes(3, byteorder="big"))         # send data
        
    #
    # bit20_R - This function command dsp to read data (20-bit) form a specific 20bit DAC register (3-bit address)
    #
    def bit20_R(self, addr):
        self.write(int(0x21).to_bytes(1, byteorder="big"))          # 0x21 for read operation
        self.write(int(addr & 0x70).to_bytes(1, byteorder="big"))   # send adress 
        rdata = int.from_bytes(self.read(3) ,"big")                 # receive data
        return ((rdata & 0x700000) << 16), (rdata & 0xfffff)        # split received data into address and data
    
    #
    # adc - This function command dsp to excute 1 conversion of specific channel and return conversion result
    #
    def adc(self, addr):
        self.write(int(0xa0).to_bytes(1, byteorder="big"))       # 0xa0 for conversion operation
        self.write(int(addr & 0xff).to_bytes(1, byteorder="big"))       # send adress    
        rdata = self.read(2)                                     # receive data
        return int.from_bytes(rdata,"big")
    
    #
    # adc_N - This function command desp to excute N conversion of specific channel and return averaged conversion result
    #
    def adc_N(self, addr, n):
        self.write(int(0xa1).to_bytes(1, byteorder="big"))       # 0xa1 for N conversion operation
        self.write(int(addr & 0xff).to_bytes(1, byteorder="big"))       # send address
        self.write(int(n & 0xffff).to_bytes(2, byteorder="big"))          # send number of conversion  
        rdata = self.read(2)                                     # receive data
        return int.from_bytes(rdata,"big")
    
    #
    # adc_W - This function command dsp to write data (8-bit) to a specific ADC program register (7-bit address)
    #
    def adc_W(self, addr, data):
        self.write(int(0xa2).to_bytes(1, byteorder="big"))              # 0xa0 for write operation
        self.write(int(addr & 0xfe).to_bytes(1, byteorder="big"))       # send adress
        self.write(int(data & 0xff).to_bytes(1, byteorder="big"))       # send adress 
    
    #
    # adc_R - This function command dsp to read data (8-bit) from a specific ADC program register (7-bit address)
    #
    def adc_R(self, addr):
        self.write(int(0xa0).to_bytes(1, byteorder="big"))              # 0xa0 for conversion operation
        self.write(int(addr & 0xfe).to_bytes(1, byteorder="big"))       # send adress    
        rdata = self.read(1)                                            # receive data
        return int.from_bytes(rdata,"big")
    
##################################################################################
# DSP = dsp('com3', 9600)
# DSP.flushInput()
# print(DSP.echo(b'a'))
# DSP.close()

# with dsp('com3', 9600) as DSP:
#     DSP.flushInput()
#     print(DSP.echo(b'f'))
        
# -*- coding: utf-8 -*-
"""
Created on Sat Nov 14 18:35:18 2020

@author: yaoji
"""
import serial
import time
from PyQt5.QtCore import pyqtSignal, QObject
import conversion as cnv
import numpy as np
import math

# class myDSP():
class myDSP(QObject):
    
    
    # Define signals
    
    succeed_signal = pyqtSignal(bool)       # Serial port open signal
    oscc_signal = pyqtSignal(int)           # Continuous oscilloscope data emit signal
    rampMeasure_signal = pyqtSignal(list)    # Ramp measure data emit signal
    giantStep_signal = pyqtSignal(int)      # Gian step number signal
    rampTo_signal = pyqtSignal(int)         # Ramp to signal
    rampDiag_signal = pyqtSignal(int, int) # Ramp diagonal signal
    
    #
    # Initial class and all flags and status variable
    #
    def __init__(self, parent = None):
        super(myDSP, self).__init__(parent)
        self.stop = True
        self.idling = True              # Flag that indicates if dsp in an idling state
        self.open = False
        self.version_obtained = False
        self.succeed = False
        self.ver = 'No DSP found'       # DSP version
        self.lastdac = [0x8000] * 16    # Last ouput of all DAC channels
        self.dacrange = [10] * 16       # All DAC channels' current range
        self.adcrange = [0] * 8         # All ADC channels' current range
        self.last20bit = 0x80000        # Last ouput of 20bit DAC
        self.lastdigital = [False] * 6  # 0 - 5 : bias dither, z dither, feedback, retract, coarse, translation
        self.lastgain = [1] * 4         # 0 -> gain 10.0, 1 -> gain 1.0, 3 -> gain 0.1
                                        # Z1 gain is different from others, 3 -> gain 10.0, 1 -> gain 1.0, 0 -> gain 0.1
        self.offset = [0] * 16          # 0 - 14 are bias offset for different range, 15 is Iset offset
        self.port = 'com1'
        self.baudrate = 38400
    
    #
    # Initial dsp
    #
    def init_dsp(self, initOut):
        try:
            self.ser = serial.Serial(self.port, self.baudrate)   # Open serial port
            self.open = True
            self.ser.flushInput()           # Flush input
            self.version()                  # Update DSP version number
            self.succeed = self.open and self.version_obtained # Set succeed flag
            
            # Obtain DSP status varaibles
            self.lastdigital_O()
            self.lastdacAcq()
            self.last20bitAcq()
            self.offsetAcq()
            self.dacRangeAcq()
            self.adcRangeAcq()
            if initOut:
                self.init_output()
        except:
            self.stop = True
            self.idling = True              # Flag that indicates if dsp in an idling state
            self.open = False
            self.version_obtained = False
            self.succeed = False
            self.ver = 'No DSP found'       # DSP version
            self.lastdac = [0x8000] * 16    # Last ouput of all DAC channels
            self.dacrange = [10] * 16       # All DAC channels' current range
            self.adcrange = [0] * 8         # All ADC channels' current range
            self.last20bit = 0x80000        # Last ouput of 20bit DAC
            self.lastdigital = [False] * 6  # 0 - 5 : bias dither, z dither, feedback, retract, coarse, translation
            self.lastgain = [1] * 4         # 0 -> gain 10.0, 1 -> gain 1.0, 3 -> gain 0.1
                                            # Z1 gain is different from others, 3 -> gain 10.0, 1 -> gain 1.0, 0 -> gain 0.1
            self.offset = [0] * 16          # 0 - 14 are bias offset for different range, 15 is Iset offset
            self.port = 'com1'
            self.baudrate = 38400
        if not self.succeed:
            self.close()
        self.succeed_signal.emit(self.succeed)    # Emit succeed signal of finding dsp  
        
    
    #
    # Check if it is ok to execute a dsp operation 
    #       
    def ok(self):
        return self.succeed and self.idling

    #
    # Read serial data after check avaiable byte, max t s before abort, proceted method, internal use only
    # Retrun if reading successful and read data
    #
    def __readt(self, n, t):
        if self.open:
            start_time = time.time()    # Start time
            
            # Wait until time is too long or received enough character
            while ((time.time() - start_time) < t) and (self.ser.inWaiting() < n):
                pass
            
            # If get enough charaters, reading sussess and read the # of byte
            if self.ser.inWaiting() >= n:
                return True, self.ser.read(n)
            
            # Otherwise reading fail, return 0 number
            else:
                return False, (0).to_bytes(n, byteorder='big')
        else:
            return False, (0).to_bytes(n, byteorder='big')
    

    #
    # Close serial
    #
    def close(self):
        if self.open:
            # if self.succeed:
            #     self.init_output()
            self.ser.close()
            self.open = False
            
    #
    # Initial output
    #
    def init_output(self):
        if self.ok():        
            # Initial all DAC output to 0 and range to +/- 10V
            for i in range(16):
                self.dac_W(i + 16, 0x8000)
                self.dac_range(i, 10)
        
            # Initila all ADC range to +/- 10.24V
            for i in range(8):
                self.adc_W(((i + 5) << 1), 0)
            
            self.bit20_W(0x10, 0x828f5)         # Initial 20bit bias to 0.1V
            self.dac_range(2, 14)               # Initial Z offset fine range to +/- 2.5V
            self.dac_range(12, 14)              # Initial bias offset range to +/- 2.5V
            self.dac_range(13, 9)               # Initial bias range to +/- 5V
        
            self.dac_W(0x14, self.offset[15])   # Initial I set offset output
            self.dac_W(0x15, 0xffff)            # Initial I set
            self.dac_W(0x1c, self.offset[9])    # Initial bias offset output
            self.dac_W(0x1d, 0x828f)            # Initial 16bit bias output to 0.1V
            self.digital_scan(1)                # Set up digital output ready for tip approach
    
    #   
    # Acquire DSP version number
    #
    def version(self):
        if self.open and self.idling:
            self.idling = False             # Turn of idling flag
            self.ser.write((0x70).to_bytes(1, byteorder='big'))       # 0x70 for version command
            
             # Read 12 bytes of version, abort after 2 sec
            self.version_obtained, rdata = self.__readt(12, 2)
            self.ver = rdata.decode()
            
            if not self.version_obtained:       # If reading doesn't succeed
                self.ver = 'No DSP found'
                
            self.ser.flushInput()
            self.idling = True              # Turn idling flag back on
        else:
            self.version_obtained = False
            
    #        
    # Acquire last digital output
    #        
    def lastdigital_O(self):
        if self.ok():
            self.idling = False
            self.ser.write((0x72).to_bytes(1, byteorder='big'))       # 0x72 lastfigital_O command
            
            # Load digital: 0 - 5 : bias dither, z dither, feedback, retract, coarse, translation
            d = int.from_bytes(self.ser.read(1), byteorder='big')
            for i in range(6):
                self.lastdigital[5 - i] = bool(d & 0x01)
                d = (d & 0xFE) >> 1
                
            # Load gain: X, Y, Z1, Z2
            g = int.from_bytes(self.ser.read(1), byteorder='big')
            for i in range(4):
                self.lastgain[3 - i] = g & 0x03
                g = (g & 0xFD) >> 2
            self.idling = True

    #        
    # Aquire last DAC output
    #
    def lastdacAcq(self):
        if self.ok():
            self.idling = False
            self.ser.write((0x73).to_bytes(1, byteorder='big'))       # 0x73 for lastdac command
            # Load lastdac
            for i in range(16):
                self.lastdac[i] = int.from_bytes(self.ser.read(2), byteorder='big')
            self.idling = True
            
    #
    # Aquire last 20bit DAC output
    #
    def last20bitAcq(self):
        if self.ok():
            self.idling = False
            self.ser.write((0x74).to_bytes(1, byteorder='big'))       # 0x74 for last20bit command
            # Load last20bit
            self.last20bit = int.from_bytes(self.ser.read(3), byteorder='big')
            self.idling = True
            
    #        
    # Acuire offset data from DSP
    #
    def offsetAcq(self):
        if self.ok():
            self.idling = False
            self.ser.write((0x75).to_bytes(1, byteorder='big'))       # 0x75 for  offset command
            # Load offset
            for i in range(16):
                self.offset[i] = int.from_bytes(self.ser.read(2), byteorder='big')
            self.idling = True
            
    #
    # Acquire DAC range data from DSP
    #
    def dacRangeAcq(self):
        if self.ok():
            self.idling = False
            self.ser.write((0x76).to_bytes(1, byteorder='big'))       # 0x76 for dacrange command
            # Load DAC range
            for i in range(8):
                rang = int.from_bytes(self.ser.read(1), byteorder='big')
                self.dacrange[15 - (2 * i)] = (rang & 0xf0) >> 4
                self.dacrange[14 - (2 * i)] = (rang & 0x0f)
            self.idling = True
    
    #
    # Acquire ADC range data from DSP
    #
    def adcRangeAcq(self):
        if self.ok():
            self.idling = False
            self.ser.write((0x77).to_bytes(1, byteorder='big'))       # 0x77 for adcrange command
             # Load ADC range
            for i in range(8):
                self.adcrange[i] = int.from_bytes(self.ser.read(1), byteorder='big')
            self.idling = True
                
    #        
    # Load offset data to DSP
    #
    def loadOffset(self):
        if self.ok():
            self.idling = False
            self.ser.write((0x78).to_bytes(1, byteorder='big'))       # 0x78 for loadoffset command
             # Send out offset data
            for i in self.offset:
                self.ser.write((i).to_bytes(1, byteorder='big')) 
            self.idling = True
    
    #        
    # adc - This function command dsp to excute 1 conversion of specific channel and return conversion result
    #
    def adc(self, addr):
        if self.ok():
            self.idling = False
            self.ser.write(int(0xa0).to_bytes(1, byteorder="big"))          # 0xa0 for conversion operation
            self.ser.write(int(addr & 0xff).to_bytes(1, byteorder="big"))   # send adress    
            rdata = int.from_bytes(self.ser.read(2),"big")                  # receive data
            self.idling = True
        else:
            rdata = 0
        return rdata
    
    #
    # adc_N - This function command desp to excute N conversion of specific channel and return averaged conversion result
    #
    def adc_N(self, addr, n):
        if self.ok():
            self.idling = False
            self.ser.write(int(0xa1).to_bytes(1, byteorder="big"))          # 0xa1 for N conversion operation
            self.ser.write(int(addr & 0xff).to_bytes(1, byteorder="big"))   # send address
            self.ser.write(int(n & 0xffff).to_bytes(2, byteorder="big"))    # send number of conversion  
            rdata = int.from_bytes(self.ser.read(2),"big")                  # receive data
            self.idling = True
        else:
            rdata = 0
        return rdata
    
    #
    # adc_W - This function command dsp to write data (8-bit) to a specific ADC program register (7-bit address)
    #
    def adc_W(self, addr, data):
        if self.ok():
            addr = addr & 0xfe
            data = data & 0xff
            self.idling = False
            self.ser.write(int(0xa2).to_bytes(1, byteorder="big"))              # 0xa0 for ADC write operation
            self.ser.write(int(addr).to_bytes(1, byteorder="big"))       # send adress
            self.ser.write(int(data).to_bytes(1, byteorder="big"))       # send adress 
            self.idling = True
            if (addr > 0x09) and (addr < 0x19):         # Update adc range variable if writing to adc range register
                self.adcrange[(addr >> 1) - 5] = data
    
    #
    # adc_R - This function command dsp to read data (8-bit) from a specific ADC program register (7-bit address)
    #
    def adc_R(self, addr):
        if self.ok():
            self.idling = False
            self.ser.write(int(0xa3).to_bytes(1, byteorder="big"))              # 0xa3 for ADC read operation
            self.ser.write(int(addr & 0xfe).to_bytes(1, byteorder="big"))       # send adress
            rdata = int.from_bytes(self.ser.read(1),"big")                      # receive data
            self.idling = True
        else:
            rdata = 0
        return rdata
    
    #
    # digital_o - This function command dsp to write a bool number to a specific digital port
    # channel: Dither0:0; Dither1:1; Hold:2; Retract:3; Coarse:4; ROT:5
    #
    def digital_o(self, channel, data):
        if self.ok():
            channel = channel & 0x7
            data = data & 0x1
            self.idling = False
            sdata = (channel << 4) | data
            self.ser.write(int(0xd3).to_bytes(1, byteorder="big"))        # 0xd3 for digital output
            self.ser.write(int(sdata).to_bytes(1, byteorder="big"))       # send data
            self.idling = True
            self.lastdigital[channel] = bool(data)            # Updata last digital variable
    
    #
    # gain - This function command dsp to change gain of a specific channel
    # channel: X:0, Y:1, Z1:2, Z2:3
    # gain command: 0->0.1; 1->1.0; 3->10.0
    # 
    def gain(self, channel, data):
        if self.ok():
            channel = channel & 0x3
            data = data & 0x3
            self.idling = False                                 
            sdata = (channel << 4) | data
            self.ser.write(int(0xd4).to_bytes(1, byteorder="big"))        # 0xd4 for changing gain
            self.ser.write(int(sdata).to_bytes(1, byteorder="big"))       # send data 
            self.idling = True
            self.lastgain[channel] = data               # Update last gain variable
        
    #
    # digital_scan - This function command dsp to set up digital output for tip approach or scan
    # 1->tip approach; 0->scan
    #
    def digital_scan(self, data):
        if self.ok():
            data = data & 0x1
            self.idling = False
            self.ser.write(int(0xd5).to_bytes(1, byteorder="big"))    # 0xd5 for set up digital scan
            self.ser.write(int(data).to_bytes(1, byteorder="big"))    # send data
            self.idling = True
            # Update last digital and last gain variable
            if data == 1:
                self.lastdigital = [False, False, True, False, True, False]
                self.lastgain = [3] * 4
            elif data == 0:
                self.lastdigital = [False, False, True, False, False, True]
                self.lastgain = [1] * 4

    #
    # bit20_W - This function command dsp to write data (20-bit) to a specific 20bit DAC register (3-bit address)
    #
    def bit20_W(self, addr, data):
        if self.ok():
            data = data & 0xfffff
            addr = addr & 0x70
            sdata = (addr << 16) | data      # patch tx data
            self.idling = False
            self.ser.write(int(0x20).to_bytes(1, byteorder="big"))          # 0x20 for write operation
            self.ser.write(int(sdata).to_bytes(3, byteorder="big"))         # send data
            self.idling = True
            # Update last 20bit DAC variable if writing to 20bitDAC DAC register
            if addr == 0x10:
                self.last20bit = data
                
    #
    # bit20_R - This function command dsp to read data (20-bit) form a specific 20bit DAC register (3-bit address)
    #
    def bit20_R(self, addr):
        if self.ok():
            self.idling = False
            self.ser.write(int(0x21).to_bytes(1, byteorder="big"))          # 0x21 for read operation
            self.ser.write(int(addr & 0x70).to_bytes(1, byteorder="big"))   # send adress 
            rdata = int.from_bytes(self.ser.read(3) ,"big")                 # receive data
            self.idling = True
        else:
            rdata = 0
        return ((rdata & 0x700000) >> 16), (rdata & 0xfffff)        # split received data into address and data
            
    #
    # dac_W - This function command dsp to write data (16-bit) to a specific DAC register (6-bit address)
    # 
    def dac_W(self, addr, data):
        if self.ok():
            data = data & 0xffff
            addr = addr & 0x3f
            sdata = (addr << 16) | data      # patch tx data
            self.idling = False
            self.ser.write(int(0xd0).to_bytes(1, byteorder="big"))         # 0xd0 for write operation
            self.ser.write(int(sdata).to_bytes(3, byteorder="big"))        # send data
            self.idling = True
            # Updata last dac variable if writing to DAC DAC register
            if (0x30 & addr) == 0x10:
                self.lastdac[addr - 16] = data
        
    #
    # dac_R - This function command dsp to read data (16-bit) form a specific DAC register (6-bit address)
    #
    def dac_R(self, addr):
        if self.ok():
            self.idling = False
            self.ser.write(int(0xd1).to_bytes(1, byteorder="big"))            # 0xd1 for read operation
            self.ser.write(int(addr & 0x3f).to_bytes(1, byteorder="big"))     # send adress 
            rdata = int.from_bytes(self.ser.read(3) ,"big")                     # read data back, first data should be address with R/W flag
                                                                            # last 2 bytes are returned data
            self.idling = True
        else:
            rdata = 0
        return ((rdata & 0x3f0000) >> 16), (rdata & 0xfffff)     # split received data into address and data

    #
    # dac_range - This function command dsp to change range of a specific DAC channel
    #
    def dac_range(self, channel, ran):
        if self.ok():
            channel = channel & 0x0F
            ran = ran & 0x0F
            sdata = (channel << 4) | ran
            self.idling = False
            self.ser.write(int(0xd2).to_bytes(1, byteorder="big"))        # 0xd2 for changing range
            self.ser.write(int(sdata).to_bytes(1, byteorder="big"))       # send data
            self.idling = True
            self.dacrange[channel] = ran
            
    #
    # zAuto0 - This function command DSP do zAuto0_DSP dunction
    #
    def zAuto0(self):
        if self.ok():
            self.idling = False
            self.ser.write(int(0xa6).to_bytes(1, byteorder="big"))        # 0xa6 for Z auto 0
            self.lastdac[3] = int.from_bytes(self.ser.read(2) ,"big")       # Update last dac Z offset coarse
            self.idling = True

    #
    # iAuto - This function command DSP do iAuto_DSP dunction
    #
    def iAuto(self):
        if self.ok():
            iset = cnv.bv(self.lastdac[5], 'd', 10)
            sdata = cnv.vb(10.0 ** (-iset / 10.0), 'd', 10)
            sdata = sdata & 0xffff
            self.idling = False
            self.ser.write(int(0xa9).to_bytes(1, byteorder="big"))        # 0xa9 for i auto
            self.ser.write(int(sdata).to_bytes(2, byteorder="big"))
            self.lastdac[2] = int.from_bytes(self.ser.read(2) ,"big")       # Update last dac Z offset fine
            self.idling = True            
            
    #
    # square - This function ask DSP to output a square wave of a specific channel
    #
    def square(self, channel, bias1, bias2):
        if self.ok():
            channel = channel & 0x3f
            bias1 = bias1 & 0x0fffff
            bias2 = bias2 & 0x0fffff
            self.idling = False
            self.ser.write(int(0xa8).to_bytes(1, byteorder="big"))        # 0xa8 for square
            self.ser.write(int(channel).to_bytes(1, byteorder="big"))   # Send out channel data
            self.ser.write(int(bias1).to_bytes(3, byteorder="big"))     # Send out the first bias
            self.ser.write(int(bias2).to_bytes(3, byteorder="big"))     # Send out the second bias
            
            # If receive start command
            if int.from_bytes(self.ser.read(1) ,"big") == 0xf0:
                self.stop = False        # Set stop flag to false
                while True:        
                    if self.stop:          # Wait until external source change the stop flag
                        self.ser.write(int(0xff).to_bytes(1, byteorder="big"))
                        break

                # If receive finish command
                if int.from_bytes(self.ser.read(1) ,"big") == 0x0f:
                    self.stop = True                                   # Set stop flag to true
                    rdata = int.from_bytes(self.ser.read(3) ,"big")     # Obtain the final data of corresponding channel
                    # Update status variable
                    if channel == 0x20:
                        self.last20bit = rdata & 0x0fffff
                    else:
                        self.lastdac[channel - 16] = rdata & 0xffff
            self.idling = True
        
    #
    # floatT - This function test the float number calculation speed
    #
    def floatT(self, n, a):
        rdata = 0
        if self.ok():
            n = n & 0xffff
            a = a & 0xffff
            self.idling = False
            self.ser.write(int(0xfd).to_bytes(1, byteorder="big"))        # 0xfd for float calculation testing
            self.ser.write(int(n).to_bytes(1, byteorder="big"))   # Send out channel data
            self.ser.write(int(a).to_bytes(1, byteorder="big"))   # Send out channel data
            
            # If receive start command
            if int.from_bytes(self.ser.read(1) ,"big") == 0xf0:
                self.stop = False        # Set stop flag to false
                while True:        
                    if self.stop:          # Wait until external source change the stop flag
                        self.ser.write(int(0xff).to_bytes(1, byteorder="big"))
                        break
                # If receive finish command
                if int.from_bytes(self.ser.read(1) ,"big") == 0x0f:
                    self.stop = True                                   # Set stop flag to true
                    rdata = int.from_bytes(self.ser.read(2) ,"big")     # Obtain the returned data
                
        return rdata
    
    #
    # echoLoop - This function start echo mode with DSP
    #
    def echoLoop(self):
        if self.ok():
            self.idling = False
            self.ser.write(int(0xec).to_bytes(1, byteorder="big"))        # 0xec for echo mode
             # If receive start command
            if int.from_bytes(self.ser.read(1) ,"big") == 0xf0:
                self.stop = False        # Set stop flag to false
                while True:        
                    if self.stop:          # Wait until external source change the stop flag
                        self.ser.write(int(0xff).to_bytes(1, byteorder="big"))
                        break
                # If receive finish command
                if int.from_bytes(self.ser.read(1) ,"big") == 0x0f:
                    self.stop = True                                   # Set stop flag to true
            self.idling = True
    
    #
    # echo - This function execute one echo cycle
    #
    def echo(self, sdata):
        rdata = 0
        if self.succeed and (not self.idling):
            sdata = sdata & 0xff
            self.ser.write(int(sdata).to_bytes(1, byteorder="big"))
            rdata = int.from_bytes(self.ser.read(1) ,"big")
        return rdata

    #
    # checkStopSeq - This function check stop sequence
    #    
    def checkStopSeq(self):
        if self.succeed and (not self.idling):
            # Check stop sequence
            fc = int.from_bytes(self.ser.read(1) ,"big")
            while True:
                if fc != 0x0f:
                    fc = int.from_bytes(self.ser.read(1) ,"big")
                else:
                    fc = int.from_bytes(self.ser.read(1) ,"big")
                    if fc == 0xaa:
                        fc = int.from_bytes(self.ser.read(1) ,"big")
                        if fc == 0x55:
                            fc = int.from_bytes(self.ser.read(1) ,"big")
                            if fc == 0xa5:
                                fc = int.from_bytes(self.ser.read(1) ,"big")
                                if fc == 0x5a:
                                    self.stop = True                # Set stop flag to true
                                    break
    
    #
    # update_last - update last analog output
    #
    def update_last(self, channel, bits):
        if channel == 0x20:
            self.last20bit = bits & 0x0fffff
        else:
            self.lastdac[channel - 16] = bits & 0xffff 
    
    #
    # limit_output - limit output based on channel
    #
    def limit_out(self, channel, bits):
        if channel == 0x20:
            maximum = 0xfffff
        else:
            maximum = 0xffff
        
        if bits > maximum:
            bits = maximum
        elif bits < 0:
            bits = 0
        
        return bits
            
    #
    # current_last - return current last analog output
    #
    def current_last(self, channel):
        if channel == 0x20:
            return self.last20bit
        else:
            return self.lastdac[channel - 16]
    
    #
    # osc_C - This function execute continuous oscilloscope function
    #
    def osc_C(self, addr, average, delay):
        if self.ok():
            self.idling = False
            self.ser.write(int(0xc0).to_bytes(1, byteorder="big"))              # 0xc0 for continuous oscilloscope
            self.ser.write(int(addr & 0xff).to_bytes(1, byteorder="big"))       # Send adress
            self.ser.write(int(average & 0xffff).to_bytes(2, byteorder="big"))  # Send average number
            self.ser.write(int(delay & 0xffff).to_bytes(2, byteorder="big"))    # Send delay us
            # If receive start command
            if int.from_bytes(self.ser.read(1) ,"big") == 0xf0:
                self.stop = False        # Set stop flag to false
                while True:        
                    if self.stop:          # Wait until external source change the stop flag
                        self.ser.write(int(0xff).to_bytes(1, byteorder="big"))  # Send stop command
                        break       # Break loop
                    else:
                        rdata = int.from_bytes(self.ser.read(2) ,"big") # Read returned data
                        self.oscc_signal.emit(rdata)   # Send out data through signal
                self.checkStopSeq() # Check stop sequence
            self.idling = True

    #
    # osc_N - This function execute N smaples oscilloscope function
    #
    def osc_N(self, addr, n, average, delay):
        rdata = []
        if n > 4095:
            n = 4095
        if self.ok():
            addr = addr & 0xff
            n = n & 0xffff
            self.idling = False
            self.ser.write(int(0xc1).to_bytes(1, byteorder="big"))              # 0xc1 for N samples oscilloscope
            self.ser.write(int(addr).to_bytes(1, byteorder="big"))              # send adress
            self.ser.write(int(n & 0xfff).to_bytes(2, byteorder="big"))         # send average number
            self.ser.write(int(average).to_bytes(2, byteorder="big"))           # send average number
            self.ser.write(int(delay & 0xffff).to_bytes(2, byteorder="big"))    # Send delay us
            for i in range(n):
                rdata = rdata + [int.from_bytes(self.ser.read(2) ,"big")]       # Obtain the read data
                # print(rdata[i])
            self.idling = True
        return rdata
    
    #
    # rampTo - This function ramp an output to specific value
    #
    def rampTo(self, channel, target, step, delay, limit, checkstop):
        if self.ok():
            target = self.limit_out(channel, target)
            channel = channel & 0x3f
            target = target & 0x0fffff
            step = step & 0x0fffff
            delay = delay & 0xffff
            limit = limit & 0x7fff
            if checkstop:
                limit += 0x8000         # Patch crash protection limit with check stop flag
            self.idling = False
            self.ser.write(int(0x31).to_bytes(1, byteorder="big"))      # 0x31 for ramp to function
            self.ser.write(int(channel).to_bytes(1, byteorder="big"))   # Send ramp channel
            self.ser.write(int(target).to_bytes(3, byteorder="big"))    # Send target
            self.ser.write(int(step).to_bytes(3, byteorder="big"))      # Send step size
            self.ser.write(int(delay).to_bytes(2, byteorder="big"))     # Send delay us
            self.ser.write(int(limit).to_bytes(2, byteorder="big"))     # Send out limit for crash protection and check stop flag
            
            i = self.current_last(channel)
            if target != i:
                step = int((target - i) * step / abs(target - i))
            else:
                step = 0
            # If receive start command
            if int.from_bytes(self.ser.read(1) ,"big") == 0xf0:
                self.stop = False                       # Set stop flag to false
                while True:      # Wait until receive returned data
                    command = int.from_bytes(self.ser.read(1) ,"big")
                    if command != 0x5a:
                        break
                    else:
                        self.update_last(channel, i)
                        self.rampTo_signal.emit(channel)
                        i += step
                    if checkstop and self.stop:    # If check stop is enabled and stop event is issued
                        self.ser.write(int(0xff).to_bytes(1, byteorder="big"))  # Send out a stop command
                        self.stop = False
                # Receive finish command
                if command == 0x1f:             # If crash protection triggered
                    self.lastdac[3] = int.from_bytes(self.ser.read(2) ,"big")   # Update last Z offset coarse output
                self.stop = True                                                # Set stop flag to true
                
                rdata = int.from_bytes(self.ser.read(3) ,"big")                 # Returned data for last output
                # Use returned data to update last output gloabl variable
                self.update_last(channel, rdata)
                    
            self.idling = True

    #
    # serialSeq _This function serial out sequence
    #
    def serialSeq(self, seq):
        if self.open:
            self.ser.write(int(seq.seq_num).to_bytes(1, byteorder="big"))                           # Send sequence number
            for i in range(seq.seq_num):
                self.ser.write(int(seq.command_list[i] & 0xff).to_bytes(1, byteorder="big"))        # Send command list
                self.ser.write(int(seq.data_list[i] & 0xffffffff).to_bytes(4, byteorder="big"))     # Send data list


    #
    # rampMeasure - This function ramp an output to specific value
    #
    def rampMeasure(self, channel, step_num, step_size, move_delay, measure_delay, direction, seq):
        if self.ok():
            channel = channel & 0x3f
            step_num = step_num & 0xffff
            step_size = step_size & 0x0fffff
            move_delay = move_delay & 0xffff
            measure_delay = measure_delay & 0xffff
            if direction:
                dir_data = 0x01
            else:
                dir_data = 0x00
            
            if channel == 0x20:
                stored = self.last20bit
            else:
                stored = self.lastdac[channel -16]

            self.idling = False
            self.ser.write(int(0x32).to_bytes(1, byteorder="big"))           # 0x32 for ramp measure function
            self.ser.write(int(channel).to_bytes(1, byteorder="big"))        # Send ramp channel information
            self.ser.write(int(step_num).to_bytes(2, byteorder="big"))       # Send step number
            self.ser.write(int(step_size).to_bytes(3, byteorder="big"))      # Send step size
            self.ser.write(int(move_delay).to_bytes(2, byteorder="big"))     # Send move delay us
            self.ser.write(int(measure_delay).to_bytes(2, byteorder="big"))  # Send measure delay us
            self.ser.write(int(dir_data).to_bytes(1, byteorder="big"))       # Send direction information
            self.serialSeq(seq)                                              # Send sequence command and data
            
            # If receive start command
            if int.from_bytes(self.ser.read(1) ,"big") == 0xf0:
                self.stop = False                                   # Set stop flag to false
                
                for i in range(step_num):
                    if self.stop:
                        break
                    # Update output value
                    if direction:
                        current = stored + (i * step_size)
                    else:
                        current = stored - (i * step_size)
                    rdata = [current]
                    for i in range(seq.read_num):
                        rdata += [int.from_bytes(self.ser.read(2) ,"big")]  # Read data
                    self.rampMeasure_signal(rdata)
                    
                # If stopped
                if self.stop:
                    self.ser.write(int(0xff).to_bytes(1, byteorder="big"))  # Send stop command
                    self.checkStopSeq()                                     # Check stop sequence
                else:
                    int.from_bytes(self.ser.read(1) ,"big") == 0x0f         # Otherwise, just wait for the finish command
                self.stop = True  
                lastout = int.from_bytes(self.ser.read(3) ,"big")           # Return data to update last output
                self.update_last(channel, lastout)                          # Update last output
                
            self.idling = True
            
    #
    # rampDiag - This function ramp 2 output channels together
    #
    def rampDiag(self, channels, channell, targets, targetl, step, delay, limit, checkstop):                        
        if self.ok():
            channels = channels & 0x3f
            channell = channell & 0x3f
            targets = targets & 0xffff
            targetl = targetl & 0xffff
            currents = self.lastdac[channels - 16]
            currentl = self.lastdac[channell - 16]
            ranges = targets - currents
            rangel = targetl - currentl
            stepnum = int(math.sqrt((ranges ** 2) + (rangel ** 2))) & 0xffff
            steps = ranges / stepnum
            stepl = rangel / stepnum
            delay = delay & 0xffff
            limit = limit & 0x7fff
            if checkstop:
                limit += 0x8000         # Patch crash protection limit with check stop flag
            self.idling = False
            self.ser.write(int(0x33).to_bytes(1, byteorder="big"))      # 0x33 for ramp diagonal function
            self.ser.write(int(channels).to_bytes(1, byteorder="big"))  # Send channels
            self.ser.write(int(channell).to_bytes(1, byteorder="big"))  # Send channell
            self.ser.write(int(targets).to_bytes(2, byteorder="big"))   # Send targets
            self.ser.write(int(targetl).to_bytes(2, byteorder="big"))   # Send targetl
            self.ser.write(int(stepnum).to_bytes(2, byteorder="big"))   # Send step number
            self.ser.write(int(delay).to_bytes(2, byteorder="big"))     # Send delay us
            self.ser.write(int(limit).to_bytes(2, byteorder="big"))     # Send out limit for crash protection and check stop flag

            # If receive start command
            if int.from_bytes(self.ser.read(1) ,"big") == 0xf0:
                self.stop = False                       # Set stop flag to false
                
                # Wait until receive any instruction
                while True:
                    command = int.from_bytes(self.ser.read(1) ,"big")
                    if command != 0x5a:
                        break
                    else:
                        self.lastdac[channels - 16] = int(currents) & 0xffff
                        self.lastdac[channell - 16] = int(currentl) & 0xffff
                        self.rampDiag_signal.emit(channels, channell)
                        currents += steps
                        currentl += stepl
                    if checkstop and self.stop:    # If check stop is enabled and stop event is issued
                        self.ser.write(int(0xff).to_bytes(1, byteorder="big"))  # Send out a stop command
                        self.stop = False  
                
                # Receive finish command
                if int.from_bytes(self.ser.read(1) ,"big") == 0x1f:             # Crash protection triggered
                    self.lastdac[3] = int.from_bytes(self.ser.read(2) ,"big")   # Update last Z offset coarse output
                self.stop = True    # Set stop flag to true
                
                # Update last dac based on return data
                rdatas = int.from_bytes(self.ser.read(2) ,"big")                # Last output data
                self.lastdac[channels - 16] = rdatas & 0xffff
                rdatal = int.from_bytes(self.ser.read(2) ,"big")                # Last output data
                self.lastdac[channell - 16] = rdatal & 0xffff
                
            self.idling = True            
            
    #
    # giantStep - This function perform giant step of a specific channel
    #
    def giantStep(self, channel, x, z, delay1, delay, gx, gz, stepnum):
        if self.ok():
            channel = channel & 0x3f
            x = x & 0xffff
            z = z & 0xffff
            delay1 = delay1 & 0xffff
            delay = delay & 0xffff
            gx = gx & 0xffff
            gz = gz & 0xffff
            stepnum = stepnum & 0xffff
            self.idling = False
            self.ser.write(int(0x83).to_bytes(1, byteorder="big"))      # 0x83 for giant step function
            self.ser.write(int(channel).to_bytes(1, byteorder="big"))   # Send ramp channel
            self.ser.write(int(x).to_bytes(2, byteorder="big"))         # Send x step
            self.ser.write(int(z).to_bytes(2, byteorder="big"))         # Send z step
            self.ser.write(int(delay1).to_bytes(2, byteorder="big"))    # Send inter giant step delay us
            self.ser.write(int(delay).to_bytes(2, byteorder="big"))     # Send wait sample stable delay us
            self.ser.write(int(gx).to_bytes(2, byteorder="big"))        # Send acceleration for x ramp
            self.ser.write(int(gz).to_bytes(2, byteorder="big"))        # Send acceleration for z ramp
            self.ser.write(int(stepnum).to_bytes(2, byteorder="big"))   # Send step number
            
            # If receive start command
            if int.from_bytes(self.ser.read(1) ,"big") == 0xf0:
                self.stop = False           # Set stop flag to false
                i = 0                       # Intial giant step counter
                instruction = 0                 # Initial instruction
                while (instruction != 0x0f):    # Wait until receive finish command
                
                    # If stop event issued
                    if self.stop:
                        self.ser.write(int(0xff).to_bytes(1, byteorder="big"))  # Send out a stop command
                        self.stop = False
                    instruction = int.from_bytes(self.ser.read(1) ,"big")       # Receive new instruction
                    
                    # If instruction is ongoing
                    if instruction == 0x5a:
                        i += 1                          # Giant step counter + 1
                        self.giantStep_signal.emit(i)   # Emit Giant step counter
                self.stop = True                # Set stop flag to true
            self.idling = True

    #
    # tipApproach - This function perform tipApproach
    #
    def tipApproach(self, x, z, delay1, delay, gx, gz, giant, baby, babyg, limit):
        if self.ok():
            x = x & 0xffff
            z = z & 0xffff
            delay1 = delay1 & 0xffff
            delay = delay & 0xffff
            gx = gx & 0xffff
            gz = gz & 0xffff
            giant = giant & 0xffff
            baby = baby & 0xffff
            babyg = babyg & 0xffff
            limit = limit & 0xffff
            self.idling = False
            self.ser.write(int(0x84).to_bytes(1, byteorder="big"))      # 0x84 for giant step function
            self.ser.write(int(x).to_bytes(2, byteorder="big"))         # Send x step
            self.ser.write(int(z).to_bytes(2, byteorder="big"))         # Send z step
            self.ser.write(int(delay1).to_bytes(2, byteorder="big"))    # Send inter giant step delay us
            self.ser.write(int(delay).to_bytes(2, byteorder="big"))     # Send wait sample stable delay us
            self.ser.write(int(gx).to_bytes(2, byteorder="big"))        # Send acceleration for x ramp
            self.ser.write(int(gz).to_bytes(2, byteorder="big"))        # Send acceleration for z ramp
            self.ser.write(int(giant).to_bytes(2, byteorder="big"))     # Send giant step number
            self.ser.write(int(baby).to_bytes(2, byteorder="big"))      # Send baby step ramp size
            self.ser.write(int(babyg).to_bytes(2, byteorder="big"))     # Send baby step restore acceleration
            self.ser.write(int(limit).to_bytes(2, byteorder="big"))     # Send tunneling current lower limit
            
            # If receive start command
            if int.from_bytes(self.ser.read(1) ,"big") == 0xf0:
                self.stop = False           # Set stop flag to false
                i = 0                       # Intial giant step counter
                instruction = 0                 # Initial instruction
                while (instruction != 0x0f):    # Wait until receive finish command
                
                    # If stop event issued
                    if self.stop:
                        self.ser.write(int(0xff).to_bytes(1, byteorder="big"))  # Send out a stop command
                        self.stop = False
                    instruction = int.from_bytes(self.ser.read(1) ,"big")       # Receive new instruction
                    
                    # If instruction is ongoing
                    if instruction == 0x5a:
                        i += giant                      # Giant step counter + giant step number
                        self.giantStep_signal.emit(i)   # Emit Giant step counter
                self.stop = True                # Set stop flag to true
            self.idling = True    
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
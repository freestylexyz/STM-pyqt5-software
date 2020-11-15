# -*- coding: utf-8 -*-
"""
Created on Sat Nov 14 18:35:18 2020

@author: yaoji
"""
from serial import *
from PyQt5.QtCore import *

class DSP(QObject):
    succeed_signal = pyqtSignal(bool)          # Serial port open signal
    
    def __init__(self, parent=None, port='com1', baudrate=38400):
        super(DSP, self).__init__(parent, port, baud)
        try:
            self.ser = Serial(port, baud)   # Open serial port
            self.open = True
            self.idling = True
            self.__flush()                  # Flush input and output
            self.version()                  # Update DSP version number
            self.status()                   # Update DSP status information
            self.succeed = True
            
        except:
            self.open = False               # Flag that indicates if com port successfully open
            self.idling = True              # Flag that indicates if dsp in an idling state
            self.ver = 'No DSP found'       # DSP version
            self.lastdac = [0x8000] * 16    # Last ouput of all DAC channels
            self.dacrange = [10] * 16       # All DAC channels' current range
            self.adcrange = [0] * 8         # All ADC channels' current range
            self.last20bit = 0              # Last ouput of 20bit DAC
            self.lastdigital = [False] * 6  # 0 - 5 : bias dither, z dither, feedback, retract, coarse, translation
            self.lastgain = [2] * 4         # 0 -> gain 10.0, 2 -> gain 1.0, 3 -> gain 0.1
            self.offset = [0] * 16          # 0 - 14 are bias offset for different range, 15 is Iset offset
            self.succeed = False            # Flag that indicates if dsp successfully intialized
            
        self.succeed_signal.emit(self.succeed)    # Emit succeed signal of finding dsp
    
    # Check if it is ok to execute a dsp operation        
    def ok(self):
        return self.succeed & self.idling

    # Read serial data, proceted method, internal use only
    def __read(self, n):
        if self.open:
            return self.ser.read(n)
        else:
            return (0).to_bytes(n, byteorder='big')
    
    # Write serial data, proceted method, internal use only
    def __write(self, txdata):
        if self.open:
            self.ser.write(txdata)
    
    # Close serial
    def close(self):
        if self.open & self.idling:
            self.ser.close()
    
    # Flush serial input and ouput, proceted method, internal use only
    def __flush(self):
        if self.open:
            self.ser.flushInput()
            self.ser.flushOutput()
            
    # !!! Not finished
    # Acquire DSP version number
    def version(self):
        if self.open & self.idling:
            self.ver = 'x'
    
    # !!! Not finished
    # Acquire DSP status information
    def status(self):
        if self.open & self.idling:
            self.lastdac = [0x8000] * 16
            self.dacrange = [10] * 16
            self.adcrange = [0] * 8
            self.last20bit = 0
            self.lastdigital = [False] * 6  # 0 - 5 : bias dither, z dither, feedback, retract, coarse, translation
            self.lastgain = [2] * 4         # 0 -> gain 10.0, 2 -> gain 1.0, 3 -> gain 0.1
            self.offset = [0] * 16          # 0 - 14 are bias offset for different range, 15 is Iset offset
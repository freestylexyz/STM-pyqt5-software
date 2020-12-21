# -*- coding: utf-8 -*-
"""
Created on Sun Dec 20 13:52:32 2020

@author: yaoji
"""

import conversion as cnv
import numpy as np
import math

class mySeqence():
    def __init__(self, command_list = [], data_list = []):
        # Basic property
        self.command_list = command_list        # Initialize command list
        self.data_list = data_list              # Intialize data list
        self.seq_num = len(self.command)        # Figure out sequence number
        self.read_num = 0                       # Initialize read number
        # Figure out read number
        for i in self.command:
            if (i >= 0xc000) and (i <= 0xdc00):
                self.read_num += 1
        
        if self.seq_num > 0:
            self.valid = True       # For dirct use without compile
        else:
            self.valid = False      # Need to be compiled
                
        # Raw code
        self.command = []
        self.channel = []
        self.option1 = []
        self.option2 = []
        self.data = []
        
        # For compile use
        self.commandDict = {'Wait': [0x00, 0x00, 0x00000000, 0x00000000, 0xffffffff],\
                           'Match': [0x20, 0x00, 0x10000000, 0x00000000, 0x0000ffff],\
                           'Dout': [0x40, 0x07, 0x00000000, 0x00000000, 0x00000001],\
                           'Shift': [0x60, 0x1f, 0x10000000, 0x00000000, 0x000fffff],\
                           'Aout': [0x80, 0x1f, 0x00000000, 0x00000000, 0x000fffff],\
                           'Ramp': [0xa0, 0x1f, 0x00000000, 0xfff00000, 0x000fffff],\
                           'Read': [0xc0, 0x1c, 0x00000000, 0x00000000, 0x0000ffff],\
                           'ShiftRamp': [0xe0, 0x1f, 0x10000000, 0x7ff00000, 0x000fffff]}
        self.channelDict = {'Z offset fine': 2, 'Z offset': 3, 'Iset': 5, 'DAC6': 6, 'DAC7': 7, 'DAC8': 8, 'DAC9': 9, 'DAC11': 11, 'Bias' : 13,\
                           'AIN0': 0x00, 'AIN1': 0x04, 'AIN2': 0x08, 'AIN3': 0x0c, 'AIN4': 0x10, 'AIN5': 0x14, 'ZOUT': 0x18, 'PREAMP': 0x1c}
        self.dataDict = {'Wait': self.bb, 'Match': self.mb, 'Dout': self.bb, 'Shift': self.ab,\
                        'Aout': self.ab, 'Ramp': self.ab, 'Read': self.bb, 'ShiftRamp': self.ab}
        
        # System status
        self.mode = True                        # True for read sequence, False for deposition sequence
                                                # Default read sequence
        # !!! may need to optimze
        self.range = [10] * 17                  # Default +/- 10V range for all
        self.pream_gain = 9                     # Default preamp gain
        self.dac = [0x8000] * 16 +  [0x80000]   # Default all DAC to middle scale
        self.digital = [False] * 6              # Default all digital to false

    
    #
    # Configure current STM setting
    #
    def configure(self, bias_dac, preamp_gain, dacrange, lastdac, last20bitdac, lastdigital):
        if bias_dac:
            self.bias_mode = '20'
            self.channel.update({'Bias' : 16})
        else:
            self.bias_mode = 'd'
            self.channel.update({'Bias' : 13})
        self.range = dacrange
        self.dac = lastdac + [last20bitdac]
        self.digital = lastdigital
        self.pream_gain = preamp_gain

    #
    # Load raw code
    # command, channel and data are lists of string
    # option1 and option2 are list of data
    #
    def load_data(self, command, channel, option1, option2, data):
        self.command = command
        self.channel = channel
        self.option1 = option1
        self.option2 = option2
        self.data = data
    
    #
    # Load squence from file
    #
    def load_sequence(self):
        pass

    #
    # Validate raw code
    #
    def validation(self):
        if self.mode:       # Validate read sequence
            self.valid = True
        else:               # Validate deposition sequence
            self.valid = True
    
    #
    # Simulate image, return image data
    #
    def simulation_i(self):
        image = np.zeros((128, 128))
        return image
    
    #
    # Simulate spectroscopy, return spctroscopy data
    #
    def simulation_s(self):
        spc = np.zeros(200)
        return spc
    
    
    # 
    # Compile raw code
    #
    def build(self):
        self.command_list = []              # Initialize command list
        self.data_list = []                 # Intialize data list
        self.seq_num = len(self.command)    # Total sequence number
        self.read_num = 0                   # Re-initialize read number
        
        # Figure out read number
        for i in range(self.seq_num):
            comm = self.command[i]
            comp = self.commandDict[comm]
            ch = self.channelDict[self.channel[i]]
            op1 = self.option1[i]
            op2 = self.option2[i]
            dstr = self.data[i]
            
            self.command_list += (self.comp[0] & 0xe0) | (ch & comp[1])
            self.data_list += (self.dataDict[comm](dstr, ch) & comp[4]) | ((op2 << 20) & comp[3]) | ((op1 * comp[2]) & 0x10000000)
            
            if comm == 'Read':
                self.read_num += 1
    
    # Data comipling fuction for general use
    def bb(self, dstr, ch):
        return int(dstr)
    
    # Data comipling function for matching current
    def mb(self, dstr, ch):
        v = cnv.bv(self.dac[5], 'd', self.range[5])
        b = cnv.vb(10.0 ** (-v / 10.0), 'a')
        return b
                
    # Data comipling function for analog output related
    def ab(self, dstr, ch):
        if (ch == 2) or (ch == 3):
            b = int(dstr)                       # Convert data string to bits value
        elif ch == 5:           
            # Determin mulitplier based on gain
            if self.pream_gain == 8:
                multiplier = 10.0
            elif self.pream_gain == 9:
                multiplier = 1.0
            elif self.pream_gain == 10:
                multiplier = 0.1
                
            i = float(dstr)                     # Convert data string to current value
            i = i / multiplier                  # Divide multiplier
            i = -10.0 * math.log(i, 10)         # Calculate I set voltage
            b = cnv.vb(i, 'd', self.range[5])   # Convert it to I set bits
        else:
            v = float(dstr)                     # Convert data string to voltage value
            if ch != 16:
                b = cnv.vb(v, 'd', self.range[ch])
            else:
                b = cnv.vb(v, '20')
        return b



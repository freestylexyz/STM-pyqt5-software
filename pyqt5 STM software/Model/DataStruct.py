# -*- coding: utf-8 -*-
"""
Created on Sun Jan  3 15:55:44 2021

@author: yaoji
"""
from datetime import datetime
from DigitalSignalProcessor import myDSP

class STMData():
    def __init__(self):
        now = datetime.now()
        self.time = now.strftime("%d/%m/%Y %H:%M:%S")
        self.seq_name = 'Default'
        self.path = ''
        self.bias_dac = False
        self.preamp_gain = 9
        
        self.lastdac = [0x8000] * 16    # Last ouput of all DAC channels
        self.dacrange = [10] * 16       # All DAC channels' current range
        self.adcrange = [0] * 8         # All ADC channels' current range
        self.last20bit = 0x80000        # Last ouput of 20bit DAC
        self.lastdigital = [False] * 6  # 0 - 5 : bias dither, z dither, feedback, retract, coarse, translation
        self.lastgain = [1] * 4         # 0 -> gain 10.0, 1 -> gain 1.0, 3 -> gain 0.1
                                        # Z1 gain is different from others, 3 -> gain 10.0, 1 -> gain 1.0, 0 -> gain 0.1
        self.offset = [0] * 16          # 0 - 14 are bias offset for different range, 15 is Iset offset
        
    def load_status(self, dsp, preamp_gain, bias_dac):
        self.bias_dac = bias_dac
        self.preamp_gain = preamp_gain
        
        self.lastdac = dsp.lastdac
        self.dacrange = dsp.dacrange
        self.adcrange = dsp.adcrange
        self.last20bit = dsp.last20bit
        self.lastdigital = dsp.lastdigital
        self.lastgain = dsp.lastgain
        self.offset = dsp.offset

class DepData(STMData):
    def __init__(self):
        super().__init__()
        self.data = []
        self.seq_name = 'Poke Default'
        self.read = [0xdc] + ([0] * 7) #[read_ch, read_mode, read_delay, read_delay2, read_num, read_avg, read_change, read_stop_num]
        
class ScanData(STMData):
    def __init__(self):
        super().__init__()
        self.data = []
        self.seq_name = 'General Constant Current'
        
class SpcData(STMData):
    def __init__(self):
        super().__init__()
        self.data = []
        self.seq_name = 'SPC default'
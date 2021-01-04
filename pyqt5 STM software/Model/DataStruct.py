# -*- coding: utf-8 -*-
"""
Created on Sun Jan  3 15:55:44 2021

@author: yaoji
"""
from datetime import datetime

class STMData():
    def __init__(self):
        now = datetime.now()
        self.time = now.strftime("%d/%m/%Y %H:%M:%S")
        self.seq_name = 'Default'
        self.path = ''
        self.bias_dac = False
        self.preamp_gain = 9
        self.bias_ran = 9
        self.setpoint = [0x8000] * 8    #[Bias, Iset, Zoff, Zoff_fine, Xoff, Xin, Yoff, Yin]
        self.gain = [1.0] * 4           #[Xgain, Ygain, Zgain_1, Zgain_2]
        self.digital = [False] * 4      #[Dither0, Dither1, Feedback, Retract]

class DepData(STMData):
    def __init__(self):
        super().__init__()
        self.data = []
        self.seq_name = 'Poke Default'
        self.read = [0xdc] + ([0] * 7) #[read_ch, read_mode, read_delay, read_delay2, read_num, read_avg, read_change, read_stop_num]
        
class ImageData(STMData):
    def __init(self):
        super().__init__()
        self.data = []
        self.seq_name = 'General Constant Current'
        
class SpcData(STMData):
    def __init(self):
        super().__init__()
        self.data = []
        self.seq_name = 'SPC default'
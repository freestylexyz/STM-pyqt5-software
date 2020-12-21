# -*- coding: utf-8 -*-
"""
Created on Thu Dec  3 11:02:51 2020

@author: yaoji
"""

import sys
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from MainMenu import myMainMenu


#  mySettingControl class - This class handles signal from setting, uppack data and execute corresponding operation
class mySettingControl(myMainMenu):
    # Setting initial DSP slot
    def setting_init_slot(self, vport, vbaudrate):
        self.dsp.port = vport           # Set DSP COM port
        self.dsp.baudrate = vbaudrate   # Set DSP serial baud rate
        self.dsp.init_dsp(True)         # Initial DSP and initial output
    
    # Setting load offset to DSP slot
    def setting_load_slot(self, offset1, offset2, offset3, offset4):
        # Load offset data to dsp status variable
        self.dsp.offset[14] = offset1
        self.dsp.offset[9] = offset2
        self.dsp.offset[10] = offset3
        self.dsp.offset[15] = offset4
        
        self.dsp.loadOffset()   # Set offset data to DSP

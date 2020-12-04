# -*- coding: utf-8 -*-
"""
Created on Thu Dec  3 11:02:51 2020

@author: yaoji
"""

import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSignal, Qt, QMetaObject, QSettings
from Setting import mySetting
from TipApproach import myTipApproach
from Etest import myEtest
from MainMenu import myMainMenu
import conversion as cnv
import threading

class mySettingControl(myMainMenu):
    # Setting initial DSP slot
    def setting_init_slot(self, vport, vbaudrate):
        self.dsp.port = vport
        self.dsp.baudrate = vbaudrate
        self.dsp.init_dsp(True)
    
    # Setting load offset to DSP slot
    def setting_load_slot(self, offset1, offset2, offset3, offset4):
        self.dsp.offset[14] = offset1
        self.dsp.offset[9] = offset2
        self.dsp.offset[10] = offset3
        self.dsp.offset[15] = offset4
        self.dsp.loadOffset()

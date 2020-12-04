# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 16:04:07 2020

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
import math

class myCurrentControl(myMainMenu):
    # Init bias dock
    def init_current_dock(self):
        self.init_current()
    
    # Show current dock
    def current_show(self):
        self.init_current()
        self.Current.show()
    
    def init_current(self):
        # !!! Enable serial related modules
        self.groupBox_Gain_Current.setEnabled(self.dsp.succeed)
        self.groupBox_Setpoint_Current.setEnabled(self.dsp.succeed)
        self.groupBox_Ramp_Current.setEnabled(self.dsp.succeed)
        
        # Set up preamp gain radio button
        self.radioButton_8_Gain.setChecked(self.preamp_gain == 8)
        self.radioButton_9_Gain.setChecked(self.preamp_gain == 9)
        self.radioButton_10_Gain.setChecked(self.preamp_gain == 10)
        
        # Set up set point
        bits = self.dsp.lastdac[5]
        self.scrollBar_Input_Setpoint.setValue(bits)
        self.spinBox_Input_Setpoint.setValue(self.b2i(bits))
        
        # Set up ramp
        self.current_spinbox_range
        
    def b2i(self, bits):
        value = cnv.bv(bits, 'd', self.dsp.dacrange[5])
        if self.preamp_gain == 8:
            multiplier = 10.0
        elif self.preamp_gain == 9:
            multiplier = 1.0
        elif self.preamp_gain == 9:
            multiplier = 0.1
        else:
            multiplier = 0.0
        return 10.0 ** (-value / 10.0) * multiplier
    
    def i2b(self, value):
        if self.preamp_gain == 8:
            multiplier = 10.0
        elif self.preamp_gain == 9:
            multiplier = 1.0
        elif self.preamp_gain == 9:
            multiplier = 0.1
        else:
            multiplier = 0.0
        value = value / multiplier
        value = -10.0 * math.log(value, 10)
        bits = cnv.vb(value, 'd', self.dsp.dacrange[5])
        return bits
        
    # Set up all spin boxes input range
    def current_spinbox_range(self):
        minimum = self.b2i(0xffff)
        maximum = self.b2i(0x0)

        # Set minimum
        self.spinBox_Input_Setpoint.setMinimum(minimum)
        # !!! ramp spin boxes

        # Set maximum
        self.spinBox_Input_Setpoint.setMaximum(maximum)
        # !!! ramp spin boxes
        
    def current_gain(self):
        if self.dsp.lastdigital[3]:
            # !!! pop out window, turn feeedback off
            pass
        else:
            value = self.spinBox_Input_Setpoint.value()
            bits = self.i2b(value)
            self.dsp.rampTo(0x15, bits)
            self.scrollBar_Input_Setpoint.setValue(bits)
            

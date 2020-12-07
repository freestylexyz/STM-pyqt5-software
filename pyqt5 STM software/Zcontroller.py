# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 16:04:05 2020

@author: yaoji
"""

import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication, QMainWindow, QDesktopWidget
from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSignal, Qt, QMetaObject, QSettings
from MainMenu import myMainMenu
import time
import conversion as cnv
import threading
from functools import partial

class myZcontroller(myMainMenu):
    # Init Zcontroller dock
    def init_Zcontroller_dock(self):
        self.init_Zcontroller()
        
        # Z offset coarse
        self.scrollBar_Input_Zoffset.valueChanged.connect(self.spinBox_Input_Zoffset.setValue)
        self.spinBox_Input_Zoffset.valueChanged.connect(self.scrollBar_Input_Zoffset.setValue)
        self.pushButton_Send_Zoffset.clicked.connect(self.z_send)
        self.pushButton_Zauto_Zoffset.clicked.connect(self.z_auto)
        self.pushButton_HardRetract_Zoffset.clicked.connect(lambda: self.z_retract(True, True))
        
        # Z offset fine
        self.spinBox_Indication_Zoffset.valueChanged.connect(self.slider_Input_Zoffset.setValue)
        self.pushButton_Advance_Zoffsetfine.clicked.connect(self.z_advance)
        self.pushButton_Zero_Zoffsetfine.clicked.connect(self.z_advance)
        self.pushButton_MatchCurrent_Zoffsetfine.clicked.connect(self.z_match_i)
        
        # Digital toggle
        self.radioButton_ON_Feedback.toggled.connect(self.z_feedback)
        self.radioButton_ON_Retract.toggled.connect(partial(self.z_retract, False))
        self.radioButton_ON_ZDither.toggled.connect(self.z_dither)
        
        # Gain Change
        self.radioButton_Z1gain01_Gain.toggled.connect(self.z_gain_1)
        self.radioButton_Z1gain1_Gain.toggled.connect(self.z_gain_1)
        self.radioButton_Z1gain10_Gain.toggled.connect(self.z_gain_1)
        self.radioButton_Z2gain01_Gain.toggled.connect(self.z_gain_2)
        self.radioButton_Z2gain1_Gain.toggled.connect(self.z_gain_2)
        self.radioButton_Z2gain10_Gain.toggled.connect(self.z_gain_2)
    
    # Show Zcontroller dock
    def Zcontroller_show(self):
        self.init_Zcontroller()
        self.Zcontrol.show()
    
    def init_Zcontroller(self):
        # Enable serial related modules
        self.enable_Zcontrol_serial(self.dsp.succeed)
        
        # Load status
        self.scrollBar_Input_Zoffset.setValue(self.dsp.lastdac[3])
        self.spinBox_Input_Zoffset.setValue(self.dsp.lastdac[3])
        self.spinBox_Indication_Zoffset.setValue(self.dsp.lastdac[2])
        self.slider_Input_Zoffset.setValue(self.dsp.lastdac[2])
        self.radioButton_ON_ZDither.setChecked(self.dsp.lastdigital[1])
        self.radioButton_OFF_ZDither.setChecked(not self.dsp.lastdigital[1])
        self.radioButton_ON_Feedback.setChecked(self.dsp.lastdigital[2])
        self.radioButton_OFF_Feedback.setChecked(not self.dsp.lastdigital[2])
        self.radioButton_ON_Retract.setChecked(self.dsp.lastdigital[3])
        self.radioButton_OFF_Retract.setChecked(not self.dsp.lastdigital[3])
        self.load_z1_gain()
        self.load_z2_gain()
        
        # Hard retract
        if self.hard_retract:
            self.pushButton_HardRetract_Zoffset.setText("Hard retract")
        else:
            self.pushButton_HardRetract_Zoffset.setText("Hard unretract")
        
    # Load Z1 gain from DSP
    def load_z1_gain(self):
        if self.dsp.lastgain[2] == 0:
            self.radioButton_Z1gain10_Gain.setChecked(True)
        elif self.dsp.lastgain[2] == 1:
            self.radioButton_Z1gain1_Gain.setChecked(True)
        elif self.dsp.lastgain[2] == 3:
            self.radioButton_Z1gain01_Gain.setChecked(True)
    
    # Load Z2 gain from DSP
    def load_z2_gain(self):
        if self.dsp.lastgain[3] == 0:
            self.radioButton_Z1gain01_Gain.setChecked(True)
        elif self.dsp.lastgain[3] == 1:
            self.radioButton_Z1gain1_Gain.setChecked(True)
        elif self.dsp.lastgain[3] == 3:
            self.radioButton_Z1gain10_Gain.setChecked(True)
    
    # Z offset coarse send
    def z_send(self):
        bits = self.scrollBar_Input_Zoffset.value()
        bits = bits + 0x8000
        if bits != self.dsp.lastdac[3]:
            threading.Thread(target = (lambda: self.z_coarse_excu(bits))).start()  # Execute with thread
        self.scrollBar_Input_Zoffset.setValue(self.dsp.lastdac[3] - 0x8000)
        self.spinBox_Indication_Zoffset.setValue(self.dsp.lastdac[3] - 0x8000)
    
    # Z offset coarse ramp exectuion
    def z_coarse_excu(self, bits):
        self.enable_mode_serial(False)                      # Disable all serial related component in current window
        self.idling = False     
        self.dsp.rampTo(0x13, bits, 1, 100, 0, False)
        self.idling = True
        self.enable_mode_serial(True)
    
    # Z auto 0
    def z_auto(self):
        pass
    
    # Z auto 0 exectuion
    def z_auto_excu(self, delayed):
        if delayed:
            time.sleep(5)
    
    # Z offset fine advance
    def z_advance(self):
        pass
    
    # Z offset fine zero
    def z_zero(self):
        pass
    
    # Z match current
    def z_match_i(self):
        pass
    
    # Feedback toggle
    def z_feedback(self):
        pass

    # Retract toggle
    def z_retract(self, hard, retract):
        popped = False
        if hard:
            retract = not self.hard_retracted
                
        if retract != self.dsp.lastdigital[3]:
            if self.dsp.lastdigital[2]:
                self.dsp.digital_o(3, retract)
            else:
                # !!! pop out message, feedback is off
                popped = True
            self.radioButton_ON_Retract.setChecked(self.dsp.lastdigital[3])
        
        if hard and self.dsp.lastdigital[2]:
            self.hard_retracted = not self.hard_retracted
            if self.hard_retracted:
                self.pushButton_HardRetract_Zoffset.setText("Hard retract")
            else:
                self.pushButton_HardRetract_Zoffset.setText("Hard unretract")
            threading.Thread(target = (lambda: self.z_auto_excu(True))).start()
        elif hard and (not popped):
            # !!! pop out message, feedback is off
            pass

    
    # Z dither toggle
    def z_dither(self):
        pass
    
    # Z1 gain toggle
    def z_gain_1(self):
        pass
    
    # Z2 gain toggle
    def z_gain_2(self):
        pass


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
        
        # self.radioButton_8_Gain.toggled.connect(self.current_gain)
        # self.radioButton_9_Gain.toggled.connect(self.current_gain)
        # self.radioButton_10_Gain.toggled.connect(self.current_gain)
        
        # self.spinBox_Input_Setpoint.editingFinished.connect(self.current_value)
        # self.scrollBar_Input_Setpoint.valueChanged.connect(self.current_out)
        
        # self.pushButton_Rampto1_CurrRamp.clicked.connect(self.current_ramp_1)
        # self.pushButton_Rampto2_CurrRamp.clicked.connect(self.current_ramp_2)
        # self.pushButton_Rampto3_CurrRamp.clicked.connect(self.current_ramp_3)
        # self.pushButton_Rampto4_CurrRamp.clicked.connect(self.current_ramp_4)
    
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
        self.current_set_radio()
        
        # Set up set point
        bits = self.dsp.lastdac[5]
        self.scrollBar_Input_Setpoint.setValue(bits)
        self.spinBox_Input_Setpoint.setValue(self.b2i(bits, self.preamp_gain))
        
        # Set up ramp
        self.current_spinbox_range
        
    def b2i(self, bits, gain):
        value = cnv.bv(bits, 'd', self.dsp.dacrange[5])
        if gain == 8:
            multiplier = 10.0
        elif gain == 9:
            multiplier = 1.0
        elif gain == 9:
            multiplier = 0.1
        else:
            multiplier = 0.0
        return 10.0 ** (-value / 10.0) * multiplier
    
    def i2b(self, value, gain):
        if gain == 8:
            multiplier = 10.0
        elif gain == 9:
            multiplier = 1.0
        elif gain == 9:
            multiplier = 0.1
        else:
            multiplier = 0.0
        value = value / multiplier
        value = -10.0 * math.log(value, 10)
        bits = cnv.vb(value, 'd', self.dsp.dacrange[5])
        return bits
        
    # Set up all spin boxes input range
    def current_spinbox_range(self):
        minimum = self.b2i(0xffff, self.preamp_gain)
        maximum = self.b2i(0x0, self.preamp_gain)

        # Set minimum
        self.spinBox_Input_Setpoint.setMinimum(minimum)
        self.spinBox_Input1_CurrRamp.setMinimum(minimum)
        self.spinBox_Input2_CurrRamp.setMinimum(minimum)
        self.spinBox_Input3_CurrRamp.setMinimum(minimum)
        self.spinBox_Input4_CurrRamp.setMinimum(minimum)

        # Set maximum
        self.spinBox_Input_Setpoint.setMaximum(maximum)
        self.spinBox_Input1_CurrRamp.setMaximum(maximum)
        self.spinBox_Input2_CurrRamp.setMaximum(maximum)
        self.spinBox_Input3_CurrRamp.setMaximum(maximum)
        self.spinBox_Input4_CurrRamp.setMaximum(maximum)
    
    # Set radio button for preamp gain
    def current_set_radio(self):
        if self.preamp_gain == 8:
            self.radioButton_8_Gain.setChecked(True)
        elif self.preamp_gain == 9:
            self.radioButton_9_Gain.setChecked(True)
        elif self.preamp_gain == 10:
            self.radioButton_10_Gain.setChecked(True)
    
    def current_gain(self):
        if self.radioButton_8_Gain.isChecked():
            gain = 8
        elif self.radioButton_9_Gain.isChecked():
            gain = 9
        elif self.radioButton_10_Gain.isChecked():
            gain = 10
        
        if gain != self.preamp_gain:
            minimum = self.b2i(0xffff, gain)
            maximum = self.b2i(0x0, gain)
            value = self.spinBox_Input_Setpoint.value()         # Current value
            if (value > maximum) and (value < minimum):
                # !!! pop out window, out of range
                self.current_set_radio()
            elif self.dsp.lastdigital[3]:
                # !!! pop out window, turn feeedback off
                self.current_set_radio()
            else:
                self.preamp_gain = gain
                self.current_spinbox_range()
                bits = self.i2b(value, self.preamp_gain)
                self.dsp.rampTo(0x15, bits, 2, 1000, 0, False)
                self.scrollBar_Input_Setpoint.setValue(bits)
                
    # Setpoint spinBox slot
    def current_value(self):
        value = self.spinBox_Input_Setpoint.value()
        self.scrollBar_Input_Setpoint.setValue(self.i2b(value, self.preamp_gain))
    
    # Setpoint scroll bar slot
    def current_out(self, bits):
        if bits != self.dsp.lastdac[5]:
            self.dsp.bit20_W(0x15, bits)
            self.spinBox_Input_Setpoint.setValue(self.b2i(bits, self.preamp_gain))
    
    # Current stop ramp button slot
    def current_stop(self):
        self.dsp.stop = True
        self.pushButton_StopRamp_CurrRamp.setEnabled(False)
    
        
    # Setpoint ramp function
    def current_ramp(self, value):
        self.enable_dock_serial(False)
        self.enable_mode_serial(False)
        self.pushButton_StopRamp_CurrRamp.setEnabled(True)
        step = self.spinBox_SpeedInput_CurrRamp.value()
        target = self.i2b(value, self.preamp_gain)
        self.rampTo(0x15, target, step, 1000, 0, True)
        self.scrollBar_Input_Setpoint.setValue(self.dsp.lastdac[5])
        self.enable_dock_serial(True)
        self.enable_mode_serial(True)
        
    # Setpoint ramp button 1 slot
    def current_ramp_1(self):
        value = self.spinBox_Input1_CurrRamp.value()
        threading.Thread(target = (lambda: self.current_ramp(value))).start()
    
    # Setpoint ramp button 1 slot
    def current_ramp_2(self):
        value = self.spinBox_Input2_CurrRamp.value()
        threading.Thread(target = (lambda: self.current_ramp(value))).start()
        
    # Setpoint ramp button 1 slot
    def current_ramp_3(self):
        value = self.spinBox_Input3_CurrRamp.value()
        threading.Thread(target = (lambda: self.current_ramp(value))).start()
    
    # Setpoint ramp button 1 slot
    def current_ramp_4(self):
        value = self.spinBox_Input4_CurrRamp.value()
        threading.Thread(target = (lambda: self.current_ramp(value))).start()

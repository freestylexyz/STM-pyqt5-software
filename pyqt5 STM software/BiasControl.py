# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 13:52:10 2020

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

class myBiasControl(myMainMenu):
    # Init bias dock
    def init_bias_dock(self):
        self.init_bias()
        
        # Connect slots
        self.scrollBar_Input_Bias.valueChanged.connect(self.bias_out)
        self.spinBox_Input_Bias.editingFinished.connect(self.bias_value)
        self.pushButton_Filp_Bias.clicked.connect(self.bias_flip)
        self.radioButton_ON_BiasDither.toggled.connect(self.bias_dither)
        self.radioButton_20bit_DAC.toggled.connect(self.bias_dac_slot)
        self.radioButton_25_Range.toggled.connect(self.bias_range)
        self.radioButton_5_Range.toggled.connect(self.bias_range)
        self.radioButton_10_Range.toggled.connect(self.bias_range)
        self.pushButton_StopRamp_BiasRamp.clicked.connect(self.bias_stop)
        self.pushButton_Rampto1_BiasRamp.clicked.connect(self.bias_ramp_1)
        self.pushButton_Rampto2_BiasRamp.clicked.connect(self.bias_ramp_2)
        self.pushButton_Rampto3_BiasRamp.clicked.connect(self.bias_ramp_3)
        self.pushButton_Rampto4_BiasRamp.clicked.connect(self.bias_ramp_4)
        self.pushButton_Rampto5_BiasRamp.clicked.connect(self.bias_ramp_5)
        self.pushButton_Rampto6_BiasRamp.clicked.connect(self.bias_ramp_6)
        self.pushButton_Rampto7_BiasRamp.clicked.connect(self.bias_ramp_7)
        self.pushButton_Rampto8_BiasRamp.clicked.connect(self.bias_ramp_8)
    
    # Show bias dock
    def bias_show(self):
        self.init_bias()
        self.Bias.show()
    
    # Init bias UI
    def init_bias(self):
        # !!! Enable serial related modules
        self.groupBox_Dither_Bias.setEnabled(self.dsp.succeed)
        self.groupBox_Range_Bias.setEnabled(self.dsp.succeed)
        self.groupBox_Bias_Bias.setEnabled(self.dsp.succeed)
        
        # Set up UI
        self.radioButton_ON_BiasDither.setChecked(self.dsp.lastdigital[0])
        self.radioButton_OFF_BiasDither.setChecked(not self.dsp.lastdigital[0])
        self.radioButton_16bit_DAC.setChecked(not self.bias_dac)
        self.radioButton_20bit_DAC.setChecked(self.bias_dac)
        self.bias_dac_slot(self.bias_dac)
    
    # Check feedback and cross zero to determin if OK to change current value
    # True stands for can not change    
    def check_feedback(self, bits, equal0):
        fb = self.dsp.lastdigital[3]
        if self.bias_dac:
            if equal0:
                cross_zero = ((bits - 0x80000) * self.dsp.last20bit) == 0
            else:
                cross_zero = ((bits - 0x80000) * self.dsp.last20bit) <= 0
        else:
            if equal0:
                cross_zero = ((bits - 0x8000) * self.dsp.lastdac[13]) == 0
            else:
                cross_zero = ((bits - 0x8000) * self.dsp.lastdac[13]) <= 0
        return fb and cross_zero
    
    # Set up all spin boxes input range
    def bias_spinbox_range(self):
        if self.bias_dac:
            minimum = cnv.bv(0, '20')
            maximum = cnv.bv(0xfffff, '20')
        else:
            minimum = cnv.bv(0, 'd', self.dsp.dacrange[13])
            maximum = cnv.bv(0xffff, 'd', self.dsp.dacrange[13])
        # Set minimum
        self.spinBox_Input_Bias.setMinimum(minimum)
        self.spinBox_Input1_BiasRamp.setMinimum(minimum)
        self.spinBox_Input2_BiasRamp.setMinimum(minimum)
        self.spinBox_Input3_BiasRamp.setMinimum(minimum)
        self.spinBox_Input4_BiasRamp.setMinimum(minimum)
        self.spinBox_Input5_BiasRamp.setMinimum(minimum)
        self.spinBox_Input6_BiasRamp.setMinimum(minimum)
        self.spinBox_Input7_BiasRamp.setMinimum(minimum)
        self.spinBox_Input8_BiasRamp.setMinimum(minimum)
        # Set maximum
        self.spinBox_Input_Bias.setMaximum(maximum)
        self.spinBox_Input1_BiasRamp.setMaximum(maximum)
        self.spinBox_Input2_BiasRamp.setMaximum(maximum)
        self.spinBox_Input3_BiasRamp.setMaximum(maximum)
        self.spinBox_Input4_BiasRamp.setMaximum(maximum)
        self.spinBox_Input5_BiasRamp.setMaximum(maximum)
        self.spinBox_Input6_BiasRamp.setMaximum(maximum)
        self.spinBox_Input7_BiasRamp.setMaximum(maximum)
        self.spinBox_Input8_BiasRamp.setMaximum(maximum)
    
    # Set bias range radio button
    def bias_range_radio(self):
        if self.bias_dac:
            self.radioButton_5_Range.setChecked(True)
            self.groupBox_Range_Bias.setEnabled(False)
        else:
            if self.dsp.dacrange[13] == 14:
                self.radioButton_25_Range.setChecked(True)
            elif self.dsp.dacrange[13] == 9:
                self.radioButton_5_Range.setChecked(True)
            elif self.dsp.dacrange[13] == 10:
                self.radioButton_10_Range.setChecked(True)
            self.groupBox_Range_Bias.setEnabled(True)
    
    # Bias selection radio button slot
    def bias_dac_slot(self, checked):
        self.bias_dac = checked
        self.bias_spinbox_range()
        self.bias_range_radio()
        if self.bias_dac:
            self.scrollBar_Input_Bias.setMaximum(0xfffff)
            self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
            self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.last20bit, '20'))
            self.spinBox_SpeedInput_BiasRamp.setValue(20)
        else:
            self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])
            self.scrollBar_Input_Bias.setMaximum(0xffff)
            self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.lastdac[13], 'd', self.dsp.dacrange[13]))
            self.spinBox_SpeedInput_BiasRamp.setValue(1)
            
    # Bias spinBox_Input_Bias slot
    def bias_value(self):
        value = self.spinBox_Input_Bias.value()
        if self.bias_dac:
            self.scrollBar_Input_Bias.setValue(cnv.vb(value, '20'))
        else:
            self.scrollBar_Input_Bias.setValue(cnv.vb(value, 'd', self.dsp.dacrange[13]))
    
    # Bias scrollBar_Input_Bias slot
    def bias_out(self, bits):
        if self.bias_dac:
            if bits != self.dsp.last20bit:
                if self.check_feedback(bits, True):
                    # !!! pop out window, feedback is on can not cross zero
                    self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
                else:
                    self.dsp.bit20_W(0x10, bits)
            self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.last20bit, '20'))
        else:
            if bits != self.dsp.lastdac[13]:
                if self.check_feedback(bits, True):
                    # !!! pop out window, feedback is on can not cross zero
                    self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])
                else:
                    self.dsp.dac_W(0x1d, bits)
            self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.lastdac[13], 'd', self.dsp.dacrange[13]))
    
    # Bias flip button slot
    def bias_flip(self):
        value = -self.spinBox_Input_Bias.value()
        self.spinBox_Input_Bias.setValue(value)
        
    # Bias dither radio button slot
    def bias_dither(self, checked):
        if self.dsp.lastdigital[0] != checked:
            self.dsp.digital_o(0, checked)
    
    # Bias range radio button slot
    def bias_range(self):
        if self.radioButton_25_Range.isChecked():
            ran = 14
        elif self.radioButton_5_Range.isChecked():
            ran = 9
        elif self.radioButton_10_Range.isChecked():
            ran = 10
        minimum = cnv.bv(0, 'd', ran)                   # Target range minimum
        maximum = cnv.bv(0xffff, 'd', ran)              # Target range maximum
        value = self.spinBox_Input_Bias.value()         # Current value
        
        if (value > maximum) or (value < minimum):
            # Abort range changing if current value out of target range
            # !!! pop window current bias is out of target range, can not change
            # Restore to original radio button setup
            self.bias_range_radio()
        else:
            # Proceed change if bias range is changed
            if (not self.bias_dac) and (self.dsp.dacrange[13] != ran):
                self.enable_dock_serial(False)
                self.enable_mode_serial(False)
                feedback_store = self.dsp.lastdigital[3]        # Store current feedback status
                self.dsp.digital_o(3, False)                    # Feedback off
                self.dsp.rampTo(0x1d, 0x8000, 10, 200, 0, False)   # Ramp to 0
                self.dsp.dac_range(13, ran)                     # Change range
                self.dsp.rampTo(0x1d, cnv.vb(value, 'd', self.dsp.dacrange[13]), 10, 200, 0, False)  # Restore to original bias voltage
                self.dsp.digital_o(3, feedback_store)           # Restore orignial feedback status
                
                self.bias_spinbox_range()                        # Set spin boxes range
                self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])                                    # Set scroll bar value
                self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.lastdac[13], 'd', self.dsp.dacrange[13]))  # Set spin box value
                self.enable_dock_serial(True)
                self.enable_mode_serial(True)

    # Bias stop ramp button slot
    def bias_stop(self):
        self.dsp.stop = True
        self.pushButton_StopRamp_BiasRamp.setEnabled(False)
    
    # Bias ramp function
    def bias_ramp(self, value):
        self.enable_dock_serial(False)
        self.enable_mode_serial(False)
        self.pushButton_StopRamp_BiasRamp.setEnabled(True)
        step = self.spinBox_SpeedInput_BiasRamp.value()
        if self.bias:
            channel = 0x20
            target = cnv.vb(value, '20')
        else:
            channel = 0x1d
            target = cnv.vb(value, 'd', self.dsp.dacrange[13])
        if self.check_feedback(target, False):
            # !!! pop out window, feedback on, can not ramp cross 0
            pass
        else:
            self.rampTo(channel, target, step, 1000, 0, True)
        if self.bias:
            self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
        else:
            self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])
        self.enable_dock_serial(True)
        self.enable_mode_serial(True)
        
    # Bias ramp button 1 slot
    def bias_ramp_1(self):
        value = self.spinBox_Input1_BiasRamp.value()
        threading.Thread(target = (lambda: self.bias_ramp(value))).start()
        
    # Bias ramp button 2 slot
    def bias_ramp_2(self):
        value = self.spinBox_Input2_BiasRamp.value()
        threading.Thread(target = (lambda: self.bias_ramp(value))).start()
    
    # Bias ramp button 3 slot
    def bias_ramp_3(self):
        value = self.spinBox_Input3_BiasRamp.value()
        threading.Thread(target = (lambda: self.bias_ramp(value))).start()
    
    # Bias ramp button 4 slot
    def bias_ramp_4(self):
        value = self.spinBox_Input4_BiasRamp.value()
        threading.Thread(target = (lambda: self.bias_ramp(value))).start()

    # Bias ramp button 5 slot
    def bias_ramp_5(self):
        value = self.spinBox_Input5_BiasRamp.value()
        threading.Thread(target = (lambda: self.bias_ramp(value))).start()
        
    # Bias ramp button 6 slot
    def bias_ramp_6(self):
        value = self.spinBox_Input6_BiasRamp.value()
        threading.Thread(target = (lambda: self.bias_ramp(value))).start()
    
    # Bias ramp button 7 slot
    def bias_ramp_7(self):
        value = self.spinBox_Input7_BiasRamp.value()
        threading.Thread(target = (lambda: self.bias_ramp(value))).start()
    
    # Bias ramp button 8 slot
    def bias_ramp_8(self):
        value = self.spinBox_Input8_BiasRamp.value()
        threading.Thread(target = (lambda: self.bias_ramp(value))).start()

    
# -*- coding: utf-8 -*-
"""
Created on Thu Dec  3 11:11:48 2020

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

class myEtestControl(myMainMenu):
    
    def etest_ramp():
        pass
    
    def etest_ramp_start(self):
        # lambda
        threading.Thread(self.etest_ramp).start()

    # range changed slot
    def range_changed_slot(self, index, range):
        if index == 0:                                  # adc range changed
            ch = self.adc_ch.currentIndex()             # get channel from view
            self.dsp.adc_W((ch+5) << 1, range)          # output to dsp
        else:                                           # dac range changed
            ch = self.dac_ch.currentIndex()             # get channel from view
            self.dsp.dac_range(ch, range)               # output to dsp
            self.myEtest.set_dac_spinBox_range(range)   # update spinBox range from dsp

    # channel changed slot
    def ch_changed_slot(self, index, ch):
        if index == 0:                                             # adc channel changed
            self.myEtest.load_range(index, self.dsp.adcrange[ch])  # update range view from dsp
        else:                                                      # dac channel changed
            self.myEtest.load_range(index, self.dsp.dacrange[ch])  # update range view from dsp

    # last digital changed slot
    def digital_changed_slot(self, ch, status):
        self.dsp.digital_o(ch, status)                  # output to dsp

    # last gain changed slot
    def gain_changed_slot(self, ch, val):
        self.dsp.gain(ch, val)                          # output to dsp

    # adc input button slot
    def adc_input_slot(self):
        ch = self.adc_ch.currentIndex()             # get channel from view
        ran = self.dsp.adcrange[ch]                 # get range from dsp
        val = cnv.bv(self.dsp.adc(ch), 'a', ran)    # get value from dsp
        self.adc_val.setValue(val)                  # update indication view

    # dac ouput/zero button slot
    def dac_output_slot(self, bits):
        ch = self.dac_ch.currentIndex()             # get channel from view
        self.dsp.dac_W(ch+16, bits)                 # output to dsp

    # 20 bit dac output/zero button slot
    def bit20_output_slot(self, bits):
        self.dsp.bit20_W(0x10, bits)                # output to dsp
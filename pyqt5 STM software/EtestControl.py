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

    # I/O | range changed slot
    def range_changed_slot(self, index, ran):
        if index == 0:                                # adc range changed
            ch = self.etest.adc_ch.currentIndex()     # get channel from view
            self.dsp.adc_W((ch+5) << 1, ran)          # output to dsp
        else:                                         # dac range changed
            ch = self.etest.dac_ch.currentIndex()     # get channel from view
            self.dsp.dac_range(ch, ran)               # output to dsp
            self.etest.set_dac_spinBox_range(ran)     # update spinBox range from dsp
            self.etest.dac_range = ran                # update range variable

    # I/O | channel changed slot
    def ch_changed_slot(self, index, ch):
        if index == 0:                                             # adc channel changed
            self.etest.load_range(index, self.dsp.adcrange[ch])    # update range view from dsp
        else:                                                      # dac channel changed
            self.etest.load_range(index, self.dsp.dacrange[ch])    # update range view from dsp

    # I/O | last digital changed slot
    def digital_changed_slot(self, ch, status):
        self.dsp.digital_o(ch, status)                # output to dsp

    # I/O | last gain changed slot
    def gain_changed_slot(self, ch, val):
        self.dsp.gain(ch, val)                        # output to dsp

    # I/O | adc input button slot
    def adc_input_slot(self):
        ch = self.etest.adc_ch.currentIndex()         # get channel from view
        ran = self.dsp.adcrange[ch]                   # get range from dsp
        val = cnv.bv(self.dsp.adc(ch), 'a', ran)      # get value from dsp
        self.etest.adc_val.setValue(val)              # update indication view

    # Ramp Test | setup ranmp
    def rtest_ramp(self, index, outch, inch, init, final, step_size):
        # if self.etest.idling:
        #     self.etest.enable_serial(False)
        #     self.etest.idling = False
        #     if index == 0:  # ramp button clicked
        #         self.dsp.rampTo()  # Ramp
        #     else:   # ramp read button clicked
        #         self.dsp.rampMeasure()  # Ramp Read
        #     self.etest.idling = True  # Toggle dock idling flag
        #     self.etest.enable_serial(True)
        pass

    # Ramp Test | ramp signal slot
    def rtest_ramp_slot(self, index, outch, inch, init, final, step_size):
        # threading.Thread(target=(lambda: self.rtest_ramp(final))).start()
        pass
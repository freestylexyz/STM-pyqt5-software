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

    # I/O | range changed slot
    def range_changed_slot(self, index, ran):
        if index == 0:                                              # adc range changed
            ch = self.etest.adc_ch.currentIndex()                   # get channel from view
            self.dsp.adc_W((ch+5) << 1, ran)                        # output to dsp
        else:                                                       # dac range changed
            ch = self.etest.dac_ch.currentIndex()                   # get channel from view
            self.dsp.dac_range(ch, ran)                             # output to dsp
            # !!! use "ran" or "self.dsp.dacrange[ch]" ?
            self.etest.set_dac_spinBox_range(ran)                   # update spinBox range from dsp
            self.etest.dac_range = ran                              # update range variable
            self.etset.load_dac_output(0, self.dsp.lastdac[ch])     # update dac output view from dsp

    # I/O & Ramp Text | channel changed slot
    def ch_changed_slot(self, index, ch):
        if index == 0:                                                      # adc channel changed
            self.etest.load_range(index, self.dsp.adcrange[ch])             # update range view from dsp
        elif index == 1:                                                    # dac channel changed
            self.etest.load_range(index, self.dsp.dacrange[ch])             # update range view from dsp
            self.etset.load_dac_output(0, self.dsp.lastdac[ch])             # update output view from dsp
        elif index == 2:                                                    # ramp test input
            self.etest.set_range_text(index, self.dsp.adcrange[ch])         # update range view from dsp
        elif index == 3:                                                    # ramp test output
            self.etest.set_range_text(index, self.dsp.dacrange[ch])         # update range view from dsp
            self.etest.set_rtest_spinBox_range(ch, self.dsp.dacrange[ch])   # update spinBox range from dsp

    # I/O | adc input button slot
    def adc_input_slot(self):
        ch = self.etest.adc_ch.currentIndex()         # get channel from view
        ran = self.dsp.adcrange[ch]                   # get range from dsp
        val = cnv.bv(self.dsp.adc(ch), 'a', ran)      # get value from dsp
        self.etest.adc_val.setValue(val)              # update indication view

    # Ramp Test | setup ranmp
    def rtest_ramp(self, index, inch, outch, init, final, step_size):
        if self.etest.idling:
            self.etest.enable_serial(False)
            self.etest.idling = False
            if index == 0:                                                  # ramp button clicked
                self.etest.pushButton_Ramp_RTest.setText("Stop")            # change ramp button text
                self.etest.pushButton_Ramp_RTest.setEnabled(True)           # enable stop button
                self.dsp.rampTo(outch, init, 1, 100, 0, False)     # Ramp to initial value
                self.dsp.rampTo(outch, final, step_size, 10000, 0, True)    # Ramp to final value
                self.rtest_ramp_update(index, outch)                        # update ramp data
            else:                                                           # ramp read button clicked
                self.etest.pushButton_RRead_RTest.setText("Stop")           # change ramp read button text
                self.etest.pushButton_RRead_RTest.setEnabled(True)          # enable stop button
                command = 4 * inch + 0xC000
                self.dsp.rampMeasure(outch, init, 1, 100, 10000, [command], [10])   # Ramp Read to initial value
                self.dsp.rampMeasure(outch, final, step_size, 10000, 10000, [command], [10])  # Ramp Read to final value
                # !!! update current and rdata by signal
            self.etest.idling = True
            self.etest.enable_serial(True)
            self.etest.pushButton_Ramp_RTest.setText("Ramp")                 # reset ramp button
            self.etest.pushButton_RRead_RTest.setEnabled("Ramp read")        # reset ramp read button

    # Ramp Test | update ramp data
    def rtest_ramp_update(self, index, channel):
        if index == 0:  # ramp
            if channel != 20:
                self.etest.rtest_ramp_data += [self.dsp.lastdac[channel - 16]]
            else:
                self.etest.rtest_ramp_data += [self.dsp.last20bit]

    # Ramp Test | update ramp read data
    def record_ramp_read_data(self, current, rdata):
        self.etest.rtest_ramp_read_data += [(current, rdata)]

    # Ramp Test | ramp signal slot
    def rtest_ramp_slot(self, index, inch, outch, init, final, step_size):
        threading.Thread(target=(lambda: self.rtest_ramp(self, index, inch, outch, init, final, step_size))).start()

    # stop button slot
    def stop_slot(self):
        self.dsp.stop = True


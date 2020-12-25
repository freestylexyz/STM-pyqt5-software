# -*- coding: utf-8 -*-
"""
Created on Thu Dec  3 11:11:48 2020

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
from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSignal, Qt, QMetaObject, QSettings
from sequence import mySequence
from MainMenu import myMainMenu
import conversion as cnv
import threading

class myEtestControl(myMainMenu):

    # I/O | range changed slot
    def range_changed_slot(self, index, ran):
        if index == 0:                                              # adc range changed
            ch = self.etest.adc_ch.currentIndex()                   # get channel from view
            self.dsp.adc_W((ch+5) << 1, ran)                        # output to dsp
            # self.dsp.adc_W(ch*4+0xC0, ran)                        # !!! this causes error
        else:                                                       # dac range changed
            ch = self.etest.dac_ch.currentIndex()                   # get channel from view
            self.dsp.dac_range(ch, ran)                             # output to dsp
            # !!! Function dsp.dac_range() requires channel, not addr?
            self.etest.set_dac_spinBox_range(ran)                   # update spinBox range from dsp
            self.etest.dac_range = ran                              # update range variable
            self.etest.load_dac_output(0, self.dsp.current_last(ch+16))     # update dac output view from dsp

    # I/O & Ramp Text | channel changed slot
    def ch_changed_slot(self, index, ch):
        if index == 0:                                                          # adc channel changed
            self.etest.load_range(index, self.dsp.adcrange[ch])                 # update range view from dsp
        elif index == 1:                                                        # dac channel changed
            self.etest.load_range(index, self.dsp.dacrange[ch])                 # update range view from dsp
            self.etest.load_dac_output(0, self.dsp.current_last(ch+16))         # update output view from dsp
        elif index == 2:                                                        # ramp test input
            self.etest.set_range_text(index, self.dsp.adcrange[ch])             # update range view from dsp
        elif index == 3:                                                        # ramp test output
            if ch != 16:                                                        # 16 bit dac
                self.etest.set_spinBox_range(index, ch, self.dsp.dacrange[ch])  # update spinBox range from dsp
                self.etest.set_range_text(index, self.dsp.dacrange[ch])         # update range view from dsp
            else:                                                               # 20 bit dac
                self.etest.set_spinBox_range(index, ch, 16)                     # update spinBox range from dsp
                self.etest.set_range_text(index, 16)                            # update range view from dsp
        elif index == 4:                                                        # square wave output
            if ch != 16:                                                        # 16 bit dac
                self.etest.set_spinBox_range(index, ch, self.dsp.dacrange[ch])  # update spinBox range from dsp
                self.etest.set_range_text(index, self.dsp.dacrange[ch])         # update range view from dsp
            else:                                                               # 20 bit dac
                self.etest.set_spinBox_range(index, ch, 16)                     # update spinBox range from dsp
                self.etest.set_range_text(index, 16)                            # update range view from dsp

    # I/O | adc input button slot
    def adc_input_slot(self):
        ch = self.etest.adc_ch.currentIndex()               # get channel from view
        ran = self.dsp.adcrange[ch]                         # get range from dsp
        val = cnv.bv(self.dsp.adc(ch*4+0xC0), 'a', ran)     # get value from dsp
        self.etest.adc_val.setValue(val)                    # update indication view

    # Ramp Test | setup ranmp
    def rtest_ramp(self, index, inch, outch, init, final, step_size):
        if self.etest.idling:
            self.etest.enable_serial(False)
            self.etest.idling = False
            origin = self.dsp.current_last(outch+16)                        # get initial value
            if index == 0:                                                  # ramp button clicked
                self.etest.pushButton_Ramp_RTest.setText("Stop")            # change ramp button text
                self.etest.pushButton_Ramp_RTest.setEnabled(True)           # enable stop button
                self.dsp.rampTo(outch+16, init, step_size, 10000, 0, False)    # ramp to initial value
                self.dsp.rampTo(outch+16, final, step_size, 10000, 0, True)    # ramp to final value
                self.dsp.rampTo(outch+16, origin, step_size, 10000, 0, False)  # ramp back to original value
            else:                                                           # ramp read button clicked
                self.etest.pushButton_RRead_RTest.setText("Stop")           # change ramp read button text
                self.etest.pushButton_RRead_RTest.setEnabled(True)          # enable stop button
                command = 4 * inch + 0xC0
                read_seq = mySequence([command], [10])
                direction = (final - init) >= 0
                step_num = int(abs((final - init)/step_size))
                self.dsp.rampTo(outch+16, init, step_size, 10000, 0, False)    # ramp to initial value
                self.dsp.rampMeasure(outch+16, step_num, step_size, 10000, 10000, direction, read_seq)  # ramp meaure to final value
                self.dsp.rampTo(outch+16, origin, step_size, 10000, 0, False)    # ramp back to original value
            self.etest.idling = True
            self.etest.enable_serial(True)
            self.etest.pushButton_Ramp_RTest.setText("Ramp")              # reset ramp button
            self.etest.pushButton_RRead_RTest.setText("Ramp read")        # reset ramp read button

    # Ramp Test | update ramp read data
    def ramp_read_update(self, rdata):
        inch = self.etest.rtest_get_inch()
        outch = self.etest.rtest_get_outch()
        ramp_read_indata = cnv.bv(rdata[1], 'a', self.dsp.adcrange[inch])
        if outch != 16:
            ramp_read_outdata = cnv.bv(rdata[0], 'd', self.dsp.dacrange[outch])
        else:
            ramp_read_outdata = cnv.bv(rdata[0], 20)
        self.etest.rtest_ramp_read_indata += [ramp_read_indata]
        self.etest.rtest_ramp_read_outdata += [ramp_read_outdata]

        self.etest.ptr2 += 1

        if self.etest.ptr2 >= len(self.etest.rtest_ramp_read_outdata):
            tmp1 = self.etest.rtest_ramp_read_outdata
            self.etest.rtest_ramp_read_outdata = [] * (len(self.etest.rtest_ramp_read_outdata) * 2)
            self.etest.rtest_ramp_read_outdata[:len(tmp1)] = tmp1
        self.etest.rtest_output_curve2.setData(self.etest.rtest_ramp_read_outdata[:self.etest.ptr2])

        if self.etest.ptr2 >= len(self.etest.rtest_ramp_read_indata):
            tmp2 = self.etest.rtest_ramp_read_indata
            self.etest.rtest_ramp_read_indata = [] * (len(self.etest.rtest_ramp_read_indata) * 2)
            self.etest.rtest_ramp_read_indata[:len(tmp2)] = tmp2
        self.etest.rtest_output_curve3.setData(self.etest.rtest_ramp_read_indata[:self.etest.ptr2])

    # Ramp Test | update ramp data
    def ramp_update(self, channel):
        ramp_data = self.dsp.current_last(channel)
        channel = channel - 16
        if channel != 16:   # 16 bit dac
            self.etest.rtest_ramp_data += [cnv.bv(ramp_data, 'd', self.dsp.dacrange[channel])]
        else:               # 20 bit dac
            self.etest.rtest_ramp_data += [cnv.bv(ramp_data, '20')]
        self.etest.ptr2 += 1

        if self.etest.ptr2 >= len(self.etest.rtest_ramp_data):
            tmp1 = self.etest.rtest_ramp_data
            self.etest.rtest_ramp_data = [] * (len(self.etest.rtest_ramp_data) * 2)
            self.etest.rtest_ramp_data[:len(tmp1)] = tmp1
        self.etest.rtest_output_curve1.setData(self.etest.rtest_ramp_data[:self.etest.ptr2])

    # Ramp Test | ramp signal slot
    def rtest_ramp_slot(self, index, inch, outch, init, final, step_size):
        threading.Thread(target=(lambda: self.rtest_ramp(index, inch, outch, init, final, step_size))).start()

    # Square Wave | setup square
    def swave_start(self, ch, voltage1, voltage2):
        if self.etest.idling:
            self.etest.idling = False
            self.etest.pushButton_Start_SWave.setText("Stop")   # change ramp button text
            self.dsp.square(ch + 16, voltage1, voltage2)        # dsp square wave function
            self.etest.idling = True
            self.etest.pushButton_Start_SWave.setText("Start")  # reset start button

    # Square Wave | start signal slot
    def swave_start_slot(self, ch, voltage1, voltage2):
        threading.Thread(target=(lambda: self.swave_start(ch, voltage1, voltage2))).start()

    # Oscilloscope | set up osc
    def osci_start(self, ch, mode, N, avg_times, delay):
        if self.etest.idling:
            self.etest.enable_serial(False)
            self.etest.idling = False
            if mode == 0:   # N sample
                rdata = self.dsp.osc_N(ch*4+0xC0, N, avg_times, delay)
                data = cnv.bv(rdata, 'a', self.dsp.adcrange[ch])
                self.etest.osci_nsample_data = data
                self.etest.osci_nsample_curve.setData(self.etest.osci_nsample_data)
            else:           # Continuous
                self.etest.pushButton_SorS_Osci.setText("Stop")
                self.etest.pushButton_SorS_Osci.setEnabled(True)
                self.dsp.osc_C(ch*4+0xC0, avg_times, delay)
            self.etest.idling = True
            self.etest.enable_serial(True)
            self.etest.pushButton_SorS_Osci.setText("Start")

    # Oscilloscope | start signal slot
    def osci_start_slot(self, ch, mode, N, avg_times, delay):
        threading.Thread(target=(lambda: self.osci_start(ch, mode, N, avg_times, delay))).start()

    # Oscilloscope | update continuous mode data
    def osci_update(self, rdata):
        ch = self.etest.osci_get_ch()
        data = cnv.bv(rdata, 'a', self.dsp.adcrange[ch])
        self.etest.osci_continuous_data += [data]
        self.etest.ptr1 += 1
        if self.etest.ptr1 >= len(self.etest.osci_continuous_data):
            tmp1 = self.etest.osci_continuous_data
            self.etest.osci_continuous_data = [] * (len(self.etest.osci_continuous_data) * 2)
            self.etest.osci_continuous_data[:len(tmp1)] = tmp1
        self.etest.osci_continuous_curve.setData(self.etest.osci_continuous_data[:self.etest.ptr1])

    # init current tab
    def init_tab_slot(self, index):
        if index == 0:
            self.etest.init_io(self.dsp.dacrange, self.dsp.adcrange, self.dsp.lastdigital, self.dsp.lastgain, self.dsp.lastdac, self.dsp.last20bit)
        elif index == 1:
            self.etest.init_rtest(self.dsp.adcrange, self.dsp.dacrange)
        elif index == 2:
            self.etest.init_swave(self.dsp.dacrange)
        elif index == 3:
            self.etest.init_osci()
        elif index == 4:
            self.etest.init_echo()
        elif index == 5:
            self.etest.init_ftest()

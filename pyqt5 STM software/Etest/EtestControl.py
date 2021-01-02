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
from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox
from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSignal, Qt, QMetaObject, QSettings
from sequence import mySequence
from MainMenu import myMainMenu
import conversion as cnv
import threading
import numpy as np
import math
import time

class myEtestControl(myMainMenu):

    # I/O | range changed slot
    def range_changed_slot(self, index, ran):
        if index == 0:                                              # adc range changed
            ch = self.etest.adc_ch.currentIndex()                   # get channel from view
            self.dsp.adc_W((ch+5) << 1, ran)                        # output to dsp
        else:                                                       # dac range changed
            ch = self.etest.dac_ch.currentIndex()                   # get channel from view
            self.dsp.dac_range(ch, ran)                             # output to dsp
            self.etest.set_dac_spinBox_range(ran)                           # update spinBox range from dsp
            self.etest.dac_range[0] = ran                                   # update range variable
            self.etest.load_dac_output(0, self.dsp.current_last(ch+16))     # update dac output view from dsp

    # I/O & Ramp Text & Square Wave | channel changed slot
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
                self.etest.dac_range[1] = self.dsp.dacrange[ch]                 # update range variable
            else:                                                               # 20 bit dac
                self.etest.set_spinBox_range(index, ch, 16)                     # update spinBox range from dsp
                self.etest.set_range_text(index, 16)                            # update range view from dsp
                self.etest.dac_range[1] = 16                                    # update range variable
        elif index == 4:                                                        # square wave output
            if ch != 16:                                                        # 16 bit dac
                self.etest.set_spinBox_range(index, ch, self.dsp.dacrange[ch])  # update spinBox range from dsp
                self.etest.set_range_text(index, self.dsp.dacrange[ch])         # update range view from dsp
                self.etest.dac_range[2] = self.dsp.dacrange[ch]                 # update range variable
            else:                                                               # 20 bit dac
                self.etest.set_spinBox_range(index, ch, 16)                     # update spinBox range from dsp
                self.etest.set_range_text(index, 16)                            # update range view from dsp
                self.etest.dac_range[2] = 16                                    # update range variable

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
            origin = self.dsp.current_last(outch+16)                            # get initial value
            if index == 0:                                                      # ramp button clicked
                self.etest.pushButton_Ramp_RTest.setText("Stop")                # change ramp button text
                self.etest.pushButton_Ramp_RTest.setEnabled(True)               # enable stop button
                self.dsp.rampTo(outch+16, init, 10, 100, 0, False)              # ramp to initial value
                self.dsp.rampTo(outch+16, final, step_size, 10000, 0, True)     # ramp to final value
                self.dsp.rampTo(outch+16, origin, 10, 100, 0, False)            # ramp back to original value
            else:                                                               # ramp read button clicked
                self.etest.pushButton_RRead_RTest.setText("Stop")               # change ramp read button text
                self.etest.pushButton_RRead_RTest.setEnabled(True)              # enable stop button
                command = 4 * inch + 0xC0
                read_seq = mySequence([command], [10])
                direction = (final - init) >= 0
                step_num = int(abs((final - init)/step_size))
                self.dsp.rampTo(outch+16, init, 10, 100, 0, False)              # ramp to initial value
                self.dsp.rampMeasure(outch+16, step_num, step_size, 1, 10000, direction, read_seq)  # ramp meaure to final value
                self.dsp.rampTo(outch+16, origin, 10, 100, 0, False)            # ramp back to original value
            self.etest.idling = True
            self.etest.enable_serial(True)
            self.etest.pushButton_Ramp_RTest.setText("Ramp")                    # reset ramp button
            self.etest.pushButton_RRead_RTest.setText("Ramp read")              # reset ramp read button

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
        self.etest.rtest_output_curve.setData(self.etest.rtest_ramp_read_outdata[:self.etest.ptr2])

        if self.etest.ptr2 >= len(self.etest.rtest_ramp_read_indata):
            tmp2 = self.etest.rtest_ramp_read_indata
            self.etest.rtest_ramp_read_indata = [] * (len(self.etest.rtest_ramp_read_indata) * 2)
            self.etest.rtest_ramp_read_indata[:len(tmp2)] = tmp2
        self.etest.rtest_input_curve.setData(self.etest.rtest_ramp_read_indata[:self.etest.ptr2])

    # Ramp Test | update ramp data
    def ramp_update(self, channel, ramp_data):
        # ramp_data = self.dsp.current_last(channel)
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
        self.etest.rtest_output_curve.setData(self.etest.rtest_ramp_data[:self.etest.ptr2])

    # Ramp Test | ramp signal slot
    def rtest_ramp_slot(self, index, inch, outch, init, final, step_size):
        threading.Thread(target=(lambda: self.rtest_ramp(index, inch, outch, init, final, step_size))).start()

    # Square Wave | setup square
    def swave_start(self, ch, voltage1, voltage2, delay):
        if self.etest.idling:
            self.etest.enable_serial(False)
            self.etest.idling = False
            self.etest.pushButton_Start_SWave.setText("Stop")       # change ramp button text
            self.etest.pushButton_Start_SWave.setEnabled(True)      # change ramp button text
            self.dsp.square(ch + 16, voltage1, voltage2, delay)     # dsp square wave function
            self.etest.idling = True
            self.etest.enable_serial(True)
            self.etest.pushButton_Start_SWave.setText("Start")      # reset start button

    # Square Wave | start signal slot
    def swave_start_slot(self, ch, voltage1, voltage2, delay):
        threading.Thread(target=(lambda: self.swave_start(ch, voltage1, voltage2, delay))).start()

    # Oscilloscope | set up osc
    def osci_start(self, ch, mode, N, avg_times, delay):
        if self.etest.idling:
            self.etest.enable_serial(False)
            self.etest.idling = False
            if mode == 0:   # N sample
                rdata = self.dsp.osc_N(ch*4+0xC0, N, avg_times, delay)
                # for i in rdata:
                #     print(hex(i))
                data = [cnv.bv(i, 'a', self.dsp.adcrange[ch]) for i in rdata]
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

    # Oscilloscope | update continuous mode data (dsp.oscc_signal slot)
    def osci_update(self, rdata):
        ch = self.etest.osci_get_ch()
        data = cnv.bv(rdata, 'a', self.dsp.adcrange[ch])
        self.etest.ptr1 += 1            # plot count
        if self.etest.ptr1 <= 200:      # direct plot when number of data <= 200
            self.etest.osci_continuous_data = np.append(self.etest.osci_continuous_data, data)
        else:                           # rolling plot when number of data > 200
            self.etest.osci_continuous_data[:-1] = self.etest.osci_continuous_data[1:]
            self.etest.osci_continuous_data[-1] = data
        self.etest.osci_continuous_curve.setData(self.etest.osci_continuous_data[1:])
        self.etest.osci_continuous_curve.setPos(self.etest.ptr1, 0)

    # Echo | start setup
    def echo_start(self):
        if self.etest.idling:
            self.etest.enable_serial(False)
            self.etest.idling = False
            self.etest.pushButton_Start_Echo.setText("Stop")
            self.etest.pushButton_Start_Echo.setEnabled(True)
            self.etest.pushButton_Query_Echo.setEnabled(True)
            self.dsp.echoLoop()
            self.etest.idling = True
            self.etest.enable_serial(True)
            self.etest.pushButton_Query_Echo.setEnabled(False)
            self.etest.pushButton_Start_Echo.setText("Start")

    # Echo | start signal slot
    def echo_start_slot(self):
        threading.Thread(target=(lambda: self.echo_start())).start()

    # Echo | query signal slot
    def echo_query_slot(self, input):
        if input == 256:
            QMessageBox.warning(None, "Echo", "Can not send STOP command!", QMessageBox.Ok)
        elif input > 256:
            QMessageBox.warning(None, "Echo", "Can not send integer bigger than 256!", QMessageBox.Ok)
        else:
            output = str(self.dsp.echo(input))
            self.etest.lineEdit_2_Echo.setText(output)

    # Feedback Test | test loop
    def ftest_loop(self, outch):
            self.etest.ftest_stop = False                                  # set stop flag to false

            # init range and gain
            self.dsp.gain(2, 0)                                            # init Z1 gain to 0.1
            self.dsp.gain(3, 0)                                            # init Z2 gain to 10
            self.dsp.adc_W((6 + 5) << 1, 0)                                # init Zout range to +/-10.24V
            self.dsp.dac_range(outch, 10)                                  # init dac output range to +/-10V
            self.dsp.dac_range(5, 10)                                      # init I set range to +/-10V

            # start loop
            while True:
                if self.etest.ftest_stop:  # Wait until external source change the stop flag
                    break
                self.etest.ptr3 += 1                                       # plot count +1

                # Input | Zout (feedback Z)
                Zout = self.dsp.adc(6 * 4 + 0xC0)                          # load Z out from dsp
                Zout_val = cnv.bv(Zout, 'a', self.dsp.adcrange[6])         # convert bits to value

                # Output | Vout (output voltage)
                A = self.etest.ftest_parm[0]                               # require A value
                Z0 = self.etest.ftest_parm[1]                              # require Z0 value
                Vout_val = math.exp(A * (Zout_val - Z0))                   # calculate output voltage
                if Vout_val > 9.999694:                                    # if calculated value is too large
                    Vout_val = 9.999694                                    # set output voltage to maximum
                elif Vout_val < -10.0:                                     # if calculated value is too small
                    Vout_val = -10.0                                       # set output voltage to minimum
                Vout = cnv.vb(Vout_val, 'd', self.dsp.dacrange[outch])     # convert value to bits
                self.dsp.dac_W(outch + 16, Vout)                           # send to dsp out channel

                # Plot | Zout (feedback Z) and  Vout (output voltage)
                if self.etest.ptr3 <= 200:                                 # cumulative plot for first 200 points
                    self.etest.ftest_zout_data = np.append(self.etest.ftest_zout_data, Zout_val)
                    self.etest.ftest_input_curve.setData(self.etest.ftest_zout_data[1:])
                    self.etest.ftest_vout_data = np.append(self.etest.ftest_vout_data, Vout_val)
                    self.etest.ftest_output_curve.setData(self.etest.ftest_vout_data[1:])
                else:                                                      # rolling plot after 200 point
                    self.etest.ftest_zout_data[:-1] = self.etest.ftest_zout_data[1:]
                    self.etest.ftest_zout_data[-1] = Zout_val
                    self.etest.ftest_input_curve.setData(self.etest.ftest_zout_data)
                    self.etest.ftest_vout_data[:-1] = self.etest.ftest_vout_data[1:]
                    self.etest.ftest_vout_data[-1] = Vout_val
                    self.etest.ftest_output_curve.setData(self.etest.ftest_vout_data)
                self.etest.ftest_input_curve.setPos(self.etest.ptr3, 0)
                self.etest.ftest_output_curve.setPos(self.etest.ptr3, 0)

                # Output | Isetpoint (I set point)
                Ispt_val = self.etest.spinBox_IsptInput_FTest.value()      # require I set point value
                Ispt_voltage_val = -10.0 * math.log(Ispt_val, 10)
                if Ispt_voltage_val > 9.999694:                            # if converted value is too large
                    Ispt_voltage_val = 9.999694                            # set output voltage to maximum
                elif Ispt_voltage_val < -10.0:                             # if converted value is too small
                    Ispt_voltage_val = -10.0                               # set output voltage to minimum
                Isetpoint = cnv.vb(Ispt_voltage_val, 'd', self.dsp.dacrange[5])
                self.dsp.dac_W(5+16, Isetpoint)                            # send to dsp

                # Output | Feedback and Retract
                feedback_on = self.etest.radioButton_ON_Fdbk.isChecked()   # require feedback status
                self.dsp.digital_o(2, feedback_on)                         # send to dsp
                retract_on = self.etest.radioButton_ON_Retr.isChecked()    # require retract status
                self.dsp.digital_o(3, retract_on)                          # send to dsp

                # Delay | from view
                delay = self.etest.ftest_parm[2]                           # require delay value(ms)
                if delay < 0.1:
                    delay = 0.1
                time.sleep(delay/1000)                                     # take some time(s)

            if self.etest.ftest_stop:
                # self.etest.ftest_output_curve.clear()                      # clear old plot
                # self.etest.ftest_input_curve.clear()                       # clear old plot
                # self.etest.ftest_output_curve.setPos(0, 0)                 # reset plot origin
                # self.etest.ftest_input_curve.setPos(0, 0)                  # reset plot origin
                pass
            self.etest.ftest_stop = True                                   # flip stop flag

    # Feedback Test | start setup
    def ftest_start(self, outch):
        if self.etest.idling:
            self.etest.enable_serial(False)
            self.etest.idling = False
            self.etest.pushButton_Start_FTest.setText("Stop")
            self.etest.pushButton_Start_FTest.setEnabled(True)
            # allow I set point, Feedback and Retract input
            self.etest.spinBox_IsptInput_FTest.setEnabled(True)
            self.etest.radioButton_ON_Fdbk.setEnabled(True)
            self.etest.radioButton_OFF_Fdbk.setEnabled(True)
            self.etest.radioButton_ON_Retr.setEnabled(True)
            self.etest.radioButton_OFF_Retr.setEnabled(True)
            # self defined loop made up of dsp functions, including stop flag
            self.ftest_loop(outch)
            self.etest.idling = True
            self.etest.enable_serial(True)
            self.etest.pushButton_Start_FTest.setText("Start")

    # Feedback Test | start signal slot
    def ftest_start_slot(self, outch):
        threading.Thread(target=(lambda: self.ftest_start(outch))).start()

    # init current tab
    def init_tab_slot(self, index):
        if self.etest.idling:                                       # no serial process ongoing
            self.etest.mode = index                                 # save current tab index
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
                self.etest.init_ftest(self.dsp.lastdigital)
        else:                                                       # serial process ongoing
            for tab in range(6):
                if tab != self.etest.mode:
                    self.etest.Etest.setTabEnabled(tab, False)
            # if index != self.etest.mode:
            #     self.etest.Etest.setCurrentIndex(self.etest.mode)   # return to former tab
            #     QMessageBox.warning(None, "Etest", "Process ongoing!", QMessageBox.Ok)


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


    def init_etest(self):
        # Enable serial related modules
        self.enable_serial(self.dsp.succeed)

        # load status from dsp
        self.dither0_on.setChecked(self.dsp.lastdigital[0])
        self.dither0_off.setChecked(not self.dsp.lastdigital[0])
        self.dither1_on.setChecked(self.dsp.lastdigital[1])
        self.dither1_off.setChecked(not self.dsp.lastdigital[1])
        self.feedback_on.setChecked(self.dsp.lastdigital[2])
        self.feedback_off.setChecked(not self.dsp.lastdigital[2])
        self.retract_on.setChecked(self.dsp.lastdigital[3])
        self.retract_off.setChecked(not self.dsp.lastdigital[3])
        self.coarse.setChecked(self.dsp.lastdigital[4])
        self.fine.setChecked(not self.dsp.lastdigital[4])
        self.rotation.serChecked(not self.dsp.lastdigital[5])
        self.translation.setCheked(self.dsp.setChecked[5])
        self.load_x_gain()
        self.load_y_gain()
        self.load_z1_gain()
        self.load_z2_gain()

        # connect signals
        self.adc_range_signal.connect(self.adc_indication_ran)
        self.adc_ch_signal.connect(self.adc_indication_ch)
        self.adc_input_signal.connect(self.adc_indication)
        self.dac_send_signal.connect(self.dac_send_slot)
        self.bit20_dac_signal.connect(self.bit20_dac_slot)
        self.rtest_ramp_signal.connect(self.etest_ramp_start)
        self.swave_start_signal.connect(self.etest_ramp_start)

    # Load X gain from DSP
    def load_x_gain(self):
        if self.dsp.lastgain[0] == 0:
            self.xgain10.setChecked(True)
        elif self.dsp.lastgain[0] == 1:
            self.xgain1.setChecked(True)
        elif self.dsp.lastgain[0] == 3:
            self.xgain0_1.setChecked(True)

    # Load Y gain from DSP
    def load_y_gain(self):
        if self.dsp.lastgain[1] == 0:
            self.ygain10.setChecked(True)
        elif self.dsp.lastgain[1] == 1:
            self.ygain1.setChecked(True)
        elif self.dsp.lastgain[1] == 3:
            self.ygain0_1.setChecked(True)

    # Load Z1 gain from DSP
    def load_z1_gain(self):
        if self.dsp.lastgain[2] == 0:
            self.z1gain10.setChecked(True)
        elif self.dsp.lastgain[2] == 1:
            self.z1gain1.setChecked(True)
        elif self.dsp.lastgain[2] == 3:
            self.z1gain0_1.setChecked(True)

    # Load Z2 gain from DSP
    def load_z2_gain(self):
        if self.dsp.lastgain[3] == 0:
            self.z2gain10.setChecked(True)
        elif self.dsp.lastgain[3] == 1:
            self.z2gain1.setChecked(True)
        elif self.dsp.lastgain[3] == 3:
            self.z2gain0_1.setChecked(True)

    def adc_indication_ch(self,ch):
        return ch

    def adc_indication_ran(self,ran):
        return ran

    def adc_indication(self):
        in_ch = self.adc_indication_ch()
        range = self.adc_indication_ran()
        self.dsp.adcrange[in_ch] = range
        # self.dsp.adc_R(,)   # need thread? read from dsp

    def dac_send_slot(self, out_ch, out_range, out_val, out_iset_val):
        self.dsp.offset[15] = out_iset_val
        self.dsp.dac_range(out_ch, out_range)
        self.dsp.dac_W(out_ch, out_val)
        self.dsp.loadOffset()
        if bits != self.dsp.lastdac[out_ch]:  # If it is a real output
            self.dsp.dac_range(out_ch, out_range)
        else:
            self.scrollBar_Input_Zoffset.setValue(self.dsp.lastdac[out_ch] - 0x8000)  # Update indication
            self.spinBox_Indication_Zoffset.setValue(self.dsp.lastdac[out_ch] - 0x8000)  # Upadate input spin box

    def bit20_dac_slot(self,out_val):
        # self.dsp.20bit_W(,)
        pass

    # digital output when radio button changed
    def io_radio(self):
        # Dither 0
        self.dither0_on.toggled.connect(lambda : self.dither0_slot(True))
        self.dither0_off.toggled.connect(lambda : self.dither0_slot(False))
        # Dither 1
        self.dither1_on.toggled.connect(lambda : self.dither1_slot(True))
        self.dither1_off.toggled.connect(lambda : self.dither1_slot(False))
        # Coarse
        self.coarse.toggled.connect(lambda : self.coarse_slot(True))
        self.fine.toggled.connect(lambda : self.coarse_slot(False))
        # Feedback
        self.feedback_on.toggled.connect(lambda : self.feedback_slot(True))
        self.feedback_off.toggled.connect(lambda : self.feedback_slot(False))
        # Retract
        self.retract_on.toggled.connect(lambda : self.retract_slot(True))
        self.retract_off.toggled.connect(lambda : self.retract_slot(False))
        # Rotation
        self.rotation.toggled.connect(lambda : self.rotation_slot(False))
        self.translation.toggled.connect(lambda : self.rotation_slot(True))
        # X gain
        self.xgain0_1.toggled.connect(lambda : self.xgain_slot(3))
        self.xgain1.toggled.connect(lambda : self.xgain_slot(1))
        self.xgain10.toggled.connect(lambda : self.xgain_slot(0))
        # Y gain
        self.ygain0_1.toggled.connect(lambda : self.ygain_slot(3))
        self.ygain1.toggled.connect(lambda : self.ygain_slot(1))
        self.ygain10.toggled.connect(lambda : self.ygain_slot(0))
        # Z1 gain
        self.z1gain0_1.toggled.connect(lambda : self.z1gain_slot(0))
        self.z1gain1.toggled.connect(lambda : self.z1gain_slot(1))
        self.z1gain10.toggled.connect(lambda : self.z1gain_slot(3))
        # Z2 gain
        self.z2gain0_1.toggled.connect(lambda : self.z2gain_slot(3))
        self.z2gain1.toggled.connect(lambda : self.z2gain_slot(1))
        self.z2gain10.toggled.connect(lambda : self.z2gain_slot(0))

    def dither0_slot(self, bool):
        self.dsp.lastdigital[0] = bool

    def dither1_slot(self, bool):
        self.dsp.lastdigital[1] = bool

    def coarse_slot(self, bool):
        self.dsp.lastdigital[4] = bool

    def feedback_slot(self, bool):
        self.dsp.lastdigital[2] = bool

    def retract_slot(self, bool):
        self.dsp.lastdigital[3] = bool

    def rotation_slot(self, bool):
        self.dsp.lastdigital[5] = bool

    def xgain_slot(self, gain):
        self.dsp.lastgain[0] = gain

    def ygain_slot(self, gain):
        self.dsp.lastgain[1] = gain

    def z1gain_slot(self, gain):
        self.dsp.lastgain[2] = gain

    def z2gain_slot(self, gain):
        self.dsp.lastgain[3] = gain

    def etest_ramp():
        pass

    def etest_ramp_start(self):
        # lambda
        threading.Thread(self.etest_ramp).start()





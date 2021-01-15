# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:18:34 2020
@author: yaoji
"""

import sys
import io
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup, QFileDialog
from PyQt5.QtGui import QPixmap, QPen
from PyQt5.QtCore import pyqtSignal , Qt, QRectF
from PyQt5 import QtCore

from Scan_ui import Ui_Scan
from Spectroscopy import mySpc
from Deposition import myDeposition
from Track import myTrack
from ScanOptions import myScanOptions
from SendOptions import mySendOptions
from PointEditor import myPointEditor
from SequenceList import mySequenceList
from ScanInfo import myScanInfo
from LockIn import myLockIn
from DigitalSignalProcessor import myDSP

from DataStruct import ScanData
from symbols import mySymbols
from images import myImages

import pyqtgraph as pg
import numpy as np
import functools as ft
import cv2 as cv
import scipy.io

class myScan_(QWidget, Ui_Scan):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_property()
        self.init_UI()
        
    def init_property(self):
        # Modeules
        self.spc = mySpc()
        self.dep = myDeposition()
        self.track = myTrack()
        self.scan_options = myScanOptions()
        self.send_options = mySendOptions()
        self.point_editor = myPointEditor()
        self.seq_list = mySequenceList()
        self.info = myScanInfo()
        self.lockin = myLockIn()
        
        # Data
        self.data = ScanData()
        self.dlg = QFileDialog()
        self.file_idex = [0, 0]

        # Flags
        self.mode = 0               # Scan mode: Scan(0), Spectroscopy(1), Deposition(2)
        self.stop = True
        self.idling = True
        self.saved = True
        self.bias_dac = False       # Bias DAC selection
        self.bias_ran = 9           # Bias range
        
        # XY and image variables
        self.last_xy = [0]*4            # Xin(0), Yin(1), X offset(2), Y offset(3) --> values sent last time
        self.current_xy = [0]*4         # Xin(0), Yin(1), X offset(2), Y offset(3)
        self.scan_size = [128, 25600]   # Scan size(0), Step size(1)
        self.imagine_gain = 10          # X/Y gain imaginary value
        self.tilt = [0.0] * 2           # Tilt X, Tilt Y
        
        # Sequence lists and selected sequence
        self.scan_seq_list = []
        self.dep_seq_list = []
        self.spc_seq_list = []
        self.scan_seq_selected = -1
        self.dep_seq_selected = -1
        self.spc_seq_selected = -1
        
        # Spectroscopy related
        self.point_list = [[0, 0]]          # Point list for mapping [[X position, Y position]]
        self.pattern = [0, 0, 1]            # Pattern for matching [X position, Y position, Size]
        
    def init_UI(self):
        # Init ui position and size
        screen = QDesktopWidget().screenGeometry()
        self.resize(1029, 549)
        size = self.frameGeometry()
        self.move(int((screen.width()-size.width())/2), int((screen.height()-size.height())/2))
        self.setFixedSize(self.width(), self.height())

        # radioButton | X/Y gain
        self.XY_gain_group = QButtonGroup()
        self.XY_gain_group.addButton(self.radioButton_Gain0_1_XY, 3)
        self.XY_gain_group.addButton(self.radioButton_Gain1_XY, 1)
        self.XY_gain_group.addButton(self.radioButton_Gain10_XY, 0)
        self.XY_gain_group.buttonToggled[int, bool].connect(self.xy_gain_cnv)

        # spinBox | X/Y offset and X/Y in
        self.spinBox_Xin_XY.editingFinished.connect(lambda: self.xy_in_cnv(True, 0, 0))
        self.spinBox_Yin_XY.editingFinished.connect(lambda: self.xy_in_cnv(True, 1, 0))
        self.spinBox_Xoffset_XY.editingFinished.connect(lambda: self.xy_off_cnv(True, 0, 0))
        self.spinBox_Yoffset_XY.editingFinished.connect(lambda: self.xy_off_cnv(True, 1, 0))

        # scrollBar | X/Y offset and X/Y in
        self.scrollBar_Xin_XY.valueChanged.connect(ft.partial(self.xy_in_cnv, False, 0))
        self.scrollBar_Yin_XY.valueChanged.connect(ft.partial(self.xy_in_cnv, False, 1))
        self.scrollBar_Xoffset_XY.valueChanged.connect(ft.partial(self.xy_off_cnv, False, 0))
        self.scrollBar_Yoffset_XY.valueChanged.connect(ft.partial(self.xy_off_cnv, False, 1))

        # spinBox | Scan size and Step Size
        self.spinBox_ScanSize_ScanControl.editingFinished.connect(lambda: self.scan_size_cnv(True, 0, 0))
        self.spinBox_StepSize_ScanControl.editingFinished.connect(lambda: self.scan_size_cnv(True, 1, 0))

        # scrollBar | Scan size and Step Size
        self.scrollBar_ScanSize_ScanControl.valueChanged.connect(ft.partial(self.scan_size_cnv, False, 0))
        self.scrollBar_StepSize_ScanControl.valueChanged.connect(ft.partial(self.scan_size_cnv, False, 1))
        
    def init_scan(self, dsp, bias_dac, preamp_gain):
        succeed = dsp.succeed
        bias_ran = dsp.dacrange[13]
        
        # Set up view in case of successfully finding DSP
        self.radioButton_Gain10_XY.setEnabled(succeed)
        self.radioButton_Gain1_XY.setEnabled(succeed)
        self.radioButton_Gain0_1_XY.setEnabled(succeed)
        self.pushButton_Send_XY.setEnabled(succeed)
        self.pushButton_Zero_XY.setEnabled(succeed)
        self.pushButton_Start_Scan.setEnabled(succeed)
        # self.pushButton_SaveAll_Scan.setEnabled(not self.data.data)
        # self.pushButton_Info_Scan.setEnabled(not self.data.data)

        if dsp.lastgain[0] == 0:
            self.radioButton_Gain10_XY.setChecked(True)
            self.imagine_gain = 100
        elif dsp.lastgain[0] == 1:
            self.radioButton_Gain1_XY.setChecked(True)
            self.imagine_gain = 10
        elif dsp.lastgain[0] == 3:
            self.radioButton_Gain0_1_XY.setChecked(True)
            self.imagine_gain = 1
        
        self.scrollBar_Xin_XY.setValue(dsp.lastdac[0] - 32768)
        self.scrollBar_Yin_XY.setValue(dsp.lastdac[15] - 32768)
        self.scrollBar_Xoffset_XY.setValue(dsp.lastdac[1] - 32768)
        self.scrollBar_Yoffset_XY.setValue(dsp.lastdac[14] - 32768)

        self.dep.init_deposition(succeed, bias_dac, bias_ran, self.dep_seq_list, self.dep_seq_selected)
        self.track.init_track(succeed)
        self.spc.init_spc(succeed, bias_dac, bias_ran)


    # XY in conversion
    def xy_in_cnv(self, flag, xy, value):
        scroll = self.scrollBar_Yin_XY if xy else self.scrollBar_Xin_XY
        spin = self.spinBox_Yin_XY if xy else self.spinBox_Xin_XY
        ul = min(int((3276700 - self.current_xy[xy + 2]) / self.imagine_gain), 32767)
        ll = max(int((-3276800 - self.current_xy[xy + 2]) / self.imagine_gain), -32768)
        if flag:
            value = spin.value()
        value = min(ul, max(ll, value))
        spin.setValue(value)
        scroll.setValue(value)
        self.current_xy[xy] = value * self.imagine_gain
        # Set XY offset range
        self.xy_offset_range(xy)
        
        # !!! Set graphic
        
    # XY offset conversion
    def xy_off_cnv(self, flag, xy, value):
        scroll = self.scrollBar_Yoffset_XY if xy else self.scrollBar_Xoffset_XY
        spin = self.spinBox_Yoffset_XY if xy else self.spinBox_Xoffset_XY
        scan_area = self.scan_size[0] * self.scan_size[1]
        ul = min(int((3276700 - self.current_xy[xy]) / 100), int((3276700 - (scan_area / 2)) / 100))
        ll = max(int((-3276800 - self.current_xy[xy]) / 100), int((-3276800 + (scan_area / 2)) / 100))
        if flag:
            value = spin.value()
        value = min(ul, max(ll, value))
        spin.setValue(value)
        scroll.setValue(value)
        self.current_xy[xy + 2] = value * self.imagine_gain
        # Set XY in and scan size range
        self.xy_in_range(xy)
        self.scan_size_range()
        
        # !!! Set graphic
        
    # Scan size converstion
    def scan_size_cnv(self, flag, index, value):
        scroll = self.scrollBar_StepSize_ScanControl if index else self.scrollBar_ScanSize_ScanControl
        spin = self.spinBox_StepSize_ScanControl if index else self.spinBox_ScanSize_ScanControl
        ul = 65535 if index else 1024
        gain = self.imagine_gain if index else 1
        area_ul = int(2 * min(3276700 - self.current_xy[2], 3276700 - self.current_xy[3], 3276800 + self.current_xy[2], 3276800 + self.current_xy[3]))
        ul = min(ul, int(area_ul / self.scan_size[1 - index] / gain))
        if flag:
            value = spin.value()
        value = min(ul, value)
        if (not index) and (value % 2):
            value -= 1
        spin.setValue(value)
        scroll.setValue(value)
        self.scan_size[index] = value * gain
        # Set XY offset range and the other scan size range
        self.xy_in_range(0)
        self.xy_in_range(1)
        self.scan_size_range()
        
        # !!! Set graphic
    
        
    # Set XY in spin boxes range
    def xy_in_range(self, xy):
        spin = self.spinBox_Yin_XY if xy else self.spinBox_Xin_XY
        ul = min(int((3276700 - self.current_xy[xy + 2]) / self.imagine_gain), 32767)
        ll = max(int((-3276800 - self.current_xy[xy + 2]) / self.imagine_gain), -32768)
        spin.setMaximum(ul)
        spin.setMinimum(ll)
        
    # Set XY offset spin boxes range
    def xy_offset_range(self, xy):
        spin = self.spinBox_Yoffset_XY if xy else self.spinBox_Xoffset_XY
        scan_area = self.scan_size[0] * self.scan_size[1]
        ul = min(int((3276700 - self.current_xy[0]) / 100), int((3276700 - (scan_area / 2)) / 100))
        ll = max(int((-3276800 - self.current_xy[0]) / 100), int((-3276800 + (scan_area / 2)) / 100))
        spin.setMaximum(ul)
        spin.setMinimum(ll)
        
    # Set scan size spin boxes range
    def scan_size_range(self):
        area_ul = int(2 * min(3276700 - self.current_xy[2], 3276700 - self.current_xy[3], 3276800 + self.current_xy[2], 3276800 + self.current_xy[3]))
        size_ul = min(65535, int(area_ul / self.scan_size[0] / self.imagine_gain))
        num_ul = min(1024, int(area_ul / self.scan_size[1]))
        if (num_ul % 2):
            num_ul -= 1
        self.spinBox_StepSize_ScanControl.setMaximum(size_ul)
        self.spinBox_ScanSize_ScanControl.setMaximum(num_ul)

    # enable/disable X/Y gain based on X/Y in
    def enable_gain(self):
        enable = (self.last_xy[0] == 0 and self.last_xy[1] == 0)
        self.radioButton_Gain0_1_XY.setEnabled(enable)     
        self.radioButton_Gain10_XY.setEnabled(enable)
        self.radioButton_Gain1_XY.setEnabled(enable)
        
    # XY gain conversion
    def xy_gain_cnv(self, index, state):
        if state:
            self.imagine_gain = 100 * (index == 0) + 10 *(index == 1) + (index == 3)    
            self.scrollBar_Yin_XY.setValue(0)
            self.scrollBar_Xin_XY.setValue(0)            
            self.scrollBar_StepSize_ScanControl.setValue(1)
            self.scrollBar_ScanSize_ScanControl.setValue(2)
            
            self.last_xy[0] = int(self.label_Xin_XY.text()) * self.imagine_gain
            self.last_xy[1] = int(self.label_Yin_XY.text()) * self.imagine_gain

    # Enable serial
    def enable_serial(self, enable):
        self.XY_offset.setEnabled(enable)
        self.XY.setEnabled(enable)
        self.XY_gain.setEnabled(enable)
        self.options.setEnabled(enable)
        self.toolButton.setEnabled(enable)
        self.file_widget.setEnabled(enable)
        self.groupBox_ScanControl.setEnabled(enable)
        
        self.pushButton_Zero_XY.setEnabled(enable)
        self.pushButton_Send_XY.setEnabled(enable)
        self.pushButton_Start_Scan.setEnabled(enable)
        # !!! need to enable and disable mouse event
        
        # self.pushButton_SaveAll_Scan.setEnabled(enable and (not self.data.data))
        # self.pushButton_Info_Scan.setEnabled(enable and (not self.data.data))
        # self.pushButton_Load_Scan.setEnabled(enable)

        self.dep.enable_serial(enable)
        self.track.enable_serial(enable)
        self.spc.enable_serial(enable)    

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myScan_()
    dsp = myDSP()
    dsp.succeed = True
    window.init_scan(dsp, False, 9)
    window.show()
    sys.exit(app.exec_())

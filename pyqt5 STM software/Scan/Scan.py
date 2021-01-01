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
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup
from PyQt5.QtGui import QPixmap
from PyQt5.QtCore import pyqtSignal , Qt, QRectF
from PyQt5 import QtCore
from Scan_ui import Ui_Scan
from Spectroscopy import mySpc
from Deposition import myDeposition
from Track import myTrack
from Hop import myHop
from Manipulation import myManipulation
from ScanOptions import myScanOptions
from SendOptions import mySendOptions
from SequenceEditor import mySequenceEditor
import pyqtgraph as pg
import numpy as np
from symbols import mySymbols
import functools as ft

class myScan(QWidget, Ui_Scan):
    close_signal = pyqtSignal()
    gain_changed_signal = pyqtSignal(int, int)
    send_signal = pyqtSignal(int, int, int, int, int)
    stop_signal = pyqtSignal

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()
        self.spc = mySpc()
        self.depostion = myDeposition()
        self.track = myTrack()
        self.hop = myHop()
        self.manipulation = myManipulation()
        self.scan_options = myScanOptions()
        self.send_options = mySendOptions()
        # self.seq_editor = mySequenceEditor()
        #
        self.mode = 0       # Scan mode: Scan(0), Spectroscopy(1), Deposition(2)
                            # Track(3), Hop(4), Manipulation(5)
        self.idling = True

        self.last_xy = [0]*4     # Xin(0), Yin(1), X offset(2), Y offset(3)
        self.current_xy = [0]*4  # Xin(0), Yin(1), X offset(2), Y offset(3)
        self.scan_size = [1]*2   # Scan size(0), Step size(1)

    def init_UI(self):
        # init ui position and size
        screen = QDesktopWidget().screenGeometry()
        self.resize(1029, 549)
        size = self.frameGeometry()
        self.move(int((screen.width()-size.width())/2), int((screen.height()-size.height())/2))
        self.setFixedSize(self.width(), self.height())

        # self.close_signal.clicked(self.spc.close_signal)
        # self.close_signal.clicked(self.depostion.close_signal)
        # self.close_signal.clicked(self.track.close_signal)
        # self.close_signal.clicked(self.hop.close_signal)
        # self.close_signal.clicked(self.manipulation.close_signal)

        # PushButton | open windows
        self.pushButton_ScanOptions_Scan.clicked.connect(self.open_scan_options)
        self.pushButton_SendOptions_Scan.clicked.connect(self.open_send_options)
        # self.pushButton_SeqEditor_ScanControl.clicked.connect(self.open_seq_editor)

        # radioButton | X/Y gain
        self.XY_gain_group = QButtonGroup()
        self.XY_gain_group.addButton(self.radioButton_Gain0_1_XY, 3)
        self.XY_gain_group.addButton(self.radioButton_Gain1_XY, 1)
        self.XY_gain_group.addButton(self.radioButton_Gain10_XY, 10)
        self.XY_gain_group.buttonToggled[int, bool].connect(self.gain_changed_emit)

        # pushButton | send and zero
        self.pushButton_Send_XY.clicked.connect(lambda: self.send_emit(1))
        self.pushButton_Zero_XY.clicked.connect(lambda: self.send_emit(0))

        # spinBox | X/Y offset and X/Y in
        self.spinBox_XinInput_XY.editingFinished.connect(lambda: self.spin2scroll(0))
        self.spinBox_YinInput_XY.editingFinished.connect(lambda: self.spin2scroll(1))
        self.spinBox_XoffsetInput_XY.editingFinished.connect(lambda: self.spin2scroll(2))
        self.spinBox_YoffsetInput_XY.editingFinished.connect(lambda: self.spin2scroll(3))
        self.spinBox_XinIndication_XY.valueChanged.connect(self.enable_gain)
        self.spinBox_YinIndication_XY.valueChanged.connect(self.enable_gain)
        # self.spinBox_XinIndication_XY.valueChanged.connect(ft.partial(self.change_range, 0))
        # self.spinBox_YinIndication_XY.valueChanged.connect(ft.partial(self.change_range, 1))
        # self.spinBox_XoffsetIndication_XY.valueChanged.connect(ft.partial(self.change_range, 2))
        # self.spinBox_YoffsetIndication_XY.valueChanged.connect(ft.partial(self.change_range, 3))

        # scrollBar | X/Y offset and X/Y in
        self.scrollBar_Xin_XY.valueChanged.connect(ft.partial(self.scroll2spin, 0))
        self.scrollBar_Yin_XY.valueChanged.connect(ft.partial(self.scroll2spin, 1))
        self.scrollBar_Xoffset_XY.valueChanged.connect(ft.partial(self.scroll2spin, 2))
        self.scrollBar_Yoffset_XY.valueChanged.connect(ft.partial(self.scroll2spin, 3))

        # spinBox | Scan size and Step Size
        self.spinBox_ScanSize_ScanControl.editingFinished.connect(lambda: self.spin2scroll(4))
        self.spinBox_StepSize_ScanControl.editingFinished.connect(lambda: self.spin2scroll(5))

        # scrollBar | Scan size and Step Size
        self.scrollBar_ScanSize_ScanControl.valueChanged.connect(ft.partial(self.scroll2spin, 4))
        self.scrollBar_StepSize_ScanControl.valueChanged.connect(ft.partial(self.scroll2spin, 5))
        # self.spinBox_ScanSize_ScanControl.valueChanged.connect(ft.partial(self.change_range, 4))
        # self.spinBox_StepSize_ScanControl.valueChanged.connect(ft.partial(self.change_range, 5))


    def init_scan(self, lastgain):
        self.spc.init_spc()
        self.depostion.init_deposition()
        self.track.init_track()
        self.hop.init_hop()
        self.manipulation.init_manipulation()
        self.send_options.init_sendoptions()
        self.scan_options.init_scanoptions()
        # self.seq_editor.init_seq_editor()

        self.load_xy_gain(lastgain)

    # load X/Y gain status from dsp
    def load_xy_gain(self, lastgain):
        if lastgain[0] == 0:
            self.radioButton_Gain10_XY.setChecked(True)
        elif lastgain[0] == 1:
            self.radioButton_Gain1_XY.setChecked(True)
        elif lastgain[0] == 3:
            self.radioButton_Gain0_1_XY.setChecked(True)

    # if X/Y gain is changed by user, emit signal
    def gain_changed_emit(self, gain, status):
        if status:
            self.gain_changed_signal.emit(0, gain)
            self.gain_changed_signal.emit(1, gain)

    # emit send signal
    def send_emit(self, index):
        self.last_xy[0] = self.spinBox_XinIndication_XY.value()
        self.last_xy[1] = self.spinBox_YinIndication_XY.value()
        self.last_xy[2] = self.spinBox_XoffsetIndication_XY.value()
        self.last_xy[3] = self.spinBox_YoffsetIndication_XY.value()
        if self.idling:
            if index == 0:  # zero button clicked
                xin = 0
                yin = 0
                self.spinBox_XinInput_XY.setValue(0)
                self.spinBox_YinInput_XY.setValue(0)
                self.scrollBar_Xin_XY.setValue(32768)
                self.scrollBar_Yin_XY.setValue(32768)
                xoffset = self.spinBox_XoffsetIndication_XY.value() + 32768
                yoffset = self.spinBox_YoffsetIndication_XY.value() + 32768
                # !!! for test
                self.spinBox_XinIndication_XY.setValue(xin - 32768)
                self.spinBox_YinIndication_XY.setValue(yin - 32768)
            else:           # send button clicked
                xin = self.scrollBar_Xin_XY.value()
                yin = self.scrollBar_Yin_XY.value()
                xoffset = self.scrollBar_Xoffset_XY.value()
                yoffset = self.scrollBar_Yoffset_XY.value()
                # !!! for test
                self.spinBox_XinIndication_XY.setValue(xin - 32768)
                self.spinBox_YinIndication_XY.setValue(yin - 32768)
                self.spinBox_XoffsetIndication_XY.setValue(xoffset - 32768)
                self.spinBox_YoffsetIndication_XY.setValue(yoffset - 32768)
                self.current_xy[2] = xoffset - 32768
                self.current_xy[3] = xoffset - 32768
            # !!! for test
            self.current_xy[0] = xin - 32768
            self.current_xy[1] = yin - 32768
            self.send_signal.emit(index, xin, yin, xoffset, yoffset)
        else:
            self.stop_signal.emit()

    # spinBox -> scrollBar
    def spin2scroll(self, index):
        if index == 0:      # Xin
            bits = self.spinBox_XinInput_XY.value() + 32768
            self.scrollBar_Xin_XY.setValue(bits)
        elif index == 1:    # Yin
            bits = self.spinBox_YinInput_XY.value() + 32768
            self.scrollBar_Yin_XY.setValue(bits)
        elif index == 2:    # X offset
            bits = self.spinBox_XoffsetInput_XY.value() + 32768
            self.scrollBar_Xoffset_XY.setValue(bits)
        elif index == 3:    # Y offset
            bits = self.spinBox_YoffsetInput_XY.value() + 32768
            self.scrollBar_Yoffset_XY.setValue(bits)
        elif index == 4:    # Scan size
            bits = self.spinBox_ScanSize_ScanControl.value()
            self.scrollBar_ScanSize_ScanControl.setValue(bits)
        else:    # Step size
            bits = self.spinBox_StepSize_ScanControl.value()
            self.scrollBar_StepSize_ScanControl.setValue(bits)

    # scrollBar -> spinBox
    def scroll2spin(self, index, bits):
        if index <= 3:
            bits = bits - 32768
            if index == 0:      # Xin
                self.spinBox_XinInput_XY.setValue(bits)
            elif index == 1:    # Yin
                self.spinBox_YinInput_XY.setValue(bits)
            elif index == 2:    # X offset
                self.spinBox_XoffsetInput_XY.setValue(bits)
            else:               # Y offset
                self.spinBox_YoffsetInput_XY.setValue(bits)
        else:
            if index == 4:      # Scan size
                self.spinBox_ScanSize_ScanControl.setValue(bits)
                self.scan_size[0] = int(bits/self.scan_size[1])
            else:               # Step size
                self.spinBox_StepSize_ScanControl.setValue(bits)
                self.scan_size[1] = bits


    # enable/disable X/Y gain based on X/Y in
    def enable_gain(self):
        if self.idling:
            xin = self.spinBox_XinIndication_XY.value()
            yin = self.spinBox_YinIndication_XY.value()
            enable = xin == 0 and yin == 0
            self.radioButton_Gain0_1_XY.setEnabled(enable)
            self.radioButton_Gain1_XY.setEnabled(enable)
            self.radioButton_Gain10_XY.setEnabled(enable)

    # change spinBox/scrollBar range based on:
    # Xin(0), Yin(1), X offset(2), Y offset(3), Scan size(4), Step size(5)
    def change_range(self, index, bits):
        if self.idling:
            scan_size = int((self.scan_size[0] * self.scan_size[1]) / 2)
            if index <= 1:
                if index == 0:      # X in
                    delta = bits - self.last_xy[0]
                else:               # Y in
                    delta = bits - self.last_xy[1]
                if delta != 0 and abs(delta) > scan_size:
                    self.change_xyoffset_range(index, delta)
            elif index <= 3:
                if index == 2:      # X offset
                    delta1 = bits - self.last_xy[2]
                    delta2 = 0
                    if delta1 != 0:
                        self.change_xyin_range(index, delta1)
                else:               # Y offset
                    delta1 = 0
                    delta2 = bits - self.last_xy[3]
                    if delta2 != 0:
                        self.change_xyin_range(index, delta2)
                abs_delta = max(abs(delta1), abs(delta2))
                self.change_scansize_range(abs_delta)   # step size
            elif index == 4:        # Scan size
                self.change_xyoffset_range(index, scan_size)
            else:                   # Step size
                pass

    # change X/Y offset range
    def change_xyoffset_range(self, index, delta):

        if index == 0:  # X in
            min = self.spinBox_XoffsetInput_XY.minimum()
            max = self.spinBox_XoffsetInput_XY.maximum()
            new_min = (delta<0)*(min+abs(delta)) + (delta>0)*min
            new_max = (delta>0)*(max-abs(delta)) + (delta<0)*max
            self.spinBox_XoffsetInput_XY.setMinimum(new_min)
            self.spinBox_XoffsetInput_XY.setMaximum(new_max)
            # self.scrollBar_Xoffset_XY.setMaximum(new_max+32768)
            # self.scrollBar_Xoffset_XY.setMinimum(new_min+32768)

        elif index == 1: # Y in
            min = self.spinBox_YoffsetInput_XY.minimum()
            max = self.spinBox_YoffsetInput_XY.maximum()
            new_min = (delta<0)*(min+abs(delta)) + (delta>0)*min
            new_max = (delta>0)*(max-abs(delta)) + (delta<0)*max
            self.spinBox_YoffsetInput_XY.setMinimum(new_min)
            self.spinBox_YoffsetInput_XY.setMaximum(new_max)
            # self.scrollBar_Yoffset_XY.setMaximum(new_max+32768)
            # self.scrollBar_Yoffset_XY.setMinimum(new_min+32768)

        elif index == 4:    # Scan size
            min = -3276800
            max = 3276700
            is_x_inside = (self.current_xy[2] + delta) >= min and (self.current_xy[2] + delta) <= max
            is_y_inside = (self.current_xy[3] + delta) >= min and (self.current_xy[3] + delta) <= max
            if not (is_x_inside and is_y_inside):
                new_min = min + delta
                new_max = max - delta
                self.spinBox_XoffsetInput.setMinimum(new_min)
                self.spinBox_XoffsetInput.setMaximum(new_max)
                # self.scrollBar_Xoffset.setMaximum(new_max + 32768)
                # self.scrollBar_Xoffset.setMinimum(new_min + 32768)
                self.spinBox_YoffsetInput.setMinimum(new_min)
                self.spinBox_YoffsetInput.setMaximum(new_max)
                # self.scrollBar_Yoffset.setMaximum(new_max + 32768)
                # self.scrollBar_Yoffset.setMinimum(new_min + 32768)

        else: # Step size
            pass

    # change X/Y in range
    def change_xyin_range(self, index, delta):

        if index == 2:     # X offset
            min = self.spinBox_XinInput_XY.minimum()
            max = self.spinBox_XinInput_XY.maximum()
            new_min = (delta<0)*(min+abs(delta)) + (delta>0)*min
            new_max = (delta>0)*(max-abs(delta)) + (delta<0)*max
            self.spinBox_XinInput_XY.setMinimum(new_min)
            self.spinBox_XinInput_XY.setMaximum(new_max)
            # self.scrollBar_Xin_XY.setMaximum(new_max+32768)
            # self.scrollBar_Xin_XY.setMinimum(new_min+32768)

        elif index == 3:   # Y offset
            min = self.spinBox_YinInput_XY.minimum()
            max = self.spinBox_YinInput_XY.maximum()
            new_min = (delta<0)*(min+abs(delta)) + (delta>0)*min
            new_max = (delta>0)*(max-abs(delta)) + (delta<0)*max
            self.spinBox_YinInput_XY.setMinimum(new_min)
            self.spinBox_YinInput_XY.setMaximum(new_max)
            # self.scrollBar_Yin_XY.setMaximum(new_max+32768)
            # self.scrollBar_Yin_XY.setMinimum(new_min+32768)


    # change Scan size and Step size range
    def change_scansize_range(self, abs_delta):
        max = self.spinBox_ScanSize_ScanControl.maximum()
        new_max = max - 2 * abs_delta
        self.spinBox_ScanSize_ScanControl.setMaximum(new_max)
        # self.scrollBar_ScanSize_ScanControl.setMaximum(new_max)
        # !!!
        # self.spinBox_StepSize_ScanControl.setMaximum(int(new_max/self.scan_size[0]))

    # open Scan Options window
    def open_scan_options(self):
        # !!! init scan options
        self.scan_options.show()

    # open Send Options window
    def open_send_options(self):
        # !!! init send options
        self.send_options.show()

    # open Sequence Editor window
    def open_seq_editor(self):
        # !!! init sequence editor
        self.seq_editor.show()

    # Emit close signal
    def closeEvent(self, event):
        if self.mode == 0:
            # !!! pop window to double check
            self.close_signal.emit()
            event.accept()
        else:
            # !!! pop message close other windows
            event.ignore()
        
    def enable_serial(self, enable):
        pass

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myScan()
    window.show()
    sys.exit(app.exec_())

'''
       -> signal - slot connection <-
       
self.scan.gain_changed_signal.connect(self.dsp.gain)
self.scan.send_signal.connect(self.send_slot)
self.scan.stop_signal.connect(self.stop_slot)
self.dsp.rampDiag_signal.connect(self.xy_indication_slot)

'''
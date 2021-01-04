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
from PyQt5.QtGui import QPixmap, QPen
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
from images import myImages
import pyqtgraph as pg
import numpy as np
from symbols import mySymbols
from SequenceList import mySequenceList
import functools as ft
import cv2 as cv

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
        self.seq_list = mySequenceList()

        #
        self.mode = 0       # Scan mode: Scan(0), Spectroscopy(1), Deposition(2)
                            # Track(3), Hop(4), Manipulation(5)
        self.idling = True

        self.last_xy = [0]*4     # Xin(0), Yin(1), X offset(2), Y offset(3)
        self.current_xy = [0]*4  # Xin(0), Yin(1), X offset(2), Y offset(3)
        self.scan_size = [1]*2   # Scan size(0), Step size(1)
        self.imagine_gain = 100  # X/Y gain imaginary value
        # !!! Bias Control variable
        self.bias_dac = 0        # Bias DAC selection
        self.bias_range = 10     # Bias range

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

        # radioButton | X/Y gain
        self.XY_gain_group = QButtonGroup()
        self.XY_gain_group.addButton(self.radioButton_Gain0_1_XY, 3)
        self.XY_gain_group.addButton(self.radioButton_Gain1_XY, 1)
        self.XY_gain_group.addButton(self.radioButton_Gain10_XY, 0)
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
        self.spinBox_XinIndication_XY.valueChanged.connect(ft.partial(self.change_range, 0))
        self.spinBox_YinIndication_XY.valueChanged.connect(ft.partial(self.change_range, 1))
        self.spinBox_XoffsetIndication_XY.valueChanged.connect(ft.partial(self.change_range, 2))
        self.spinBox_YoffsetIndication_XY.valueChanged.connect(ft.partial(self.change_range, 3))

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
        self.scrollBar_ScanSize_ScanControl.valueChanged.connect(ft.partial(self.change_range, 4))
        self.scrollBar_StepSize_ScanControl.valueChanged.connect(ft.partial(self.change_range, 5))

        # radioButton | view control
        self.pallet_group = QButtonGroup()
        self.pallet_group.addButton(self.radioButton_Color_Scan, 0)
        self.pallet_group.addButton(self.radioButton_Gray_Scan, 1)
        self.pallet_group.addButton(self.radioButton_Reverse_Scan, 2)
        self.pallet_group.buttonToggled[int, bool].connect(self.pallet_changed)

        # pushButton | tool bar
        self.tool_bar_group = QButtonGroup()
        self.tool_bar_group.addButton(self.pushButton_ZoomIn_ViewControl)
        self.tool_bar_group.addButton(self.pushButton_ZoomOut_ViewControl)
        self.tool_bar_group.addButton(self.pushButton_Move_ViewControl)
        self.pushButton_ZoomIn_ViewControl.clicked.connect(self.zoom_to_box)

        # graphicsView | Scan main view

        # viewBox | setup
        self.view_box = self.graphicsView_Scan.addViewBox(enableMenu=False)
        self.view_box.setRange(QRectF(-3276800,-3276800,6553600,6553600),padding=0)
        self.view_box.setLimits(xMin=-3276800, xMax=3276800, yMin=-3276800, yMax=3276800, \
                                minXRange=10, maxXRange=6553600, minYRange=10, maxYRange=6553600)
        self.view_box.setAspectLocked(True)

        # ROI | define pens
        blue_pen = pg.mkPen((70, 200, 255, 255), width=1)
        green_pen = pg.mkPen((150, 220, 0, 255), width=1)
        baby_blue_pen = pg.mkPen((130, 220, 255, 255), width=1, dash=[2,4,2,4])
        baby_green_pen = pg.mkPen((210, 255, 120, 255), width=1, dash=[2,2,2,2])

        # ROI | scan area
        self.scan_area = pg.RectROI([2000, 2000], [2000000, 2000000], pen=blue_pen, centered=True, \
                              maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600), scaleSnap=True)
        self.scan_area.aspectLocked = True
        self.scan_area.setZValue(10)
        self.view_box.addItem(self.scan_area)

        # ROI | target area
        self.target_area = pg.RectROI([2000, 2000], [2000000, 2000000], pen=baby_blue_pen, centered=True, \
                              maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600), scaleSnap=True)
        self.target_area.setZValue(10)
        self.target_area.aspectLocked = True
        self.view_box.addItem(self.target_area)
        self.target_area.removeHandle(0)

        # ROI | tip position
        cross_pos = [int(self.scan_area.pos()[0] + self.scan_area.size()[0]/2), \
                     int(self.scan_area.pos()[1] + self.scan_area.size()[0]/2)]
        cross_size = [int(self.scan_area.size()[0]/20), int(self.scan_area.size()[1]/20)]
        self.tip_position = pg.CrosshairROI(pos=cross_pos, size=cross_size, pen=baby_green_pen, \
                                            resizable=False, rotatable=False)
        self.tip_position.setZValue(10)
        self.view_box.addItem(self.tip_position)
        self.tip_position.removeHandle(0)

        # ROI | target position
        self.target_position = pg.CrosshairROI(pos=cross_pos, size=cross_size, pen=green_pen, \
                                            resizable=False, rotatable=False)
        self.target_position.setZValue(10)
        self.view_box.addItem(self.target_position)
        self.target_position.removeHandle(0)

        # get pseudo image
        # image = QPixmap("../data/scan_example.png").toImage()
        # channels_count = 4
        # s = image.bits().asstring(image.width() * image.height() * channels_count)
        # self.arr = np.frombuffer(s, dtype = np.uint8).reshape((image.width(), image.height(), channels_count))
        self.arr = cv.imread("../data/scan_example.png")
        # imageItem setup
        self.image = pg.ImageItem()
        self.view_box.addItem(self.image)
        self.image.setImage(self.arr)


        # !!! only for check range
        self.enable_serial((True))

    def init_scan(self, succeed, lastgain, lastdac):
        # Set up view in case of successfully finding DSP
        self.enable_serial(succeed)

        # self.spc.init_spc()
        # self.depostion.init_deposition()
        # self.track.init_track()
        # self.hop.init_hop()
        # self.manipulation.init_manipulation()
        # self.send_options.init_sendoptions()
        # self.scan_options.init_scanoptions()
        # self.seq_editor.init_seq_list()

        self.load_xy_gain(lastgain)

    # load X/Y gain status from dsp
    def load_xy_gain(self, lastgain):
        if lastgain[0] == 0:
            self.radioButton_Gain10_XY.setChecked(True)
        elif lastgain[0] == 1:
            self.radioButton_Gain1_XY.setChecked(True)
        elif lastgain[0] == 3:
            self.radioButton_Gain0_1_XY.setChecked(True)
        self.imagine_gain = (lastgain[0] == 0) * 100 + (lastgain[0] == 1) * 10 + (lastgain[0] == 3) * 1

    # if X/Y gain is changed by user, emit signal
    def gain_changed_emit(self, gain, status):
        if status:
            self.gain_changed_signal.emit(0, gain)
            self.gain_changed_signal.emit(1, gain)
        self.imagine_gain = (gain == 0) * 100 + (gain == 1) * 10 + (gain == 3) * 1
        self.change_xyin_range(6, gain)

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
                self.current_xy[0] = xin
                self.current_xy[1] = yin
                self.spinBox_XinIndication_XY.setValue(xin)
                self.spinBox_YinIndication_XY.setValue(yin)
            else:           # send button clicked
                xin = self.spinBox_XinInput_XY.value()
                yin = self.spinBox_YinInput_XY.value()
                xoffset = self.spinBox_XoffsetInput_XY.value()
                yoffset = self.spinBox_YoffsetInput_XY.value()
                # !!! for test
                self.current_xy[0] = xin
                self.current_xy[1] = yin
                self.current_xy[2] = xoffset
                self.current_xy[3] = yoffset
                self.spinBox_XinIndication_XY.setValue(xin)
                self.spinBox_YinIndication_XY.setValue(yin)
                self.spinBox_XoffsetIndication_XY.setValue(xoffset)
                self.spinBox_YoffsetIndication_XY.setValue(yoffset)
            self.send_signal.emit(index, xin+32768, yin+32768, xoffset+32768, yoffset+32768)
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
            self.scan_size[0] = bits
            self.scrollBar_ScanSize_ScanControl.setValue(bits)
        else:               # Step size
            bits = self.spinBox_StepSize_ScanControl.value()
            self.scan_size[1] = bits
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
                self.scan_size[0] = bits
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
        # print("--------------------------")
        if self.idling:
            scan_size = int((self.scan_size[0] * self.scan_size[1]) / 2)
            if index <= 1:          # Xin and Yin
                if bits != self.last_xy[index] and abs(bits) > scan_size:
                    self.change_xyoffset_range(index, bits)
            elif index <= 3:        # X offset and Y offset
                if bits != self.last_xy[index]:
                    self.change_xyin_range(index, bits)
                self.change_scansize_range(index)   # step size
            elif index == 4:        # Step num
                self.change_xyoffset_range(index, scan_size)
                self.change_scansize_range(index)
            else:                   # Step size
                self.change_xyoffset_range(index, scan_size)
                self.change_scansize_range(index)

    # change X/Y offset range
    def change_xyoffset_range(self, index, bits):
        if index == 0 or index == 1:                # X/Y in
            new_max = int(32767 - self.current_xy[index] * self.imagine_gain / 100)
            new_min = int(-32768 - self.current_xy[index] * self.imagine_gain / 100)
            new_max = (bits > 0) * new_max + (bits <= 0) * 32767
            new_min = (bits < 0) * new_min + (bits >= 0) * (-32768)
            if index == 0:                          # X in
                self.spinBox_XoffsetInput_XY.setMinimum(new_min)
                self.spinBox_XoffsetInput_XY.setMaximum(new_max)
            elif index == 1:                        # Y in
                self.spinBox_YoffsetInput_XY.setMinimum(new_min)
                self.spinBox_YoffsetInput_XY.setMaximum(new_max)
        elif index == 4 or index == 5:              # Step num and Step size
            old_min_x = self.spinBox_XoffsetInput_XY.minimum()
            old_max_x = self.spinBox_XoffsetInput_XY.maximum()
            old_min_y = self.spinBox_YoffsetInput_XY.minimum()
            old_max_y = self.spinBox_YoffsetInput_XY.maximum()
            old_min = max(old_min_x, old_min_y)
            old_max = min(old_max_x, old_max_y)
            new_max = (old_max > (32767 - bits)) * (32767 - bits) + (old_max <= (32767 - bits)) * old_max
            new_min = (old_min < (-32768 + bits)) * (-32768 + bits) + (old_min >= (-32768 + bits)) * old_min
            self.spinBox_XoffsetInput_XY.setMinimum(new_min)
            self.spinBox_XoffsetInput_XY.setMaximum(new_max)
            self.spinBox_YoffsetInput_XY.setMinimum(new_min)
            self.spinBox_YoffsetInput_XY.setMaximum(new_max)
        self.scrollBar_Xoffset_XY.setMaximum(new_max + 32768)
        self.scrollBar_Xoffset_XY.setMinimum(new_min + 32768)
        self.scrollBar_Yoffset_XY.setMaximum(new_max + 32768)
        self.scrollBar_Yoffset_XY.setMinimum(new_min + 32768)

    # change X/Y in range
    def change_xyin_range(self, index, bits):
        if index == 2 or index ==3:
            new_max = int((32767 - self.current_xy[index]) / self.imagine_gain)
            new_min = int((-32768 - self.current_xy[index]) / self.imagine_gain)
            new_max = (bits > 0) * new_max + (bits <= 0) * 32767
            new_min = (bits < 0) * new_min + (bits >= 0) * (-32768)
            if index == 2:                  # X offset
                self.spinBox_XinInput_XY.setMaximum(new_max)
                self.spinBox_XinInput_XY.setMinimum(new_min)
                self.scrollBar_Xin_XY.setMaximum(new_max + 32768)
                self.scrollBar_Xin_XY.setMinimum(new_min + 32768)
            elif index ==3:                 # Y offset
                self.spinBox_YinInput_XY.setMaximum(new_max)
                self.spinBox_YinInput_XY.setMinimum(new_min)
                self.scrollBar_Yin_XY.setMaximum(new_max + 32768)
                self.scrollBar_Yin_XY.setMinimum(new_min + 32768)
        elif index == 6 :                   # X/Y gain
            new_max_x = int((32767 - self.current_xy[2]) / self.imagine_gain)*(self.current_xy[2]>0) + (self.current_xy[2]<=0)*32767
            new_min_x = int((-32768 - self.current_xy[2]) / self.imagine_gain)*(self.current_xy[2]<0) + (self.current_xy[2]>=0)*(-32768)
            new_max_y = int((32767 - self.current_xy[3]) / self.imagine_gain)*(self.current_xy[3]>0) + (self.current_xy[3]<=0)*32767
            new_min_y = int((-32768 - self.current_xy[3]) / self.imagine_gain)*(self.current_xy[3]<0) + (self.current_xy[3]>=0)*(-32768)
            self.spinBox_XinInput_XY.setMaximum(new_max_x)
            self.spinBox_XinInput_XY.setMinimum(new_min_x)
            self.spinBox_YinInput_XY.setMaximum(new_max_y)
            self.spinBox_YinInput_XY.setMinimum(new_min_y)
            self.scrollBar_Xin_XY.setMaximum(new_max_x + 32768)
            self.scrollBar_Xin_XY.setMinimum(new_min_x + 32768)
            self.scrollBar_Yin_XY.setMaximum(new_max_y + 32768)
            self.scrollBar_Yin_XY.setMinimum(new_min_y + 32768)
        else:
            pass

    # change Scan size and Step size range
    def change_scansize_range(self, index):
        abs_bits = max(abs(self.current_xy[2]), abs(self.current_xy[3]))
        scan_size = 65535 - abs_bits * 2
        if index < 4:       # X/Y offset changed
            self.spinBox_ScanSize_ScanControl.setMaximum(int(scan_size/self.scan_size[1]))
            self.spinBox_StepSize_ScanControl.setMaximum(int(scan_size/self.scan_size[0]))
            self.scrollBar_ScanSize_ScanControl.setMaximum(int(scan_size/self.scan_size[1]))
            self.scrollBar_StepSize_ScanControl.setMaximum(int(scan_size/self.scan_size[0]))
        elif index == 4:    # Step num changed
            self.spinBox_StepSize_ScanControl.setMaximum(int(scan_size/self.scan_size[0]))
            self.scrollBar_StepSize_ScanControl.setMaximum(int(scan_size/self.scan_size[0]))
        else:               # Step size changed
            self.spinBox_ScanSize_ScanControl.setMaximum(int(scan_size/self.scan_size[1]))
            self.scrollBar_ScanSize_ScanControl.setMaximum(int(scan_size/self.scan_size[1]))

    # change image pallet
    def pallet_changed(self, index, status):
        self.myimg = myImages()
        if status:
            if index == 0:  # color
                self.color_img = self.myimg.gey2color("../data/scan_example.png")
                self.image.setImage(self.color_img)
            elif index == 1:    # gray
                self.grey_img = self.myimg.color2gray(self.arr)
                self.image.setImage(self.grey_img)
            else:       # reverse
                pass

    # open Scan Options window
    def open_scan_options(self):
        # !!! init scan options
        self.scan_options.show()

    # open Send Options window
    def open_send_options(self):
        # !!! init send options
        self.send_options.show()

    # Emit close signal
    def closeEvent(self, event):
        if self.mode == 0:
            # !!! pop window to double check
            self.close_signal.emit()
            event.accept()
        else:
            # !!! pop message close other windows
            event.ignore()

    # zoom in button slot
    def zoom_to_box(self):
        if self.pushButton_ZoomIn_ViewControl.isChecked():
            self.view_box.setMouseMode(self.view_box.RectMode)
        else:
            self.view_box.setMouseMode(self.view_box.PanMode)
        
    def enable_serial(self, enable):
        self.pushButton_Zero_XY.setEnabled(enable)
        self.pushButton_Send_XY.setEnabled(enable)
        self.pushButton_Start_Scan.setEnabled(enable)
        self.radioButton_Gain0_1_XY.setEnabled(enable)
        self.radioButton_Gain1_XY.setEnabled(enable)
        self.radioButton_Gain10_XY.setEnabled(enable)


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
self.scan.pushButton_SeqList_ScanControl.clicked.connect(self.open_seq_list)

'''
# print("--------------------------")
# print("imagine_gain:", self.imagine_gain)
# print("X_offset range: \t(",self.spinBox_XoffsetInput_XY.minimum(),",",self.spinBox_XoffsetInput_XY.maximum(),")")
# print("Y_offset range: \t(",self.spinBox_YoffsetInput_XY.minimum(),",",self.spinBox_YoffsetInput_XY.maximum(),")")
# print("X_in range: \t(",self.spinBox_XinInput_XY.minimum(),",",self.spinBox_XinInput_XY.maximum(),")")
# print("Y_in range: \t(",self.spinBox_YinInput_XY.minimum(),",",self.spinBox_YinInput_XY.maximum(),")")
# print("Step_num range: \t(",self.spinBox_ScanSize_ScanControl.minimum(),",",self.spinBox_ScanSize_ScanControl.maximum(),")")
# print("Step_size range: \t(",self.spinBox_StepSize_ScanControl.minimum(),",",self.spinBox_StepSize_ScanControl.maximum(),")")
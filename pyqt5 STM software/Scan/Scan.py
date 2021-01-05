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
from DataStruct import ScanData
from DigitalSignalProcessor import myDSP

class myScan(QWidget, Ui_Scan):
    close_signal = pyqtSignal()
    seq_list_signal = pyqtSignal(str)
    stop_signal = pyqtSignal()
    gain_changed_signal = pyqtSignal(int, int)
    send_signal = pyqtSignal(int, int, int, int, int)

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.spc = mySpc()
        self.dep = myDeposition()
        self.track = myTrack()
        # self.hop = myHop()
        # self.manip = myManipulation()
        self.scan_options = myScanOptions()
        self.send_options = mySendOptions()
        self.seq_list = mySequenceList()

        # Variables
        # Flags
        self.mode = 0       # Scan mode: Scan(0), Spectroscopy(1), Deposition(2)
        self.stop = True
        self.idling = True
        self.saved = True
        # XY and image variables
        self.last_xy = [0]*4     # Xin(0), Yin(1), X offset(2), Y offset(3)
        self.current_xy = [0]*4  # Xin(0), Yin(1), X offset(2), Y offset(3)
        self.scan_size = [1]*2   # Scan size(0), Step size(1)
        self.imagine_gain = 100  # X/Y gain imaginary value
        # Bias variables
        self.bias_dac = False       # Bias DAC selection
        self.bias_ran = 9           # Bias range
        # Data
        self.data = ScanData()
        # Sequence lists and selected sequence
        self.scan_seq_list = []
        self.dep_seq_list = []
        self.spc_seq_list = []
        self.scan_seq_selected = -1
        self.dep_seq_selected = -1
        self.spc_seq_selected = -1
        self.init_UI()
        
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
        # self.close_signal.clicked(self.manip.close_signal)

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

        # checkBox | View Control
        self.checkBox_Illuminated_Scan.stateChanged.connect(ft.partial(self.image_process,0))
        self.checkBox_PlaneFit_Scan.stateChanged.connect(ft.partial(self.image_process, 1))

        # pushButton | tool bar
        self.tool_bar_group = QButtonGroup()
        self.tool_bar_group.addButton(self.pushButton_Move_ViewControl, 0)
        self.tool_bar_group.addButton(self.pushButton_ZoomIn_ViewControl, 1)
        self.tool_bar_group.setExclusive(False)
        self.tool_bar_group.buttonToggled[int, bool].connect(self.view_mode_selected)
        self.pushButton_Full_ViewControl.clicked.connect(self.full_view)
        self.pushButton_Detail_ViewControl.clicked.connect(self.detail_view)



        # graphicsView | Scan main view

        # viewBox | setup
        self.view_box = self.graphicsView_Scan.addViewBox(enableMenu=False, enableMouse=False)
        self.view_box.setRange(QRectF(-3276800, -3276800, 6553600, 6553600), padding=0)
        self.view_box.setLimits(xMin=-3276800, xMax=3276800, yMin=-3276800, yMax=3276800, \
                                minXRange=10, maxXRange=6553600, minYRange=10, maxYRange=6553600)
        self.view_box.setAspectLocked(True)
        self.view_box.setCursor(Qt.CrossCursor)

        # ROI | define pens
        blue_pen = pg.mkPen((70, 200, 255, 255), width=1)
        green_pen = pg.mkPen((150, 220, 0, 255), width=1)
        baby_blue_pen = pg.mkPen((130, 220, 255, 255), width=1, dash=[2,4,2,4])
        baby_green_pen = pg.mkPen((210, 255, 120, 255), width=1, dash=[2,2,2,2])

        # ROI | scan area
        self.scan_area = pg.RectROI([2000, 2000], [2000000, 2000000], pen=blue_pen, centered=True, \
                                    movable=False, resizable=False, rotatable=False, \
                                    maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600), scaleSnap=True)
        # self.scan_area.sigHoverEvent.connect(self.mouseDragEvent)
        self.scan_area.aspectLocked = True
        self.scan_area.setZValue(10)
        self.view_box.addItem(self.scan_area)
        self.scan_area.removeHandle(0)

        # ROI | target area
        self.target_area = pg.RectROI([2000, 2000], [2000000, 2000000], pen=baby_blue_pen, centered=True, \
                                      movable=False, resizable=False, rotatable=False, \
                                      maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600), scaleSnap=True)
        self.target_area.setZValue(10)
        self.target_area.aspectLocked = True
        self.view_box.addItem(self.target_area)
        self.target_area.removeHandle(0)

        # ROI | tip position
        cross_pos = [int(self.scan_area.pos()[0] + self.scan_area.size()[0]/2), \
                     int(self.scan_area.pos()[1] + self.scan_area.size()[0]/2)]
        cross_size = [int(self.scan_area.size()[0]/20), int(self.scan_area.size()[1]/20)]
        self.tip_position = pg.CrosshairROI(pos=cross_pos, size=cross_size, pen=green_pen, movable=False,\
                                            resizable=False, rotatable=False)
        self.tip_position.setZValue(10)
        self.view_box.addItem(self.tip_position)
        self.tip_position.removeHandle(0)

        # ROI | target position
        self.target_point = pg.CrosshairROI(pos=cross_pos, size=cross_size, pen=baby_green_pen, movable=False,\
                                            resizable=False, rotatable=False)
        self.target_point.setZValue(10)
        self.view_box.addItem(self.target_point)
        self.target_point.removeHandle(0)

        # get pseudo image

        self.arr = cv.imread("..\data\scan_example_gray.jpg")
        # imageItem setup
        self.image = pg.ImageItem()
        self.view_box.addItem(self.image)
        self.image.setImage(self.arr)


        # !!! only for check range
        # self.enable_serial((True))

    def init_scan(self, dsp, bias_dac, preamp_gain):
        succeed = dsp.succeed
        XYgain = dsp.lastgain
        bias_ran = dsp.dacrange[13]
        
        # Set up view in case of successfully finding DSP
        self.pushButton_Start_Scan.setEnabled(succeed)
        self.radioButton_Gain10_XY.setEnabled(succeed)
        self.radioButton_Gain0_1_XY.setEnabled(succeed)
        self.pushButton_Send_XY.setEnabled(succeed)
        self.pushButton_Zero_XY.setEnabled(succeed)
        self.pushButton_Start_Scan.setEnabled(succeed)
        self.pushButton_SaveAll_Scan.setEnabled(not self.data.data)
        self.pushButton_Info_Scan.setEnabled(not self.data.data)
        
         
        self.dep.init_deposition(succeed, bias_dac, bias_ran, self.dep_seq_list, self.dep_seq_selected)
        
        # self.spc.init_spc()
        # self.track.init_track()
        # self.hop.init_hop()
        # self.manip.init_manipulation()
        # self.send_options.init_sendoptions()
        # self.scan_options.init_scanoptions()

        self.load_xy_gain(XYgain)

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

    # pallet radioButton slot | Gray, Color and Reverse
    def pallet_changed(self, index, status):
        self.myimg = myImages()
        if status:
            if index == 0:  # color
                self.color_img = self.myimg.gray2color("../data/scan_example.png")
                self.image.setImage(self.color_img)
            elif index == 1:    # gray
                self.gray_img = self.myimg.color2gray(self.arr)
                self.image.setImage(self.gray_img)
            else:       # reverse
                # self.reverse_gray_img = self.myimg.gray2reverse("../data/scan_example_gray.jpg")
                # self.reverse_color_img = self.myimg.color2reverse("../data/scan_example.png")
                self.reverse_gray_img = cv.imread('..\data\scan_example_reverse.png')
                self.image.setImage(self.reverse_gray_img)

    # pallet checkBox slot | Illuminated and Plane fit
    def image_process(self, index, status):
        self.myimg = myImages()
        if status:
            if index == 0:
                # self.illuminated_img = self.myimg.illuminated("..\data\scan_example_gray.jpg")
                self.illuminated_img = cv.imread('..\data\scan_example_illuminated.png')
                self.image.setImage(self.illuminated_img)

            elif index == 1:
                # self.planefit_img = self.myimg.plane_fit("../data/scan_example.png")
                self.planefit_img = cv.imread('..\data\scan_example_planefit.png')
                self.image.setImage(self.planefit_img)
        else:
            self.gray_img = cv.imread("../data/scan_example_gray.jpg")
            self.image.setImage(self.gray_img)

    # tool bar button slot | Move mode and Zoom mode
    def view_mode_selected(self, index, status):
        '''
        Mouse interaction mode must be one of [Default mode, Move mode, Zoom mode].
        Only one mode is allowed at any given time.
        User can lick another button to change mode, or clicked again to cancel current selection.
        No button checked means that current mode is Default mode.

        **Mode details**.
        ============= ============================================================
        Default mode   left click to decide target area center position;
                       right click to decide target point position
        Move mode      left drag to pan the view (always inside canvas);
                       wheel to scale by mouse position
        Zoom mode      left drag to zoom to the box;
                       wheel to scale by center of canvas
        ============= ============================================================
        '''
        # print("Button:", index, "\tstatus:", status)
        if status:          # button checked
            self.view_box.setMouseEnabled(x=True, y=True)
            self.target_area.hide()
            self.target_point.hide()
            if index == 0:  # move mode button checked
                if self.pushButton_ZoomIn_ViewControl.isChecked():
                    self.pushButton_ZoomIn_ViewControl.setChecked(False)
                else:
                    self.pushButton_Move_ViewControl.setDown(True)
                self.view_box.setCursor(Qt.OpenHandCursor)
                self.scan_area.translatable = True
                self.scan_area.resizable = True
                self.tip_position.translatable = True
                self.tip_position.resizable = True
                self.view_box.setMouseMode(self.view_box.PanMode)
            else:           # zoom mode button checked
                if self.pushButton_Move_ViewControl.isChecked():
                    self.pushButton_Move_ViewControl.setChecked(False)
                else:
                    self.pushButton_ZoomIn_ViewControl.setDown(True)
                self.view_box.setCursor(Qt.PointingHandCursor)
                self.scan_area.translatable = False
                self.tip_position.translatable = False
                self.view_box.setMouseMode(self.view_box.RectMode)
        else:               # button unchecked
            self.pushButton_Move_ViewControl.setDown(False)
            self.pushButton_ZoomIn_ViewControl.setDown(False)
            self.view_box.setCursor(Qt.CrossCursor)
            if self.tool_bar_group.checkedId() == -1:
                self.scan_area.translatable = False
                self.tip_position.translatable = False
                self.target_area.translatable = False
                self.target_point.translatable = False
                self.view_box.setMouseEnabled(x=False, y=False)
                self.target_area.setPos(self.scan_area.pos())
                self.target_area.setSize(self.scan_area.size())
                self.target_point.setPos(self.tip_position.pos())
                self.target_area.show()
                self.target_point.show()

    #
    # def mouseDragEvent(self, ev):
    #     self.scan_area.mouseDragHandler.mouseDragEvent(ev)
    #     if ev.button() & QtCore.Qt.LeftButton:
    #         if self.pushButton_Move_ViewControl.isChecked() and self.scan_area.mouseHovering == True:
    #             self.setCursor(Qt.closedHandCursor)

    # tool bar button slot | Full view
    def full_view(self):
        '''Zoom out to full canvas'''
        self.view_box.setRange(QRectF(-3276800,-3276800,6553600,6553600), padding=0)

    # tool bar button slot | Detail view
    def detail_view(self):
        '''Zoom in to scan area'''
        self.view_box.setRange(QRectF(self.scan_area.pos()[0], self.scan_area.pos()[1], \
                                      self.scan_area.size()[0], self.scan_area.size()[1]), padding=0)
            
    # Bias range change slot
    def bias_ran_change(self, ran):
        self.dep.bias_ran_change(ran)
        self.spc.bias_ran_change(ran)

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
            msg = QMessageBox.information(None, "Scan", "Really want to exit scan?", QMessageBox.Yes | QMessageBox.No)
            if msg == QMessageBox.Yes:
                self.close_signal.emit()
                event.accept()
            else:
                event.ignore()
        else:
            QMessageBox.warning(None, "Scan", "Process ongoing!!", QMessageBox.Ok)
            event.ignore()
    
    def enable_serial(self, enable):
        self.XY_offset.setEnabled(enable)
        self.XY_in.setEnabled(enable)
        self.options.setEnabled(enable)
        self.ScanControl.setEnabled(enable)
        self.pushButton_Zero_XY.setEnabled(enable)
        self.pushButton_Send_XY.setEnabled(enable)
        self.pushButton_Start_Scan.setEnabled(enable)
        self.radioButton_Gain0_1_XY.setEnabled(enable)
        self.radioButton_Gain1_XY.setEnabled(enable)
        self.radioButton_Gain10_XY.setEnabled(enable)
        self.pushButton_Start_Scan.setEnabled(enable)
        self.pushButton_SaveAll_Scan.setEnabled(enable and (not self.data.data))
        self.pushButton_Info_Scan.setEnabled(enable and (not self.data.data))
        self.pushButton_Load_Scan.setEnabled(enable)

        self.dep.enable_serial(enable)
        # !!! need other modules's enable serial function


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myScan()
    # print(window.data.seq_name)
    window.init_scan(myDSP(), False, 9)
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
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

from sequence import mySequence
from DataStruct import ScanData
from images import myImages
from customROI import *

import pyqtgraph as pg
import numpy as np
import functools as ft
import cv2 as cv
import scipy.io
from datetime import datetime

class myScan_(QWidget, Ui_Scan):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_property()
        self.init_UI()
        
    def init_property(self):
        # Modeules
        self.spc = mySpc()                      # Spectroscopy window
        self.dep = myDeposition()               # Deposition window
        self.track = myTrack()                  # Track window
        self.scan_options = myScanOptions()     # Scan option window
        self.send_options = mySendOptions()     # Send option window
        self.point_editor = myPointEditor()     # Point editor window
        self.seq_list = mySequenceList()        # Sequence list window
        self.info = myScanInfo()                # Scan data info window
        self.lockin = myLockIn()                # Lock in parameters window
        self.dlg = QFileDialog()                # Scan file dialog window
        self.data = ScanData()                  # Scan sata
        
        # For automatically assign file name
        self.today = datetime.now().strftime("%m%d%y")
        self.file_idex = 0
        
        # Set up file dialog window
        self.dlg.setNameFilter('STM Files (*.stm)')

        # System status
        self.mode = 0                   # Scan mode: Scan(0), Spectroscopy(1), Deposition(2)
        self.stop = True                # Stop flag
        self.idling = True              # Idling status
        self.saved = True               # Saved status
        self.bias_dac = False           # Bias DAC selection
        self.bias_ran = 9               # Bias range
        self.point_mode = False
        
        # XY and image variables (unit is in imagine bit)
        self.last_xy = [0]*4            # Xin(0), Yin(1), X offset(2), Y offset(3) --> values sent last time
        self.current_xy = [0]*4         # Xin(0), Yin(1), X offset(2), Y offset(3)
        self.scan_size = [128, 2560]   # Scan size(0), Step size(1)
        self.imagine_gain = 10          # X/Y gain imaginary value
        self.tilt = [0.0] * 2           # Tilt X, Tilt Y
        
        # Sequence lists and selected sequence
        constant_current_seq = mySequence([0xd8], [500] , True)                 # Basic constant current sequence
        constant_current_seq.name = 'Basic constant current'
        constant_height_seq = mySequence([0xdc], [500] , True)                  # Basic height current sequence
        constant_height_seq.name = 'Basic height current'
        constant_height_seq.feedback = False
        self.scan_seq_list = [constant_current_seq, constant_height_seq]        # Scan sequence list
        self.dep_seq_list = []                                                  # Depostion sequence list
        self.spc_seq_list = []                                                  # Spectroscopy sequence list
        self.scan_seq_selected = 0                                              # Selected scan sequence
        self.dep_seq_selected = -1                                              # Selected deposition sequence
        self.spc_seq_selected = -1                                              # Selected spectroscopy sequence
        
        # Spectroscopy related
        self.point_list = [[0, 0]]                                              # Point list for mapping [[X position, Y position]]
        self.pattern = [0, 0, 1]                                                # Pattern for matching [X position, Y position, Size]
        
    def init_UI(self):
        # Init ui position and size
        screen = QDesktopWidget().screenGeometry()
        self.resize(981, 549)
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

        ## graphicsView | Scan main view

        # viewBox | setup
        self.graphicsView_Scan.setContentsMargins(-10, -10, -10, -10)
        self.view_box = self.graphicsView_Scan.addViewBox(enableMenu=False, enableMouse=True)
        self.view_box.setRange(QRectF(-3276800, -3276800, 6553600, 6553600), padding=0)
        self.view_box.setLimits(xMin=-3481600, xMax=3481600, yMin=-3481600, yMax=3481600, \
                                minXRange=3, maxXRange=6963200, minYRange=3, maxYRange=6963200)
        self.view_box.setAspectLocked(True)
        self.view_box.setCursor(Qt.CrossCursor)
        self.view_box.setMouseMode(self.view_box.PanMode)
        self.view_box.sigMoveROI.connect(self.default_update)

        # ROI | define pens
        blue_pen = pg.mkPen((70, 200, 255, 255), width=1)
        green_pen = pg.mkPen((150, 220, 0, 255), width=1)
        purple_pen = pg.mkPen('deaaff', width=1)
        yellow_pen = pg.mkPen('ffe6a7', width=1)
        pink_pen = pg.mkPen(('ffc8dd'), width=1)
        baby_blue_pen = pg.mkPen((130, 220, 255, 255), width=1, dash=[2, 4, 2, 4])
        baby_green_pen = pg.mkPen((210, 255, 120, 100), width=1, dash=[2, 2, 2, 2])
        baby_yellow_pen = pg.mkPen('ffe6a7', width=1, dash=[2, 4, 2, 4])
        baby_pink_pen = pg.mkPen('ffc8dd', width=1, dash=[2, 4, 2, 4])
        serial_pen_0 = pg.mkPen('bee9e8', width=1, dash=[2, 2, 2, 2])
        serial_pen_1 = pg.mkPen('70d6ff', width=1)
        serial_pen_2 = pg.mkPen('ff70a6', width=1)
        serial_pen_3 = pg.mkPen('ff9770', width=1)
        serial_pen_4 = pg.mkPen('ffd670', width=1)
        serial_pen_5 = pg.mkPen('e9ff70', width=1)
        hover_pen = pg.mkPen((255, 255, 0, 100), width=1, dash=[2, 4, 2, 4])
        self.serial_pen = [serial_pen_0, serial_pen_1, serial_pen_2, serial_pen_3, serial_pen_4, serial_pen_5]

        # ROI | scan area
        self.scan_area = CrossCenterROI([-150000, -150000], [300000, 300000], pen=green_pen, centered=True, \
                                        movable=False, resizable=False, rotatable=False, \
                                        maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600), scaleSnap=True)
        self.scan_area.aspectLocked = True
        self.scan_area.setZValue(10)
        self.view_box.addItem(self.scan_area)
        self.scan_area.removeHandle(0)
        self.scan_area.addCustomHandle(info={'type': 't', 'pos': [0.5, 0.5], 'pen': blue_pen}, index=3)
        self.scan_area.getHandles()[0].setPen(green_pen)

        # ROI | target area
        self.target_area = CrossCenterROI([-150000, -150000], [300000, 300000], pen=baby_blue_pen, hoverPen=blue_pen, centered=True, \
                                          movable=True, resizable=False, rotatable=False, \
                                          maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600), scaleSnap=True)
        self.target_area.setZValue(10)
        self.target_area.aspectLocked = True
        self.view_box.addItem(self.target_area)
        self.target_area.removeHandle(0)
        # self.target_area.hide()
        self.target_area.addCustomHandle(info={'type': 't', 'pos': [0.5, 0.5], 'pen': baby_blue_pen}, index=3)
        self.target_area.sigRegionChanged.connect(lambda: self.default_update(0))

        # ROI | tip position
        cross_pos = [self.scan_area.pos()[0] + self.scan_area.getHandles()[0].pos()[0], \
                     self.scan_area.pos()[1] + self.scan_area.getHandles()[0].pos()[1]]
        cross_size = [int(self.scan_area.size()[0] / 20), int(self.scan_area.size()[1] / 20)]

        self.tip_position = CrossCenterROI2(cross_pos, cross_size, maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600),\
                                            pen=(255, 255, 255, 0), movable=False)
        self.view_box.addItem(self.tip_position)
        self.tip_position.removeHandle(0)
        self.tip_position.addCustomHandle2(info={'type': 't', 'pos': [0.5, 0.5]}, index=3)
        self.tip_position.movePoint(self.tip_position.getHandles()[0], cross_pos)
        self.tip_position.getHandles()[0].setPen(green_pen)

        # ROI | target position
        self.target_position = CrossCenterROI2(cross_pos, cross_size, maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600),\
                                               pen=(255, 255, 255, 0), hoverPen=pink_pen, movable=True)
        self.view_box.addItem(self.target_position)
        self.target_position.removeHandle(0)
        self.target_position.addCustomHandle2_(info={'type': 't', 'pos': [0.5, 0.5]}, index=3)
        self.target_position.movePoint(self.target_position.getHandles()[0], cross_pos)
        self.target_position.sigRegionChanged.connect(lambda: self.default_update(1))

        # ROI | connect target area and scan area
        self.connect_area = LineSegmentROI(pos=[0, 0], positions=[0, 300000], pen=baby_blue_pen, movable=False)
        self.view_box.addItem(self.connect_area)

        x = self.scan_area.getHandles()[0].pos()[0] + self.scan_area.pos()[0]
        y = self.scan_area.getHandles()[0].pos()[1] + self.scan_area.pos()[1]
        self.connect_area.movePoint(self.connect_area.getHandles()[0], [x, y])
        self.connect_area.getHandles()[0].hide()
        self.scan_area.sigRegionChanged.connect(self.connect_scan_area)

        x = self.target_area.getHandles()[0].pos()[0] + self.target_area.pos()[0]
        y = self.target_area.getHandles()[0].pos()[1] + self.target_area.pos()[1]
        self.connect_area.movePoint(self.connect_area.getHandles()[1], [x, y])
        self.connect_area.getHandles()[1].hide()
        self.target_area.sigRegionChanged.connect(self.connect_target_area)

        # ROI | connect target position and tip position
        self.connect_position = LineSegmentROI(pos=[0, 0], positions=[0, 300000], pen=baby_pink_pen, movable=False)
        self.view_box.addItem(self.connect_position)
        self.connect_position.setZValue(8)

        x = self.tip_position.getHandles()[0].pos()[0] + self.tip_position.pos()[0]
        y = self.tip_position.getHandles()[0].pos()[1] + self.tip_position.pos()[1]
        self.connect_position.movePoint(self.connect_position.getHandles()[0], [x, y])
        self.connect_position.getHandles()[0].hide()
        self.tip_position.sigRegionChanged.connect(self.connect_tip_position)

        x = self.target_position.getHandles()[0].pos()[0] + self.target_position.pos()[0]
        y = self.target_position.getHandles()[0].pos()[1] + self.target_position.pos()[1]
        self.connect_position.movePoint(self.connect_position.getHandles()[1], [x, y])
        self.connect_position.getHandles()[1].hide()
        self.target_position.sigRegionChanged.connect(self.connect_target_position)

        # pushButton | tool bar
        self.pushButton_Full_ViewControl.clicked.connect(self.full_view)
        self.pushButton_Detail_ViewControl.clicked.connect(self.detail_view)
        self.pushButton_Reset_ViewControl.clicked.connect(self.reset_view)

        # ROI | points
        self.points = pg.PolyLineROI([0, 0], closed=False, pen=self.serial_pen[0], movable=False)
        self.view_box.addItem(self.points)
        self.points.removeHandle(self.points.handles[0]['item'])
        self.points.getHandles()[0].pen.setWidth(2)
        purple_brush = pg.mkBrush('deaaff')
        self.points.getHandles()[0].pen.setBrush(purple_brush)
        self.points.sigRegionChanged.connect(lambda: self.points_update(1))
        self.points.hide()

        # ROI | point selection
        pos = [self.points.pos()[0], self.points.pos()[1]]
        self.select_point = pg.RectROI([-100000,-100000], [200000,200000], pen=baby_green_pen, hoverPen=baby_yellow_pen, handlePen=baby_yellow_pen, movable=True, centered=True)
        self.view_box.addItem(self.select_point)
        self.select_point.removeHandle(0)
        # self.points.setParentItem(self.select_point)
        # self.select_point.sigRegionChanged.connect(lambda: self.points_update(3))
        self.select_point.sigRegionChanged.connect(self.points_overall)
        self.select_point.hide()

    # XY in conversion
    def xy_in_cnv(self, flag, xy, value):
        scroll = self.scrollBar_Yin_XY if xy else self.scrollBar_Xin_XY                     # Determin scrollbar based on xy flag
        spin = self.spinBox_Yin_XY if xy else self.spinBox_Xin_XY                           # Determin spinbox based on xy flag
        ul = min(int((3276700 - self.current_xy[xy + 2]) / self.imagine_gain), 32767)       # Available upper bound
        ll = max(int((-3276800 - self.current_xy[xy + 2]) / self.imagine_gain), -32768)     # Available lower bound

        if flag:                             # Spinbox to scrollbar
            value = spin.value()
        value = min(ul, max(ll, value))                     # Manually limit value
        spin.setValue(value)                                # Set spinbox
        scroll.setValue(value)                              # Set scrollbar
        self.current_xy[xy] = value * self.imagine_gain     # Update variable
        self.xy_offset_range(xy)                            # Set XY offset range

        # Set graphic
        self.target_position.movePoint(self.target_position.getHandles()[0], \
                                        [self.current_xy[0] + self.last_xy[2], self.current_xy[1] + self.last_xy[3]])

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
        self.current_xy[xy + 2] = value * 100
        self.xy_in_range(xy)            # Set XY in range
        self.scan_size_range()          # Set scan size range

        # Set graphic
        self.target_area.movePoint(self.target_area.getHandles()[0], [self.current_xy[2], self.current_xy[3]])

    # Scan size converstion
    def scan_size_cnv(self, flag, index, value):
        scroll = self.scrollBar_StepSize_ScanControl if index else self.scrollBar_ScanSize_ScanControl  # Determin scrollbar based on index flag
        spin = self.spinBox_StepSize_ScanControl if index else self.spinBox_ScanSize_ScanControl        # Determin spinbox based on index flag
        ul = 2048 if index else 511                                                                     # Hard upper bound
        gain = self.imagine_gain if index else 1                                                        # Gain

        # Area upper bound
        area_ul = int(2 * min(3276700 - self.current_xy[2], 3276700 - self.current_xy[3],\
                              3276800 + self.current_xy[2], 3276800 + self.current_xy[3]))
        area_ul = min(area_ul, 65535 * self.imagine_gain)
        n_ul = int(area_ul / self.scan_size[1 - index] / gain)
        if (not index) and (n_ul % 2):                                          # Step number has to be even
            n_ul -= 1
        ul = min(ul, n_ul) if index else min(ul, n_ul + 1)                      # Available upper bound, Pixel number = step number + 1
        value = spin.value() if flag else value                                 # Spinbox to scrollbar
        if (not index) and (value % 2 == 0):                                    # Pixel number has to be odd (so that step number can be even)
            value += 1
        value = min(ul, value)                                                  # Manually limit value
        spin.setValue(value)                                                    # Set spinbox
        scroll.setValue(value)                                                  # Set scrollbar
        self.scan_size[index] = value * gain if index else (value - 1) * gain   # Update variable
        self.xy_offset_range(0)                                                 # Set X offset range
        self.xy_offset_range(1)                                                 # Set Y offset range
        self.scan_size_range()                                                  # Set the other scan size range

        # Set graphic
        scan_size = self.scan_size[0]*self.scan_size[1]
        self.target_area.setSize(scan_size, center=(0.5, 0.5))
        self.scan_area.setSize(scan_size, center=(0.5, 0.5))
        self.target_position.setSize(int(scan_size/20), center=(0.5, 0.5))

    # Set XY in spin boxes range
    def xy_in_range(self, xy):
        spin = self.spinBox_Yin_XY if xy else self.spinBox_Xin_XY                           # Determin spinbox based on index flag
        ul = min(int((3276700 - self.current_xy[xy + 2]) / self.imagine_gain), 32767)       # Upper bound
        ll = max(int((-3276800 - self.current_xy[xy + 2]) / self.imagine_gain), -32768)     # Lower bound
        spin.setMaximum(ul)                                                                 # Set maximum
        spin.setMinimum(ll)                                                                 # Set minimum

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
        # Area upper bound
        area_ul = int(2 * min(3276700 - self.current_xy[2], 3276700 - self.current_xy[3], 3276800 + self.current_xy[2], 3276800 + self.current_xy[3]))
        area_ul = min(area_ul, 65535 * self.imagine_gain)
        size_ul = min(2048, int(area_ul / self.scan_size[0] / self.imagine_gain))   # Step size upper bound
        num_ul_a = int(area_ul / self.scan_size[1]) + 1
        if (num_ul_a % 2) == 0:                                                     # Pixel number has to be odd, so that step number can be even
            num_ul_a -= 1
        num_ul = min(511, num_ul_a)                                                 # Step number upper bound
        self.spinBox_StepSize_ScanControl.setMaximum(size_ul)                       # Set maximum
        self.spinBox_ScanSize_ScanControl.setMaximum(num_ul)                        # Set minimum

    # enable X/Y gain based on X/Y in
    def enable_gain(self, enable):
        # Enable XY gain radio button if both XY in are 0
        enable = enable and (self.last_xy[0] == 0) and (self.last_xy[1] == 0)
        self.radioButton_Gain0_1_XY.setEnabled(enable)
        self.radioButton_Gain10_XY.setEnabled(enable)
        self.radioButton_Gain1_XY.setEnabled(enable)

    # XY gain conversion
    def xy_gain_cnv(self, index, state):
        if state:
            gain = self.imagine_gain
            self.imagine_gain = 100 * (index == 0) + 10 * (index == 1) + (index == 3)   # Determin imagine gain based on XY gain
            self.scrollBar_Yin_XY.setValue(0)                                           # Set Xin scroll bar
            self.scrollBar_Xin_XY.setValue(0)                                           # Set Yin scroll bar
            # If changing to a larger gain, set scan area to minimum
            if self.imagine_gain > gain:
                self.scrollBar_StepSize_ScanControl.setValue(1)                         # Set step size scroll bar
                self.scrollBar_ScanSize_ScanControl.setValue(3)                         # Set pixel number scroll bar
            
            # Not very useful, XY gain wil not be enabled unless last = 0
            # Current has set to be zero by changing scrollbars
            self.last_xy[0] = self.last_xy[0] / gain * self.imagine_gain                # Update Xin variable
            self.last_xy[1] = self.last_xy[1] / gain * self.imagine_gain                # Update Yin variable
            self.current_xy[0] = self.current_xy[0] / gain * self.imagine_gain          # Update Xin variable
            self.current_xy[1] = self.current_xy[1] / gain * self.imagine_gain          # Update Yin variable

    # View Control | segment connecting scan area
    def connect_scan_area(self):
        x = self.scan_area.getHandles()[0].pos()[0] + self.scan_area.pos()[0]
        y = self.scan_area.getHandles()[0].pos()[1] + self.scan_area.pos()[1]
        self.connect_area.movePoint(self.connect_area.getHandles()[0], [x, y])
        self.connect_area.getHandles()[0].hide()

    # View Control | segment connecting target area
    def connect_target_area(self):
        x = self.target_area.getHandles()[0].pos()[0] + self.target_area.pos()[0]
        y = self.target_area.getHandles()[0].pos()[1] + self.target_area.pos()[1]
        self.connect_area.movePoint(self.connect_area.getHandles()[1], [x, y])
        self.connect_area.getHandles()[1].hide()

    # View Control | segment connecting tip position
    def connect_tip_position(self):
        x = self.tip_position.getHandles()[0].pos()[0] + self.tip_position.pos()[0]
        y = self.tip_position.getHandles()[0].pos()[1] + self.tip_position.pos()[1]
        self.connect_position.movePoint(self.connect_position.getHandles()[0], [x, y])
        self.connect_position.getHandles()[0].hide()

    # View Control | segment connecting target position
    def connect_target_position(self):
        x = self.target_position.getHandles()[0].pos()[0] + self.target_position.pos()[0]
        y = self.target_position.getHandles()[0].pos()[1] + self.target_position.pos()[1]
        self.connect_position.movePoint(self.connect_position.getHandles()[1], [x, y])
        self.connect_position.getHandles()[1].hide()

    # View Control | target area and target position update
    def default_update(self, index):
        if index == 0:      # target area moved
            xoffset = int((self.target_area.getHandles()[0].pos()[0]+self.target_area.pos()[0])/100)
            yoffset = int((self.target_area.getHandles()[0].pos()[1]+self.target_area.pos()[1])/100)
            # self.xy_off_cnv(False, 0, xoffset)
            # self.xy_off_cnv(False, 1, yoffset)
            if self.scrollBar_Xoffset_XY.value() != xoffset:
                self.scrollBar_Xoffset_XY.setValue(xoffset)
            if self.scrollBar_Yoffset_XY.value() != yoffset:
                self.scrollBar_Yoffset_XY.setValue(yoffset)
        elif index == 1:    # target position moved
            xin = int((self.target_position.getHandles()[0].pos()[0]+self.target_position.pos()[0]-self.last_xy[2])/self.imagine_gain)
            yin = int((self.target_position.getHandles()[0].pos()[1]+self.target_position.pos()[1]-self.last_xy[3])/self.imagine_gain)
            # print(self.target_position.getHandles()[0].pos()[0], self.target_position.getHandles()[0].pos()[1])
            # self.xy_in_cnv(False, 0, False, xin)
            # self.xy_in_cnv(False, 1, False, yin)
            if self.scrollBar_Xin_XY.value() != xin:
                self.scrollBar_Xin_XY.setValue(xin)
            if self.scrollBar_Yin_XY.value() != yin:
                self.scrollBar_Yin_XY.setValue(yin)

            # out of range protection
            if not(xin in range(-32768, 32768)) or not(yin in range(-32768, 32768)):
                if abs(xin) > abs(yin):
                    x = 32767 * self.imagine_gain if abs(xin-32768) < abs(xin+32768) else -32768 * self.imagine_gain
                    y = self.target_position.getHandles()[0].pos()[1] + self.target_position.pos()[1]
                    self.target_position.movePoint(self.target_position.getHandles()[0], [x+self.last_xy[2], y])
                elif abs(xin) < abs(yin):
                    x = self.target_position.getHandles()[0].pos()[0]+self.target_position.pos()[0]
                    y = 32767 * self.imagine_gain if abs(yin-32768) < abs(yin+32768) else -32768 * self.imagine_gain
                    self.target_position.movePoint(self.target_position.getHandles()[0], [x, y+self.last_xy[3]])
                else:
                    x = 32767 * self.imagine_gain if abs(xin - 32768) < abs(xin + 32768) else -32768 * self.imagine_gain
                    y = 32767 * self.imagine_gain if abs(yin - 32768) < abs(yin + 32768) else -32768 * self.imagine_gain
                    self.target_position.movePoint(self.target_position.getHandles()[0], [x+self.last_xy[2], y+self.last_xy[3]])

    # View Control | full view button slot
    def full_view(self):
        '''Zoom out to full canvas'''
        self.view_box.setRange(QRectF(-3276800,-3276800,6553600,6553600), padding=0)

    # View Control | detail view button slot
    def detail_view(self):
        '''Zoom in to scan area'''
        self.view_box.setRange(QRectF(self.scan_area.pos()[0], self.scan_area.pos()[1], \
                                      self.scan_area.size()[0], self.scan_area.size()[1]), padding=0)

    # View Control | reset button slot
    def reset_view(self):
        '''Reset target area to scan area, target position to tip position'''
        if self.point_mode == False:

            x1 = self.scan_area.getHandles()[0].pos()[0]+self.scan_area.pos()[0]
            y1 = self.scan_area.getHandles()[0].pos()[1] + self.scan_area.pos()[1]
            self.target_area.movePoint(self.target_area.getHandles()[0], [x1, y1])

            x2 = self.tip_position.pos()[0]+self.tip_position.getHandles()[0].pos()[0]
            y2 = self.tip_position.pos()[1]+self.tip_position.getHandles()[0].pos()[1]
            self.target_position.movePoint(self.target_position.getHandles()[0], [x2, y2])

        # '''Reset select point to origin'''
        elif self.point_mode:
            xmin = int(self.last_xy[2] - (self.scan_size[0] * self.scan_size[1] / 2))
            ymin = int(self.last_xy[3] - (self.scan_size[0] * self.scan_size[1] / 2))
            xmax = xmin + self.scan_size[0] * self.scan_size[1]
            ymax = ymin + self.scan_size[0] * self.scan_size[1]
            self.select_point.setPos([(xmin+xmax)/2,(ymin+ymax)/2])
            self.points_update(3)

    # Enable serial
    def enable_serial(self, enable):
        # XY gain enable/ disable
        self.enable_gain(enable)                        # Enable through enable_gain
        
        self.options.setEnabled(enable)
        self.toolButton.setEnabled(enable)
        self.file_widget.setEnabled(enable)
        self.groupBox_ScanControl.setEnabled(enable)
        
        self.pushButton_Zero_XY.setEnabled(enable)
        self.pushButton_Send_XY.setEnabled(enable)
        self.pushButton_Start_Scan.setEnabled(enable)

        # self.view_box.setMouseEnabled(x=enable, y=enable)   # Enable and disable mouse event
        
        # self.pushButton_SaveAll_Scan.setEnabled(enable and (not self.data.data))
        # self.pushButton_Info_Scan.setEnabled(enable and (not self.data.data))
        # self.pushButton_Load_Scan.setEnabled(enable)
        
        # Sub modules enable/ disable
        self.dep.enable_serial(enable)
        self.track.enable_serial(enable)
        self.spc.enable_serial(enable)    

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myScan_()
    window.show()
    sys.exit(app.exec_())

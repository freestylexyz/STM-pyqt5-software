# -*- coding: utf-8 -*-
"""
@Date     : 2021/1/15 10:55:50
@Author   : milier00
@FileName : GraphicsTest.py
"""
import sys
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup, QFileDialog, QAction,QMenu
from PyQt5.QtGui import QPixmap, QPen, QCursor
from PyQt5.QtCore import pyqtSignal, Qt, QRectF, QFileSystemWatcher, QCoreApplication, QDir, QFileInfo, QFile
from PyQt5 import QtCore
from GraphicsTest_ui import Ui_GraphicsDemo
from customROI import *
import pyqtgraph as pg
import numpy as np
import functools as ft
import numpy as np
import scipy.io as io
import cv2 as cv
# import matlab.engine
from image import myImages
from skimage import exposure, img_as_float
import copy


class myGraphicsDemo(QWidget, Ui_GraphicsDemo):
    close_signal = pyqtSignal()
    seq_list_signal = pyqtSignal(str)
    stop_signal = pyqtSignal()
    gain_changed_signal = pyqtSignal(int, int)
    send_signal = pyqtSignal(int, int, int, int, int)

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.Default.toggled.connect(ft.partial(self.change_mode, 0))
        self.Point.toggled.connect(ft.partial(self.change_mode, 1))
        self.Rescan.toggled.connect(ft.partial(self.change_mode, 2))

        self.checkBox_Resize_Rescan.toggled.connect(self.enable_resize)

        self.pushButton_Send_Default.clicked.connect(self.default_send)
        self.pushButton_Send_Point.clicked.connect(self.point_send)

        self.spinBox_x1_Rescan.editingFinished.connect(lambda: self.rescan_update(0))
        self.spinBox_y1_Rescan.editingFinished.connect(lambda: self.rescan_update(0))
        self.spinBox_x3_Rescan.editingFinished.connect(lambda: self.rescan_update(0))

        self.spinBox_x1_Default.editingFinished.connect(lambda: self.default_update(2))
        self.spinBox_y1_Default.editingFinished.connect(lambda: self.default_update(2))

        # graphicsView | Scan main view

        # viewBox | setup
        self.view_box = self.graphicsView_Scan.addViewBox(enableMenu=False, enableMouse=True)
        self.view_box.setRange(QRectF(-3276800, -3276800, 6553600, 6553600), padding=0)
        self.view_box.setLimits(xMin=-3276800, xMax=3276800, yMin=-3276800, yMax=3276800, \
                                minXRange=3, maxXRange=6553600, minYRange=3, maxYRange=6553600)
        self.view_box.setAspectLocked(True)
        self.view_box.setCursor(Qt.CrossCursor)
        self.view_box.setMouseMode(self.view_box.PanMode)
        self.view_box.sigMoveROI.connect(self.default_update)

        # ROI | define pens
        blue_pen = pg.mkPen((70, 200, 255, 255), width=1)
        green_pen = pg.mkPen((150, 220, 0, 255), width=1)
        purple_pen = pg.mkPen('deaaff', width=1)
        baby_blue_pen = pg.mkPen((130, 220, 255, 255), width=1, dash=[2, 4, 2, 4])
        baby_green_pen = pg.mkPen((210, 255, 120, 100), width=1, dash=[2, 2, 2, 2])
        baby_yellow_pen = pg.mkPen((255, 255, 0, 200), width=1, dash=[2, 2, 2, 2])
        serial_pen_0 = pg.mkPen('bee9e8', width=1, dash=[2, 2, 2, 2])
        serial_pen_1 = pg.mkPen('70d6ff', width=1)
        serial_pen_2 = pg.mkPen('ff70a6', width=1)
        serial_pen_3 = pg.mkPen('ff9770', width=1)
        serial_pen_4 = pg.mkPen('ffd670', width=1)
        serial_pen_5 = pg.mkPen('e9ff70', width=1)
        hover_pen = pg.mkPen((255, 255, 0, 100), width=1, dash=[2, 4, 2, 4])
        self.serial_pen = [serial_pen_0, serial_pen_1, serial_pen_2, serial_pen_3, serial_pen_4, serial_pen_5]

        # ROI | scan area
        self.scan_area = CrossCenterROI([0, 0], [300000, 300000], pen=blue_pen, centered=True, \
                                    movable=False, resizable=False, rotatable=False, \
                                    maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600), scaleSnap=True)
        self.scan_area.aspectLocked = True
        self.scan_area.setZValue(10)
        self.view_box.addItem(self.scan_area)
        self.scan_area.removeHandle(0)
        self.scan_area.addCustomHandle(info={'type': 't', 'pos': [0.5, 0.5],'pen':blue_pen}, index=3)
        self.scan_area.sigRegionChanged.connect(lambda: self.default_update(1))

        # ROI | target area
        self.target_area = CrossCenterROI([0, 0], [300000, 300000], pen=baby_blue_pen, centered=True, \
                                      movable=True, resizable=False, rotatable=False, \
                                      maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600), scaleSnap=True)
        self.target_area.setZValue(10)
        self.target_area.aspectLocked = True
        self.view_box.addItem(self.target_area)
        self.target_area.removeHandle(0)
        # self.target_area.hide()
        self.target_area.addCustomHandle(info={'type': 't', 'pos': [0.5, 0.5],'pen':baby_blue_pen}, index=3)
        self.target_area.sigRegionChanged.connect(lambda: self.default_update(0))

        # ROI | tip position
        cross_pos = [self.scan_area.pos()[0] + self.scan_area.getHandles()[0].pos()[0], \
                     self.scan_area.pos()[1] + self.scan_area.getHandles()[0].pos()[1]]
        cross_size = [int(self.scan_area.size()[0] / 20), int(self.scan_area.size()[1] / 20)]
        # self.tip_position = pg.CrosshairROI(pos=cross_pos, size=cross_size, pen=green_pen, movable=False, \
        #                                     resizable=False, rotatable=False)
        # self.tip_position.setZValue(10)
        # self.view_box.addItem(self.tip_position)
        # self.tip_position.removeHandle(0)
        # self.tip_position.hide()
        self.tip_position = CrossCenterROI2(cross_pos, cross_size, pen=(255, 255, 255, 0), movable=False)
        self.view_box.addItem(self.tip_position)
        self.tip_position.removeHandle(0)
        self.tip_position.addCustomHandle2_(info={'type': 't', 'pos': [0.5, 0.5]}, index=3)
        self.tip_position.sigRegionChanged.connect(lambda: self.default_update(3))
        self.tip_position.movePoint(self.tip_position.getHandles()[0], cross_pos)


        # ROI | target position
        # self.target_position = pg.CrosshairROI(pos=cross_pos, size=[1,1], pen=baby_green_pen, movable=True, \
        #                                     resizable=False, rotatable=False)
        # self.target_position.setZValue(10)
        # self.view_box.addItem(self.target_position)
        # self.target_position.removeHandle(0)
        # self.target_position.hide()
        self.target_position = CrossCenterROI2([200000,200000], cross_size, pen=(255, 255, 255, 0))
        self.view_box.addItem(self.target_position)
        self.target_position.removeHandle(0)
        self.target_position.addCustomHandle2(info={'type': 't', 'pos': [0.5, 0.5]}, index=3)
        self.target_position.movePoint(self.target_position.getHandles()[0], cross_pos)

        # ROI | rescan area
        self.rescan_area = CrossCenterROI([0, 0], [300000, 300000], pen=purple_pen, centered=True, \
                                    movable=True, resizable=True, rotatable=False, \
                                    maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600), scaleSnap=True)
        self.rescan_area.aspectLocked = True
        self.view_box.addItem(self.rescan_area)
        self.rescan_area.removeHandle(0)
        self.rescan_area.addScaleHandle([0, 0], [1, 1], index=0)
        self.rescan_area.addScaleHandle([1, 1], [0, 0], index=1)
        self.rescan_area.hide()
        self.rescan_area.getHandles()[0].hide()
        self.rescan_area.getHandles()[1].hide()
        self.rescan_area.sigRegionChanged.connect(lambda: self.rescan_update(1))

        # ROI | connect target area and scan area
        self.connect_area = LineSegmentROI(pos=[0, 0], positions=[0, 300000], pen=baby_blue_pen, movable=False)
        self.view_box.addItem(self.connect_area)
        # self.connect_area.replaceHandle(self.connect_area.getHandles()[0], self.scan_area.getHandles()[0])
        # self.scan_area.getHandles()[0].connectROI(self.connect_area)
        
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

        # ROI | connect target position and scan position
        self.connect_position = LineSegmentROI(pos=[0, 0], positions=[0, 300000], pen=baby_blue_pen, movable=False)
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

        # ROI | point
        self.point_0 = pg.PolyLineROI([0, 0], closed=False, pen=self.serial_pen[0],movable=True)
        self.view_box.addItem(self.point_0)
        self.point_0.getHandles()[0].pen.setWidth(2)
        purple_brush = pg.mkBrush('deaaff')
        self.point_0.getHandles()[0].pen.setBrush(purple_brush)
        self.point_0.hide()

        # ROI | point selection
        pos = [self.point_0.pos()[0],self.point_0.pos()[1]]
        self.select_point = pg.RectROI([-100000,-100000], [200000,200000], pen=baby_green_pen, hoverPen=baby_yellow_pen, handlePen=baby_yellow_pen)
        self.view_box.addItem(self.select_point)
        self.select_point.addScaleHandle([0, 0], [1, 1], index=1)
        self.point_0.setParentItem(self.select_point)
        # self.point_0.sigRegionChanged.connect(self.update_select_point)
        self.select_point.hide()


    def connect_scan_area(self):
        x = self.scan_area.getHandles()[0].pos()[0] + self.scan_area.pos()[0]
        y = self.scan_area.getHandles()[0].pos()[1] + self.scan_area.pos()[1]
        self.connect_area.movePoint(self.connect_area.getHandles()[0], [x, y])
        self.connect_area.getHandles()[0].hide()

    def connect_target_area(self):
        x = self.target_area.getHandles()[0].pos()[0] + self.target_area.pos()[0]
        y = self.target_area.getHandles()[0].pos()[1] + self.target_area.pos()[1]
        self.connect_area.movePoint(self.connect_area.getHandles()[1], [x, y])
        self.connect_area.getHandles()[1].hide()
        
    def connect_tip_position(self):
        x = self.tip_position.getHandles()[0].pos()[0] + self.tip_position.pos()[0]
        y = self.tip_position.getHandles()[0].pos()[1] + self.tip_position.pos()[1]
        self.connect_position.movePoint(self.connect_position.getHandles()[0], [x, y])
        self.connect_position.getHandles()[0].hide()
        
    def connect_target_position(self):
        x = self.target_position.getHandles()[0].pos()[0] + self.target_position.pos()[0]
        y = self.target_position.getHandles()[0].pos()[1] + self.target_position.pos()[1]
        self.connect_position.movePoint(self.connect_position.getHandles()[1], [x, y])
        self.connect_position.getHandles()[1].hide()

    def update_select_point(self):
        pos_x = []
        pos_y = []
        for handle in self.point_0.getHandles():
            pos_x += [handle.pos()[0]+self.point_0.pos()[0]]
            pos_y += [handle.pos()[1]+self.point_0.pos()[1]]
        max_x = max(pos_x)
        min_x = min(pos_x)
        max_y = max(pos_y)
        min_y = min(pos_y)
        margin_x = int(abs(max_x-min_x)*0.15)
        margin_y = int(abs(max_y - min_y) * 0.15)
        size_x = max_x - min_x + 2 * margin_x
        size_y = max_x - min_y + 2 * margin_y

        self.select_point.setPos(min_x-margin_x, min_y-margin_y)
        self.select_point.setSize(size_x, size_y)

    def change_mode(self, index, status):
        if status:
            if index == 0:
                self.mode = 0
                self.Default.setChecked(True)
                self.Point.setChecked(False)
                self.Rescan.setChecked(False)
                self.init_default_mode(True)
                self.init_point_mode(False)
                self.init_rescan_mode(False)
                self.view_box.setMouseEnabled(x=True, y=True)
            elif index == 1:
                self.mode = 1
                self.Point.setChecked(True)
                self.Default.setChecked(False)
                self.Rescan.setChecked(False)
                self.init_point_mode(True)
                self.init_default_mode(False)
                self.init_rescan_mode(False)
                self.view_box.setMouseEnabled(x=False,y=False)
            elif index == 2:
                self.mode = 2
                self.Rescan.setChecked(True)
                self.Default.setChecked(False)
                self.Point.setChecked(False)
                self.init_rescan_mode(True)
                self.init_default_mode(False)
                self.init_point_mode(False)
                self.view_box.setMouseEnabled(x=False, y=False)
        else:
            self.init_rescan_mode(False)
            self.init_default_mode(False)
            self.init_point_mode(False)

    def init_default_mode(self, status):
        if status:
            self.scan_area.show()
            self.target_area.show()
            self.connect_area.show()
            self.tip_position.show()
            self.target_position.show()
            self.connect_position.show()
        else:
            self.scan_area.hide()
            self.target_area.hide()
            self.connect_area.hide()
            self.tip_position.hide()
            self.target_position.hide()
            self.connect_position.hide()

    def init_point_mode(self, status):
        if status:
            self.point_0.show()
            self.select_point.show()
        else:
            self.point_0.hide()
            self.select_point.hide()

    def enable_resize(self, status):
        if status:
            self.rescan_area.getHandles()[0].show()
            self.rescan_area.getHandles()[1].show()
        else:
            self.rescan_area.getHandles()[0].hide()
            self.rescan_area.getHandles()[1].hide()

    def init_rescan_mode(self, status):
        if status:
            self.rescan_area.show()
            self.enable_resize(self.checkBox_Resize_Rescan.isChecked())
            x1 = int(self.rescan_area.pos()[0])
            y1 = int(self.rescan_area.pos()[1])
            x3 = int(self.rescan_area.getHandles()[1].pos()[0])
            self.spinBox_x1_Rescan.setValue(x1)
            self.spinBox_y1_Rescan.setValue(y1)
            self.spinBox_x3_Rescan.setValue(x3)
        else:
            self.rescan_area.hide()
            self.enable_resize(self.checkBox_Resize_Rescan.isChecked())

    def default_send(self):

        self.target_area.translatable = False

        x2 = self.target_area.getHandles()[0].pos()[0] + self.target_area.pos()[0]
        y2 = self.target_area.getHandles()[0].pos()[1] + self.target_area.pos()[1]
        x1 = self.scan_area.getHandles()[0].pos()[0] + self.scan_area.pos()[0]
        y1 = self.scan_area.getHandles()[0].pos()[1] + self.scan_area.pos()[1]

        self.scan_area.movePoint(self.scan_area.getHandles()[0], [x2, y2])

        self.target_area.translatable = True

    def point_send(self):
        x = self.spinBox_x_Point.value()
        y = self.spinBox_y_Point.value()

    def rescan_send(self):
        x = self.spinBox_x_Rescan.value()
        y = self.spinBox_y_Rescan.value()

    def default_update(self, index):

        if index == 0:      # target area moved
            x1 = int(self.target_area.getHandles()[0].pos()[0]+self.target_area.pos()[0])
            y1 = int(self.target_area.getHandles()[0].pos()[1]+self.target_area.pos()[1])
            self.spinBox_x1_Default.setValue(x1)
            self.spinBox_y1_Default.setValue(y1)
        elif index == 1:    # scan area moved
            x2 = int(self.scan_area.getHandles()[0].pos()[0]+self.scan_area.pos()[0])
            y2 = int(self.scan_area.getHandles()[0].pos()[1]+self.scan_area.pos()[1])
            self.spinBox_x2_Default.setValue(x2)
            self.spinBox_y2_Default.setValue(y2)
        elif index == 2:    # target area inputted
            x1 = self.spinBox_x1_Default.value()
            y1 = self.spinBox_y1_Default.value()
            self.target_area.movePoint(self.target_area.getHandles()[0], [x1,y1])


    def point_update(self):
        pass

    def rescan_update(self, index):
        if index == 0:
            self.enable_resize(self.checkBox_Resize_Rescan.isChecked())
            x1 = self.spinBox_x1_Rescan.value()
            y1 = self.spinBox_y1_Rescan.value()
            x3 = self.spinBox_x3_Rescan.value()
            self.rescan_area.movePoint(self.rescan_area.getHandles()[0], [x1, y1], finish=False)
            self.rescan_area.movePoint(self.rescan_area.getHandles()[1], [x3 + x1, x3 + y1])
            self.rescan_area.setPos([x1, y1])
        elif index == 1:
            self.enable_resize(self.checkBox_Resize_Rescan.isChecked())
            x1 = int(self.rescan_area.pos()[0])
            y1 = int(self.rescan_area.pos()[1])
            x3 = int(self.rescan_area.getHandles()[1].pos()[0])
            self.spinBox_x1_Rescan.setValue(x1)
            self.spinBox_y1_Rescan.setValue(y1)
            self.spinBox_x3_Rescan.setValue(x3)

    # tool bar button slot | Full view
    def full_view(self):
        '''Zoom out to full canvas'''
        self.view_box.setRange(QRectF(-3276800,-3276800,6553600,6553600), padding=0)

    # tool bar button slot | Detail view
    def detail_view(self):
        '''Zoom in to scan area'''
        self.view_box.setRange(QRectF(self.scan_area.pos()[0], self.scan_area.pos()[1], \
                                      self.scan_area.size()[0], self.scan_area.size()[1]), padding=0)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myGraphicsDemo()
    window.show()
    sys.exit(app.exec_())
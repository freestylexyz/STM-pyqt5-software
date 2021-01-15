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
import matlab.engine
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

        # graphicsView | Scan main view

        # viewBox | setup
        self.view_box = self.graphicsView_Scan.addViewBox(enableMenu=False, enableMouse=True)
        self.view_box.setRange(QRectF(-3276800, -3276800, 6553600, 6553600), padding=0)
        self.view_box.setLimits(xMin=-3276800, xMax=3276800, yMin=-3276800, yMax=3276800, \
                                minXRange=3, maxXRange=6553600, minYRange=3, maxYRange=6553600)
        self.view_box.setAspectLocked(True)
        self.view_box.setCursor(Qt.CrossCursor)
        self.view_box.setMouseMode(self.view_box.PanMode)
        # self.view_box.contextMenu = []

        # ROI | define pens
        blue_pen = pg.mkPen((70, 200, 255, 255), width=1)
        green_pen = pg.mkPen((150, 220, 0, 255), width=1)
        purple_pen = pg.mkPen('deaaff', width=1)
        baby_blue_pen = pg.mkPen((130, 220, 255, 255), width=1, dash=[2, 4, 2, 4])
        baby_green_pen = pg.mkPen((210, 255, 120, 255), width=1, dash=[2, 2, 2, 2])
        serial_pen_0 = pg.mkPen('bee9e8', width=1, dash=[2, 2, 2, 2])
        serial_pen_1 = pg.mkPen('70d6ff', width=1)
        serial_pen_2 = pg.mkPen('ff70a6', width=1)
        serial_pen_3 = pg.mkPen('ff9770', width=1)
        serial_pen_4 = pg.mkPen('ffd670', width=1)
        serial_pen_5 = pg.mkPen('e9ff70', width=1)
        self.serial_pen = [serial_pen_0, serial_pen_1, serial_pen_2, serial_pen_3, serial_pen_4, serial_pen_5]

        # ROI | scan area
        self.scan_area = pg.RectROI([0, 0], [300000, 300000], pen=blue_pen, centered=True, \
                                    movable=False, resizable=False, rotatable=False, \
                                    maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600), scaleSnap=True)
        # self.scan_area.sigHoverEvent.connect(self.mouseDragEvent)
        self.scan_area.aspectLocked = True
        self.scan_area.setZValue(10)
        self.view_box.addItem(self.scan_area)
        self.scan_area.removeHandle(0)

        # ROI | target area
        self.target_area = pg.RectROI([0, 0], [300000, 300000], pen=baby_blue_pen, centered=True, \
                                      movable=False, resizable=False, rotatable=False, \
                                      maxBounds=QRectF(-3276800, -3276800, 6553600, 6553600), scaleSnap=True)
        self.target_area.setZValue(10)
        self.target_area.aspectLocked = True
        self.view_box.addItem(self.target_area)
        self.target_area.removeHandle(0)
        self.target_area.hide()

        # ROI | tip position
        cross_pos = [int(self.scan_area.pos()[0] + self.scan_area.size()[0] / 2), \
                     int(self.scan_area.pos()[1] + self.scan_area.size()[0] / 2)]
        cross_size = [int(self.scan_area.size()[0] / 20), int(self.scan_area.size()[1] / 20)]
        self.tip_position = pg.CrosshairROI(pos=cross_pos, size=cross_size, pen=green_pen, movable=False, \
                                            resizable=False, rotatable=False)
        self.tip_position.setZValue(10)
        self.view_box.addItem(self.tip_position)

        self.tip_position.removeHandle(0)

        # ROI | target position
        self.target_Rescan = pg.CrosshairROI(pos=cross_pos, size=cross_size, pen=baby_green_pen, movable=False, \
                                            resizable=False, rotatable=False)
        self.target_Rescan.setZValue(10)
        self.view_box.addItem(self.target_Rescan)
        self.target_Rescan.removeHandle(0)
        self.target_Rescan.hide()

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

        # ROI | point
        self.point_0 = pg.PolyLineROI([0, 0], closed=False, pen=self.serial_pen[0])
        self.view_box.addItem(self.point_0)
        self.point_0.hide()

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
            elif index == 1:
                self.mode = 1
                self.Point.setChecked(True)
                self.Default.setChecked(False)
                self.Rescan.setChecked(False)
                self.init_point_mode(True)
                self.init_default_mode(False)
                self.init_rescan_mode(False)
            elif index == 2:
                self.mode = 2
                self.Rescan.setChecked(True)
                self.Default.setChecked(False)
                self.Point.setChecked(False)
                self.init_rescan_mode(True)
                self.init_default_mode(False)
                self.init_point_mode(False)
        else:
            self.init_rescan_mode(False)
            self.init_default_mode(False)
            self.init_point_mode(False)


    def init_default_mode(self, status):
        if status:
            self.scan_area.show()
            self.target_area.show()
            self.tip_position.show()
            self.target_Rescan.show()
        else:
            self.scan_area.hide()
            self.target_area.hide()
            self.tip_position.hide()
            self.target_Rescan.hide()

    def init_point_mode(self, status):
        if status:
            self.point_0.show()
        else:
            self.point_0.hide()

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
        x1 = self.spinBox_x1_Default.value()
        x2 = self.spinBox_x2_Default.value()
        y1 = self.spinBox_y1_Default.value()
        y2 = self.spinBox_y2_Default.value()

    def point_send(self):
        x = self.spinBox_x_Point.value()
        y = self.spinBox_y_Point.value()

    def rescan_send(self):
        x = self.spinBox_x_Rescan.value()
        y = self.spinBox_y_Rescan.value()

    def default_update(self):
        pass

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
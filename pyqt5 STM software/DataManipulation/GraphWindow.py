# -*- coding: utf-8 -*-
"""
@Date     : 2021/3/22 09:32:12
@Author   : milier00
@FileName : GraphWindow.py
"""
import sys
import os
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup, QMainWindow, QAction, QVBoxLayout, QCheckBox, QRadioButton, QFileDialog
from PyQt5.QtCore import pyqtSignal, Qt, QRectF
from PyQt5.QtGui import QIntValidator, QIcon
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
from GraphWindow_ui import Ui_Graph
import numpy as np
import conversion as cnv
import functools as ft
import math
import pickle
import pyqtgraph as pg
from images import myImages
import cv2 as cv
import copy

class myGraphWindow(QWidget, Ui_Graph):
    # Common signal
    close_signal = pyqtSignal(str)
    update_display_signal = pyqtSignal()

    def __init__(self, data_path, img_index):
        super().__init__()
        self.setupUi(self)
        self.init_UI(data_path, img_index)

    def init_UI(self, data_path, img_index):

        with open(data_path, 'rb') as input:
            self.data = pickle.load(input)
            self.data.path = data_path  # Change file path
        self.img_index = img_index
        print(type(self.data.data), self.data.data.shape[:])
        # self.setWindowFlags(Qt.WindowStaysOnTopHint)
        # self.setWindowFlags(Qt.FramelessWindowHint)
        # self.setWindowFlags(Qt.WindowMinimizeButtonHint)
        self.view_box = self.graphicsView.addViewBox()
        self.view_box.setRange(QRectF(-512, -512, 512, 512), padding=0)
        self.view_box.setLimits(xMin=-512, xMax=512, yMin=-512, yMax=512, \
                                minXRange=3, maxXRange=1024, minYRange=3, maxYRange=1024)
        self.view_box.setAspectLocked(True)
        self.view_box.setCursor(Qt.CrossCursor)
        self.view_box.setMouseMode(self.view_box.PanMode)

        self.img_display = pg.ImageItem()
        self.view_box.addItem(self.img_display)

        self.raw_img = copy.deepcopy(self.data.data[img_index])
        self.current_img = copy.deepcopy(self.raw_img)
        self.img_display.setImage(self.current_img)

        # ROI | image box
        self.roi = pg.ROI([0, 0], [self.img_display.width(), self.img_display.height()], resizable=False, removable=True, handlePen=(255,255,255,255))
        self.roi.addRotateHandle([1, 0], [0.5, 0.5])
        self.roi.addRotateHandle([0, 1], [0.5, 0.5])
        self.view_box.addItem(self.roi)
        self.img_display.setParentItem(self.roi)

        self.view_box.setRange(QRectF(0, 0, self.img_display.width(), self.img_display.height()), padding=0)
        self.myimg = myImages()

        # ROI | define pens
        blue_pen = pg.mkPen((100, 200, 255, 255), width=1)
        green_pen = pg.mkPen((150, 220, 0, 255), width=1)

        # ROI | Line Cut
        self.linecut = pg.LineSegmentROI([[0, 0], [self.img_display.width()/2, self.img_display.height()/2]], pen=green_pen)
        self.linecut.rotateSnapAngle = 0.01
        self.linecut.setZValue(10)
        self.view_box.addItem(self.linecut)
        self.linecut.removeHandle(0)
        self.linecut.addTranslateHandle(pos=[0, 0], index=0)
        self.linecut.hide()

        # ROI | Grid
        self.vgrid1 = pg.InfiniteLine(angle=90, movable=False)
        self.vgrid2 = pg.InfiniteLine(angle=90, movable=False)
        self.hgrid1 = pg.InfiniteLine(angle=0, movable=False)
        self.hgrid2 = pg.InfiniteLine(angle=0, movable=False)

        self.vgrid1.setPos(self.img_display.width()/3)
        self.vgrid2.setPos(self.img_display.width()*2/3)
        self.hgrid1.setPos(self.img_display.height()/3)
        self.hgrid2.setPos(self.img_display.height()*2/3)

        # pushButton | View Control
        self.pushButton_Rotate.clicked.connect(self.rotate)

        # radioButton | View Control
        self.pallet_group = QButtonGroup()
        self.pallet_group.addButton(self.radioButton_Color, 0)
        self.pallet_group.addButton(self.radioButton_Gray, 1)
        self.pallet_group.buttonToggled[int, bool].connect(self.pallet_changed)

        # checkBox | View Control
        self.filter_group = QButtonGroup()
        self.filter_group.addButton(self.checkBox_Reverse, 0)
        self.filter_group.addButton(self.checkBox_Illuminated, 1)
        self.filter_group.addButton(self.checkBox_PlaneFit, 2)
        self.filter_group.setExclusive(False)
        self.filter_group.buttonToggled[int, bool].connect(self.filter_changed)

        self.update_display_signal.connect(self.update_display)

    # update data
    def update_data(self, data_path, img_index):
        with open(data_path, 'rb') as input:
            self.data = pickle.load(input)
            self.data.path = data_path  # Change file path

        print(type(self.data.data), self.data.data.shape[:])

        self.raw_img = copy.deepcopy(self.data.data[img_index])
        self.current_img = copy.deepcopy(self.raw_img)
        self.img_display.setImage(self.current_img)

        self.view_box.setRange(QRectF(0, 0, self.img_display.width(), self.img_display.height()), padding=0)
        self.update_display_signal.emit()

    # rotate button slot | rotate
    def rotate(self):
        '''Rotate image 90° counterclockwise.'''
        self.roi.rotate(90, center=[0.5, 0.5])

    # grid button slot | grid on/off
    def grid(self):
        if self.pushButton_Grid.isChecked():
            self.grid = pg.GridItem()
            self.view_box.addItem(self.grid)
            self.grid.setParentItem(self.img_display)
            # print(self.grid.opts['pen'].color().name())
            self.grid.setPen((255,255,255))
            # self.grid.setTickSpacing(x=[self.img_display.width()/4], y=[self.img_display.height()/4])
            print("grid on!!")
        else:
            self.view_box.removeItem(self.grid)
            print("grid off!!")

    # pallet radioButton slot | gray, color
    def pallet_changed(self, index, status):
        '''Change color map for displayed image: afm-hot or gray.'''
        if status:
            self.update_display_signal.emit()

    # filter checkBox slot | reverse, Illuminated and Plane fit
    def filter_changed(self, index, status):
        '''Process image based on checkBox signal: Reverse, Illuminated and Plane fit.'''

        # Set mutual exclusion of Illuminated and Plane fit
        if self.checkBox_Illuminated.isChecked():
            if index == 2 and status:
                self.checkBox_Illuminated.setChecked(False)
                self.checkBox_PlaneFit.setChecked(True)
            else:
                self.checkBox_Illuminated.setChecked(True)
                self.checkBox_PlaneFit.setChecked(False)

        if self.checkBox_PlaneFit.isChecked():
            if index == 1 and status:
                self.checkBox_PlaneFit.setChecked(False)
                self.checkBox_Illuminated.setChecked(True)
            else:
                self.checkBox_PlaneFit.setChecked(True)
                self.checkBox_Illuminated.setChecked(False)

        # CheckBox status variable
        if_reverse = self.checkBox_Reverse.isChecked()
        if_illuminated = self.checkBox_Illuminated.isChecked()
        if_plane_fit = self.checkBox_PlaneFit.isChecked()

        # Get current selected display mode
        if if_reverse and (not if_plane_fit) and (not if_illuminated):
            reverse_gray_img = self.myimg.gray2reverse(self.raw_img)
            self.current_img = reverse_gray_img
        elif if_illuminated and (not if_plane_fit) and (not if_reverse):
            illuminated_img = self.myimg.illuminated(self.raw_img)
            self.current_img = illuminated_img
        elif if_plane_fit and (not if_illuminated) and (not if_reverse):
            planefit_img = self.myimg.plane_fit(self.raw_img)
            self.current_img = planefit_img
        elif if_plane_fit and if_illuminated and (not if_reverse):
            planefit_img = self.myimg.plane_fit(self.raw_img)
            illuminated_planefit_img = self.myimg.illuminated(planefit_img)
            self.current_img = illuminated_planefit_img
        elif if_reverse and if_plane_fit and (not if_illuminated):
            planefit_img = self.myimg.plane_fit(self.raw_img)
            reverse_planefit_img = self.myimg.gray2reverse(planefit_img)
            self.current_img = reverse_planefit_img
        elif if_reverse and if_illuminated and (not if_plane_fit):
            illuminated_img = self.myimg.illuminated(self.raw_img)
            revered_illuminated_img = self.myimg.gray2reverse(illuminated_img)
            self.current_img = revered_illuminated_img
        elif if_reverse and if_illuminated and if_plane_fit:
            planefit_img = self.myimg.plane_fit(self.raw_img)
            illuminated_planefit_img = self.myimg.illuminated(planefit_img)
            revered_illuminated_planefit_img = self.myimg.gray2reverse(illuminated_planefit_img)
            self.current_img = revered_illuminated_planefit_img
        elif (not if_reverse) and (not if_illuminated) and (not if_plane_fit):
            self.current_img = copy.deepcopy(self.raw_img)

        self.update_display_signal.emit()

    # update display image
    def update_display(self):
        '''Update image based on user selected filter and colormap.'''
        if self.radioButton_Gray.isChecked():
            psudo_gray_img = cv.cvtColor(self.current_img, cv.COLOR_GRAY2BGR)
            self.color_current_img = psudo_gray_img
        elif self.radioButton_Color.isChecked():
            color_img = self.myimg.color_map(self.current_img, 36)
            self.color_current_img = color_img
        self.img_display.setImage(self.color_current_img)

    def on_inTopCheckBox_clicked(self, checked):
        if not checked:
            self.setWindowFlags(QtCore.Qt.Widget) #取消置顶
        else:
            self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint) #置顶
        self.show()

    def closeEvent(self, event):
        self.close_signal.emit(self.windowTitle())
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myGraphWindow("C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software/data/02012100.stm", 0)
    window.show()
    sys.exit(app.exec_())
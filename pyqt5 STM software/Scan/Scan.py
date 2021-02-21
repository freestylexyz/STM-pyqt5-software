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
from PyQt5 import QtGui
from PyQt5.QtCore import pyqtSignal , Qt, QRectF
from PyQt5 import QtCore
import numpy as np
from Scan_ import myScan_
from DigitalSignalProcessor import myDSP
from customGradientWidget import *
import conversion as cnv
import pickle
import pyqtgraph as pg
from images import myImages
import cv2 as cv
import copy

class myScan(myScan_):
    close_signal = pyqtSignal()                                     # Close scan window signal
    seq_list_signal = pyqtSignal(str)                               # Open sequence list window signal
    gain_changed_signal = pyqtSignal(int, int)                      # Change XY gian signal
    stop_signal = pyqtSignal()                                      # Stop signal
    send_signal = pyqtSignal(int, int, int, int, int)               # Send signal
    scan_signal = pyqtSignal(int, int, int, int, int, int, int)     # Scan signal
    update_display_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setup_UI()

    def setup_UI(self):
        # Point Editor signal
        self.point_editor.points_signal.connect(self.points_update)
        self.point_editor.one_point_signal.connect(self.points_update)
        self.point_editor.close_signal.connect(self.edit_points)

        # Track signal
        self.track.pushButton_PlaneFit.clicked.connect(self.track_update_fit)               # Track want to use current image plane fit
        
        # Spectroscopy signal
        self.spc.pushButton_EditPoints.clicked.connect(lambda: self.edit_points(0))                    # Edit points mode
        self.spc.pushButton_LockIn_General.clicked.connect(self.lockin.show)                # Spectroscopy want open lock in window
        self.spc.adv.pushButton_SelectPattern_Rescan.clicked.connect(self.select_pattern)   # Select pattern for rescan
        self.spc.adv.checkBox_Tracking_Correction.stateChanged.connect(self.open_track)     # Spectroscopy want to track

        # PushButton | open windows
        self.pushButton_LockIn_ScanControl.clicked.connect(self.lockin.show)                # Open lock in window
        self.pushButton_ScanOptions_Scan.clicked.connect(self.scan_options.show)            # Open scan option window
        self.pushButton_SendOptions_Scan.clicked.connect(self.send_options.show)            # Open send option window
        self.pushButton_Info_Scan.clicked.connect(self.open_info)                           # Open scan data info window
        self.pushButton_Info_Scan.clicked.connect(lambda: self.edit_points(0))
        
        # PushButton | file
        self.pushButton_SaveAll_Scan.clicked.connect(self.save)                             # Save data
        self.pushButton_Load_Scan.clicked.connect(self.load)                                # Load data

        # radioButton | X/Y gain
        self.XY_gain_group.buttonToggled[int, bool].connect(self.gain_changed_emit)

        # pushButton | send and zero
        self.pushButton_Send_XY.clicked.connect(lambda: self.send_emit(1))
        self.pushButton_Zero_XY.clicked.connect(lambda: self.send_emit(0))

        # radioButton | View Control
        self.pallet_group = QButtonGroup()
        self.pallet_group.addButton(self.radioButton_Color_Scan, 0)
        self.pallet_group.addButton(self.radioButton_Gray_Scan, 1)
        self.pallet_group.buttonToggled[int, bool].connect(self.pallet_changed)

        # checkBox | View Control
        self.filter_group = QButtonGroup()
        self.filter_group.addButton(self.checkBox_Reverse_Scan, 0)
        self.filter_group.addButton(self.checkBox_Illuminated_Scan, 1)
        self.filter_group.addButton(self.checkBox_PlaneFit_Scan, 2)
        self.filter_group.setExclusive(False)
        self.filter_group.buttonToggled[int, bool].connect(self.filter_changed)
        
        # pushButton | scan
        self.pushButton_Start_Scan.clicked.connect(self.scan_emit)

        ## graphicsView | Scan main view

        self.update_display_signal.connect(self.update_display)

        # imageItem | setup
        self.img_display = pg.ImageItem()
        self.view_box.addItem(self.img_display)

        ## graphicsView | Scan main view

        # imageItem | setup
        self.img_display = pg.ImageItem()
        self.view_box.addItem(self.img_display)
        self.myimg = myImages()

        '''Image plot for test'''
        # Test image no.1
        # raw_img = cv.imread('../data/scan_example_gray.jpg')
        # Test image no.2
        # self.myimg.read_csv('../data/real_stm_img.csv')
        # raw_img = cv.imread('../data/real_stm_img.jpeg')

        # self.raw_img = cv.cvtColor(raw_img, cv.COLOR_BGR2GRAY)
        # self.current_img = copy.deepcopy(self.raw_img)
        # self.img_display.setImage(self.current_img)

        # self.img_display.setRect(QRectF(-300000, -300000, 300000, 300000))
        # self.view_box.setRange(QRectF(-300000, -300000, 300000, 300000), padding=0)

        # pallet bar | setup
        self.ll = QtGui.QGridLayout()
        self.ll.setSpacing(0)
        self.graphicsView_ColorBar_Scan.setLayout(self.ll)
        self.pallet_bar = customGradientWidget(orientation='right', allowAdd=False)
        self.ll.addWidget(self.pallet_bar, 0, 1)
        self.graphicsView_ColorBar_Scan.setMinimumHeight(484)
        self.graphicsView_ColorBar_Scan.setMaximumHeight(512)
        self.graphicsView_ColorBar_Scan.setMinimumWidth(30)
        self.graphicsView_ColorBar_Scan.setMaximumWidth(30)
        self.graphicsView_ColorBar_Scan.setContentsMargins(0,0,0,0)
        self.pallet_bar.loadPreset('grey')

    # Init scan
    def init_scan(self, dsp, bias_dac, preamp_gain):
        succeed = dsp.succeed
        bias_ran = dsp.dacrange[13]
        
        # Set up gain radio button
        if dsp.lastgain[0] == 0:
            self.radioButton_Gain10_XY.setChecked(True)
            self.imagine_gain = 100
        elif dsp.lastgain[0] == 1:
            self.radioButton_Gain1_XY.setChecked(True)
            self.imagine_gain = 10
        elif dsp.lastgain[0] == 3:
            self.radioButton_Gain0_1_XY.setChecked(True)
            self.imagine_gain = 1
        
        # Set up XY control
        self.scrollBar_Xin_XY.setValue(dsp.lastdac[0] - 32768)
        self.scrollBar_Yin_XY.setValue(dsp.lastdac[15] - 32768)
        self.scrollBar_Xoffset_XY.setValue(dsp.lastdac[1] - 32768)
        self.scrollBar_Yoffset_XY.setValue(dsp.lastdac[14] - 32768)

        # Set up XY indication
        self.send_update(0x10, 0x1f, dsp.lastdac[0], dsp.lastdac[15])
        self.send_update(0x11, 0x1e, dsp.lastdac[1], dsp.lastdac[14])

        # Set up view in case of successfully finding DSP
        self.enable_gain(succeed)
        self.pushButton_Send_XY.setEnabled(succeed)
        self.pushButton_Zero_XY.setEnabled(succeed)
        self.pushButton_Start_Scan.setEnabled(succeed)
        # self.pushButton_SaveAll_Scan.setEnabled(not self.data.data)
        # self.pushButton_Info_Scan.setEnabled(not self.data.data)
        
        # Init sub modules
        self.dep.init_deposition(succeed, bias_dac, bias_ran, self.dep_seq_list, self.dep_seq_selected)
        self.spc.init_spc(succeed, bias_dac, bias_ran, self.spc_seq_list, self.spc_seq_selected)
        self.track.init_track(succeed)

    # if X/Y gain is changed by user, emit signal
    def gain_changed_emit(self, gain, status):
        if status:
            self.gain_changed_signal.emit(0, gain)  # Change X gain
            self.gain_changed_signal.emit(1, gain)  # Change Y gain

    # Emit send signal
    def send_emit(self, index):
        if self.idling:
            if not index:       # Zero XY in
                self.scrollBar_Xin_XY.setValue(0)               # Set X in target as 0
                self.scrollBar_Yin_XY.setValue(0)               # Set Y in target as 0
            xoff = self.scrollBar_Xoffset_XY.value() + 0x8000   # Get X offset target
            yoff = self.scrollBar_Yoffset_XY.value() + 0x8000   # Get Y offset target
            xin = self.scrollBar_Xin_XY.value() + 0x8000        # Get X in target
            yin = self.scrollBar_Yin_XY.value() + 0x8000        # Get Y in target
            self.send_signal.emit(index, xin, yin, xoff, yoff)  # Emit send signal
        else:
            self.pushButton_Zero_XY.setEnabled(False)           # Disable stop button to avoid sending stop signal twice
            self.pushButton_Send_XY.setEnabled(False)           # Disable stop button to avoid sending stop signal twice
            self.stop_signal.emit()                             # Emit stop signal
            
    # Emit scan signal
    def scan_emit(self):
        flag = self.saved        # If able to start flag
        
        # Ask if overwrite data if not saved
        if self.idling and (not flag):
            msg = QMessageBox.question(None, "Scan", "Image not saved, do you want to continue?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            flag = (msg == QMessageBox.Yes)
            
        if self.idling and flag:    # Start case if idling and able to start
            xoff = self.scrollBar_Xoffset_XY.value() + 0x8000           # X offset
            yoff = self.scrollBar_Yoffset_XY.value() + 0x8000           # Y offset
            xin = self.scrollBar_Xin_XY.value() + 0x8000                # X in
            yin = self.scrollBar_Yin_XY.value() + 0x8000                # Yin
            step_size = self.scrollBar_StepSize_ScanControl.value()     # Pixel size
            step_num = self.scrollBar_ScanSize_ScanControl.value()      # Pixel number
            self.scan_signal.emit(xin, yin, xoff, yoff, self.imagine_gain, step_num, step_size)     # Emit scan start signal
        elif not self.idling:       # Stop case if not idling
            self.pushButton_Start_Scan.setEnabled(False)                # Disable stop button to avoid sending stop signal twice
            self.stop_signal.emit()                                     # Emit stop signal

    # Bias range change slot
    def bias_ran_change(self, ran):
        self.dep.bias_ran_change(ran)   # Change related parts in depostion
        self.spc.bias_ran_change(ran)   # Change related parts in spectroscopy

    # Update ramp diagnal
    def send_update(self, channels, channell, currents, currentl):
        func_dict = {0x10: self.label_Xin_XY, 0x1f: self.label_Yin_XY, 0x11: self.label_Xoffset_XY, 0x1e: self.label_Yoffset_XY}    # Indicator dictionary
        var_dict = {0x10: 0, 0x1f: 1, 0x11: 2, 0x1e: 3}                                                                             # Variable dictionary
        gain_dict = {0x10: self.imagine_gain, 0x1f: self.imagine_gain, 0x11: 100, 0x1e: 100}                                        # Gain dictionary
        func_dict[channels].setText(str(currents - 0x8000))                             # Update first channel indicator
        func_dict[channell].setText(str(currentl - 0x8000))                             # Update second channel indicator
        self.last_xy[var_dict[channels]] = (currents - 0x8000) * gain_dict[channels]    # Update first channel variable
        self.last_xy[var_dict[channell]] = (currentl - 0x8000) * gain_dict[channell]    # Update second channel variable

        if (channels == 0x11) or (channels == 0x1e):
            
            self.scan_area.movePoint(self.scan_area.getHandles()[0], [self.last_xy[2], self.last_xy[3]])
            # self.xy_in_cnv(False, 0, self.scrollBar_Xin_XY.value())
            # self.xy_in_cnv(False, 1, self.scrollBar_Yin_XY.value())
            self.target_position.movePoint(self.target_position.getHandles()[0], \
                                            [self.current_xy[0] + self.last_xy[2], self.current_xy[1] + self.last_xy[3]])
        self.tip_position.movePoint(self.tip_position.getHandles()[0], [self.last_xy[0] + self.last_xy[2], self.last_xy[1] + self.last_xy[3]])

    # Update scan
    def scan_update(self, rdata):
        # !!! Update graphic view
        plot_data = self.data.updata_data(rdata)            # Update scan data and obtain data used for plot
        self.raw_img = copy.deepcopy(plot_data)
        self.current_img = copy.deepcopy(self.raw_img)
        self.img_display.setImage(self.current_img)
        self.img_display.setRect(QRectF(int(self.last_xy[2] - (self.scan_size[0] * self.scan_size[1] / 2)),
                                        int(self.last_xy[3] - (self.scan_size[0] * self.scan_size[1] / 2)),
                                        self.scan_size[0] * self.scan_size[1],
                                        self.scan_size[0] * self.scan_size[1]))
        self.view_box.setRange(QRectF(int(self.last_xy[2] - (self.scan_size[0] * self.scan_size[1] / 2)),
                                      int(self.last_xy[3] - (self.scan_size[0] * self.scan_size[1] / 2)),
                                      self.scan_size[0] * self.scan_size[1],
                                      self.scan_size[0] * self.scan_size[1]), padding=0)
        
    # Update plane fit paramenters in track window
    def track_update_fit(self):
        self.track.spinBox_X_PlaneFit.setValue(self.tilt[0])    # Sync tilt X
        self.track.spinBox_Y_PlaneFit.setValue(self.tilt[1])    # Sync tilt y

    # Track update function
    def track_update(self, x, y):
        self.send_update(0x10, 0x1f, x, y)                                              # Update tip position
        # If out of track size
        stop_flag = (abs(self.track.x - x) > self.track.track_size)                     # X out of boundary
        stop_flag = stop_flag or (abs(self.track.y - y) > self.track.track_size)        # Y out of boundary
        if stop_flag and (not self.stop):       # Either X or Y outof boudary, avoid sending stop signal twice
            self.track.pushButton_Start_Track.setEnable(False)                          # Disable stop button to avoid sending stop signal twice
            self.stop_signal.emit()                                                     # Emit stop signal

    # !!!
    # Open scan information windwow
    def open_info(self):
        # self.info.init_info()
        # self.info.show()
        # print('last', self.last_xy)
        # print('current', self.current_xy)
        # print('size', self.scan_size)
        pass


    # !!!
    # Select pattern mode
    def select_pattern(self):
        pass

    # Edit points mode
    def edit_points(self, index):
        if index == 0:      # open point editor
            self.point_mode = True
            self.point_editor.show()
            self.init_point_mode(True)
            self.init_default_mode(False)
            # self.init_rescan_mode(False)
            self.view_box.setMouseEnabled(x=False, y=False)
            xmin = int(self.last_xy[2] - (self.scan_size[0] * self.scan_size[1] / 2))
            ymin = int(self.last_xy[3] - (self.scan_size[0] * self.scan_size[1] / 2))
            xmax = xmin + self.scan_size[0] * self.scan_size[1]
            ymax = ymin + self.scan_size[0] * self.scan_size[1]
            self.view_box.setLimits(xMin=xmin, xMax=xmax, yMin=ymin, yMax=ymax, minXRange=3, maxXRange=xmax - xmin,
                                    minYRange=3, maxYRange=ymax - ymin)
            self.view_box.setRange(QRectF(xmin, xmax, self.scan_size[0] * self.scan_size[1], self.scan_size[0] * self.scan_size[1]), padding=0)
            self.select_point.setSize([(xmax-xmin)/20, (ymax-ymin)/20])
            # self.select_point.setPos([int((xmax + xmin)/2-(xmax-xmin)/40), int((ymax + ymin)/2-(ymax-ymin)/40)])
            self.select_point.setPos([0,0])
            self.select_point.maxBounds = QRectF(int(self.last_xy[2] - (self.scan_size[0] * self.scan_size[1] / 2)),
                                          int(self.last_xy[3] - (self.scan_size[0] * self.scan_size[1] / 2)),
                                          self.scan_size[0] * self.scan_size[1] + (xmax-xmin)/20,
                                          self.scan_size[0] * self.scan_size[1] + (ymax-ymin)/20)
            self.points.movePoint(self.points.getHandles()[0],[self.last_xy[0]+self.last_xy[2],self.last_xy[1]+self.last_xy[3]])

        elif index == -1:   # close point editor
            self.point_mode = False
            self.init_point_mode(False)
            self.init_default_mode(True)
            # self.init_rescan_mode(False)
            self.view_box.setMouseEnabled(x=True, y=True)
            self.view_box.setRange(QRectF(-3276800, -3276800, 6553600, 6553600), padding=0)
            self.view_box.setLimits(xMin=-3481600, xMax=3481600, yMin=-3481600, yMax=3481600, \
                                    minXRange=3, maxXRange=6963200, minYRange=3, maxYRange=6963200)

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
            self.points.show()
            self.select_point.show()
        else:
            self.points.hide()
            self.select_point.hide()
    
    # Save
    def save(self):
        # Set up file dialog for save
        self.dlg.setFileMode(QFileDialog.AnyFile)
        self.dlg.setAcceptMode(QFileDialog.AcceptSave)
        
        if self.data.time.strftime("%m%d%y") != self.today:             # New day, init file_index
            self.today = self.data.time.strftime("%m%d%y")
            self.file_idex = 0
        name_list = '0123456789abcdefghijklmnopqrstuvwxyz'                                      # Name list
        name = self.today + name_list[self.file_idex // 36] + name_list[self.file_idex % 36]    # Auto configure file name
        self.dlg.selectFile(self.dlg.directory().path() + '/' + name + '.stm')                  # Set default file name as auto configured
        
        if self.dlg.exec_():                                            # File selected
            fname = self.dlg.selectedFiles()[0]                                                 # File path
            directory = self.dlg.directory()                                                    # Directory path
            self.dlg.setDirectory(directory)                                                    # Set directory path for next call
            save_name = fname.replace(directory.path() + '/', '').replace('.stm', '')           # Get the real file name 
            
            # If default file name is not used
            if save_name != name:
                try:        # See if current file name is in our naming system
                    if save_name[0:6] == self.today:        # Reset file index if match
                        self.file_idex = name_list.index(save_name[6]) * 36 + name_list.index(save_name[7])
                    else:
                        self.file_idex -= 1
                except:     # Otherwise do not consume file index
                    self.file_idex -= 1
            self.saved = True                                               # Toggle saved flag
            self.setWindowTitle('Scan-' + save_name)                        # Chage window title for saving status indication
            self.file_idex += 1                                             # Consume 1 file index
            with open(fname, 'wb') as output:
                self.data.path = fname                                      # Save path
                pickle.dump(self.data, output, pickle.HIGHEST_PROTOCOL)     # Save data
    
    # Load
    def load(self):
        flag = self.saved        # If able to load flag
        if not flag:
            msg = QMessageBox.question(None, "Scan", "Imaged not saved, do you overwirte current data?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            flag = (msg == QMessageBox.Yes)
        if flag:                # Able to load
            # Set up file dialog for load
            self.dlg.setFileMode(QFileDialog.ExistingFile)
            self.dlg.setAcceptMode(QFileDialog.AcceptOpen)
            if self.dlg.exec_():        # File selected
                fname = self.dlg.selectedFiles()[0]             # File path
                directory = self.dlg.directory().path()         # Directory path
                # Load data
                with open(fname, 'rb') as input:
                    self.data = pickle.load(input)
                    self.data.path = fname                      # Change file path
                self.saved = True
                self.setWindowTitle('Scan-' + fname.replace(directory + '/', '').replace('.stm', ''))   # Chage window title for saving status indication
                    
                # Set up scroll bars
                self.scrollBar_Xin_XY.setValue(self.data.lastdac[0] - 0x8000)
                self.scrollBar_Yin_XY.setValue(self.data.lastdac[15] - 0x8000)
                self.scrollBar_Xoffset_XY.setValue(self.data.lastdac[1] - 0x8000)
                self.scrollBar_Yoffset_XY.setValue(self.data.lastdac[14] - 0x8000)
                self.scrollBar_ScanSize_ScanControl.setValue(self.data.step_num)
                self.scrollBar_StepSize_ScanControl.setValue(self.data.step_size)
                # !!! Need to plot image
                self.raw_img = copy.deepcopy(self.data.data[0])
                self.current_img = copy.deepcopy(self.raw_img)
                self.img_display.setImage(self.current_img)
                self.img_display.setRect(QRectF(int(self.current_xy[2]-(self.scan_size[0]*self.scan_size[1]/2)), int(self.current_xy[3]-(self.scan_size[0]*self.scan_size[1]/2)), self.scan_size[0] * self.scan_size[1],
                                                self.scan_size[0] * self.scan_size[1]))
                self.view_box.setRange(QRectF(int(self.current_xy[2]-(self.scan_size[0]*self.scan_size[1]/2)), int(self.current_xy[3]-(self.scan_size[0]*self.scan_size[1]/2)), self.scan_size[0] * self.scan_size[1],
                                              self.scan_size[0] * self.scan_size[1]), padding=0)
    
    # Pop out message
    def message(self, text):
        QMessageBox.warning(None, "Scan", text, QMessageBox.Ok)

    # Spctroscopy open track
    def open_track(self, state):
        if state == 0:
            self.track.closable = True
            self.track.comboBox_ReadCh_Track.setEnabled(True)
            self.track.pushButton_Start_Track.setEnabled(self.pushButton_Start_Scan.isEnabled())    # Reset enable based on succeed
            self.track.close()
        elif state == 2:
            self.track.closable = False
            self.track.pushButton_Start_Track.setEnabled(False)
            self.track.comboBox_ReadCh_Track.setEnabled(False)
            self.track.show()

    # pallet radioButton slot | gray, color
    def pallet_changed(self, index, status):
        '''Change color map for displayed image: afm-hot or gray.'''
        if status:
            self.update_display_signal.emit()

    # filter checkBox  slot | reverse, Illuminated and Plane fit
    def filter_changed(self, index, status):
        '''Process image based on checkBox signal: Reverse, Illuminated and Plane fit.'''
        if self.checkBox_Illuminated_Scan.isChecked():
            if index == 2 and status:
                self.checkBox_Illuminated_Scan.setChecked(False)
                self.checkBox_PlaneFit_Scan.setChecked(True)
            else:
                self.checkBox_Illuminated_Scan.setChecked(True)
                self.checkBox_PlaneFit_Scan.setChecked(False)

        if self.checkBox_PlaneFit_Scan.isChecked():
            if index == 1 and status:
                self.checkBox_PlaneFit_Scan.setChecked(False)
                self.checkBox_Illuminated_Scan.setChecked(True)
            else:
                self.checkBox_PlaneFit_Scan.setChecked(True)
                self.checkBox_Illuminated_Scan.setChecked(False)

        if_reverse = self.checkBox_Reverse_Scan.isChecked()
        if_illuminated = self.checkBox_Illuminated_Scan.isChecked()
        if_plane_fit = self.checkBox_PlaneFit_Scan.isChecked()

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
        if self.radioButton_Gray_Scan.isChecked():
            psudo_gray_img = cv.cvtColor(self.current_img, cv.COLOR_GRAY2BGR)
            self.color_current_img = psudo_gray_img
            self.pallet_bar.loadPreset('grey')
        elif self.radioButton_Color_Scan.isChecked():
            color_img = self.myimg.color_map(self.current_img, 36)
            self.color_current_img = color_img
            self.pallet_bar.loadPreset('thermal')
        self.img_display.setImage(self.color_current_img)

    # Point Editor | update varible and draw points
    def points_update(self, index):
        # update graphics
        if index == 0:

            # update point_list variable
            self.point_list.clear()
            for point in self.point_editor.points:
                self.point_list += [(point[0]*self.imagine_gain+self.last_xy[2], point[1]*self.imagine_gain+self.last_xy[3])]

            # draw points
            if len(self.points.getHandles()) <= len(self.point_list):
                for i in range(len(self.points.getHandles())):
                    self.points.movePoint(self.points.getHandles()[i], self.point_list[i])
                for i in range(len(self.points.getHandles()), len(self.point_list)):
                    self.points.addFreeHandle(self.point_list[i])
            elif len(self.points.getHandles()) > len(self.point_list):
                # for i in range(len(self.point_list)):
                #     self.points.movePoint(self.points.getHandles()[i], self.point_list[i])
                # for i in range(len(self.point_list), len(self.points.getHandles())):
                    # self.points.scene().removeItem(self.points.handles[i]['item'])
                    # self.points.removeHandle(self.points.handles[i]['item'])
                self.view_box.removeItem(self.points)
                self.points = pg.PolyLineROI([0, 0], closed=False, pen=self.serial_pen[0], movable=False)
                self.view_box.addItem(self.points)
                self.points.removeHandle(self.points.handles[0]['item'])
                self.points.getHandles()[0].pen.setWidth(2)
                purple_brush = pg.mkBrush('deaaff')
                self.points.getHandles()[0].pen.setBrush(purple_brush)
                self.points.sigRegionChanged.connect(lambda: self.points_update(1))
                self.points.setParentItem(self.select_point)
                self.points.movePoint(self.points.getHandles()[0], self.point_list[0])
                for i in range(1,len(self.point_list)):
                    self.points.addFreeHandle(self.point_list[i])

            # draw dashed lines
            start = -1 if self.points.closed else 0
            for i in range(start, len(self.points.handles) - 1):
                self.points.addSegment(self.points.handles[i]['item'], self.points.handles[i + 1]['item'])
            self.points.update()

        # update point editor
        elif index == 1:

            # set point limit
            xmin = int(self.last_xy[2] - (self.scan_size[0] * self.scan_size[1] / 2))
            ymin = int(self.last_xy[3] - (self.scan_size[0] * self.scan_size[1] / 2))
            xmax = xmin + self.scan_size[0] * self.scan_size[1]
            ymax = ymin + self.scan_size[0] * self.scan_size[1]
            print("range:", xmin, xmax, ymin, ymax)
            x_handles = []
            y_handles = []
            for i in range(len(self.points.getHandles())):
                x_handles += [abs(self.points.getHandles()[i].pos()[0] + self.points.pos()[0] - xmin) \
                                  if abs(self.points.getHandles()[i].pos()[0] + self.points.pos()[0] - xmin) < \
                                     abs(self.points.getHandles()[i].pos()[0] + self.points.pos()[0] - xmax) \
                                  else abs(self.points.getHandles()[i].pos()[0] + self.points.pos()[0] - xmax)]
                y_handles += [abs(self.points.getHandles()[i].pos()[1] + self.points.pos()[1] - ymin) \
                                  if abs(self.points.getHandles()[i].pos()[1] + self.points.pos()[1] - ymin) < \
                                     abs(self.points.getHandles()[i].pos()[1] + self.points.pos()[1] - ymax) \
                                  else abs(self.points.getHandles()[i].pos()[1] + self.points.pos()[1] - ymax)]
            index = x_handles.index(min(x_handles)) if min(x_handles) < min(y_handles) else y_handles.index(min(y_handles))
            x_handle = int(self.points.getHandles()[index].pos()[0] + self.points.pos()[0])
            y_handle = int(self.points.getHandles()[index].pos()[1] + self.points.pos()[1])

            print("index:", index)
            print("position:", x_handle, y_handle)

            self.point_list_others = []
            for i in range(len(self.points.handles)):
                x_other = self.points.getHandles()[i].pos()[0]
                y_other = self.points.getHandles()[i].pos()[1]
                self.point_list_others += [(x_other, y_other)]

            if (not (x_handle in range(xmin, xmax+1))) or (not (y_handle in range(ymin, ymax+1))):
                print("hhhhhhhhhhhhhhhh:")
                if abs(x_handle - self.last_xy[2]) > abs(y_handle - self.last_xy[3]):
                    x = xmax if abs(x_handle - xmax) < abs(x_handle - xmin) else xmin
                    y = int(self.points.getHandles()[index].pos()[1]+self.points.pos()[1])
                    # self.points.movePoint(self.points.getHandles()[index], [x, y])
                elif abs(x_handle - self.last_xy[2]) < abs(y_handle - self.last_xy[3]):
                    x = int(self.points.getHandles()[index].pos()[0]+self.points.pos()[0])
                    y = ymax if abs(y_handle - ymax) < abs(y_handle - ymin) else ymin
                    # self.points.movePoint(self.points.getHandles()[index], [x, y])
                else:
                    x = xmax if abs(x_handle - xmax) < abs(x_handle - xmin) else xmin
                    y = ymax if abs(y_handle - ymax) < abs(y_handle - ymin) else ymin
                    # self.points.movePoint(self.points.getHandles()[index], [x, y])

                for i in range(len(self.point_list_others)):
                    if i == index:
                        self.points.movePoint(self.points.getHandles()[index], [x, y])
                    else:
                        self.points.movePoint(self.points.getHandles()[i], [self.point_list_others[i][0], self.point_list_others[i][1]])

                self.points.update()


            # update table widget
            self.point_list_2table = []
            for handle in self.points.getHandles():
                x = int((handle.pos()[0] + self.points.pos()[0] - self.last_xy[2])/self.imagine_gain)
                y = int((handle.pos()[1] + self.points.pos()[1] - self.last_xy[3])/self.imagine_gain)
                self.point_list_2table += [(x, y)]
            self.point_editor.update_from_graphics(self.point_list_2table)

        # special case for only one point left
        elif index == 2:

            # update table widget
            self.point_list.clear()
            self.point_list += [(self.last_xy[0]+self.last_xy[2], self.last_xy[1]+self.last_xy[3])]
            self.point_editor.update_from_graphics(self.point_list)

            # draw points
            # if len(self.points.handles) == 2:
            self.view_box.removeItem(self.points)
            self.points = pg.PolyLineROI([0, 0], closed=False, pen=self.serial_pen[0], movable=False)
            self.view_box.addItem(self.points)
            self.points.removeHandle(self.points.handles[0]['item'])
            self.points.getHandles()[0].pen.setWidth(2)
            purple_brush = pg.mkBrush('deaaff')
            self.points.getHandles()[0].pen.setBrush(purple_brush)
            self.points.sigRegionChanged.connect(lambda: self.points_update(1))
            self.points.movePoint(self.points.getHandles()[0],
                                  [self.last_xy[0] + self.last_xy[2], self.last_xy[1] + self.last_xy[3]])
            # self.points.setParentItem(self.select_point)

        # overall displacement
        elif index == 3:
            # get points position from table widget
            self.point_list_overall = []
            print("row count --->", self.point_editor.table_Content.rowCount())
            if self.point_editor.table_Content.rowCount() > 0:
                for i in range(self.point_editor.table_Content.rowCount()):
                    x = int(self.point_editor.table_Content.cellWidget(i, 0).value())
                    y = int(self.point_editor.table_Content.cellWidget(i, 1).value())
                    self.point_list_overall += [(x * self.imagine_gain + self.last_xy[2], y * self.imagine_gain + self.last_xy[3])]

            if len(self.point_list_overall) > 0:
                ref_position = self.point_list_overall[0]
            for i in range(len(self.point_list_overall)):
                self.point_list_overall[i] = copy.deepcopy((self.point_list_overall[i][0] - ref_position[0], self.point_list_overall[i][1] - ref_position[1]))

            origin_position = (self.select_point.pos()[0], self.select_point.pos()[1])
            for i in range(len(self.point_list_overall)):
                self.points.movePoint(self.points.getHandles()[i], [self.point_list_overall[i][0]+origin_position[0], self.point_list_overall[i][1]+origin_position[1]])


    # Emit close signal
    def closeEvent(self, event):
        if (self.mode == 0) and (self.idling):
            # Double check exit
            msg = QMessageBox.question(None, "Scan", "Really want to exit scan?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if msg == QMessageBox.Yes:
                self.track.close()          # Close track
                self.close_signal.emit()    # Emit close signal
                event.accept()
            else:
                event.ignore()
        else:
            QMessageBox.warning(None, "Scan", "Process ongoing!!", QMessageBox.Ok)
            event.ignore()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myScan()
    dsp = myDSP()
    dsp.succeed = True
    window.init_scan(dsp, False, 9)
    window.show()
    sys.exit(app.exec_())

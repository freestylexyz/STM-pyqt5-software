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
        # Track signal
        self.track.pushButton_PlaneFit.clicked.connect(self.track_update_fit)               # Track want to use current image plane fit
        
        # Spectroscopy signal
        self.spc.pushButton_EditPoints.clicked.connect(self.edit_points)                    # Edit points mode
        self.spc.pushButton_LockIn_General.clicked.connect(self.lockin.show)                # Spectroscopy want open lock in window
        self.spc.adv.pushButton_SelectPattern_Rescan.clicked.connect(self.select_pattern)   # Select pattern for rescan
        self.spc.adv.checkBox_Tracking_Correction.stateChanged.connect(self.open_track)     # Spectroscopy want to track

        # PushButton | open windows
        self.pushButton_LockIn_ScanControl.clicked.connect(self.lockin.show)                # Open lock in window
        self.pushButton_ScanOptions_Scan.clicked.connect(self.scan_options.show)            # Open scan option window
        self.pushButton_SendOptions_Scan.clicked.connect(self.send_options.show)            # Open send option window
        self.pushButton_Info_Scan.clicked.connect(self.open_info)                           # Open scan data info window
        
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
        plot_data = self.data.updata_data(rdata)        # Update scan data and obtain data used for plot
        # !!! Update graphic view
        self.img_display.setImage(plot_data)
        self.img_display.setRect(QRectF(self.last_xy[2], self.last_xy[3], self.scan_size[0]*self.scan_size[1], self.scan_size[0]*self.scan_size[1]))
        self.view_box.setRange(QRectF(self.last_xy[2], self.last_xy[3], self.scan_size[0]*self.scan_size[1], self.scan_size[0]*self.scan_size[1]), padding=0)
        self.raw_img = plot_data
        
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
        print('last', self.last_xy)
        print('current', self.current_xy)
        print('size', self.scan_size)
        self.target_position.movePoint(self.target_position.getHandles()[0], \
                                       [self.current_xy[0] + self.last_xy[2], self.current_xy[1] + self.last_xy[3]])

    # !!!
    # Select pattern mode
    def select_pattern(self):
        pass
    
    # !!!
    # Edit points mode
    def edit_points(self):
        self.point_editor.show()
    
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
                self.img_display.setRect(QRectF(self.current_xy[2], self.current_xy[3], self.scan_size[0] * self.scan_size[1],
                                                self.scan_size[0] * self.scan_size[1]))
                self.view_box.setRange(QRectF(self.current_xy[2], self.current_xy[3], self.scan_size[0] * self.scan_size[1],
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

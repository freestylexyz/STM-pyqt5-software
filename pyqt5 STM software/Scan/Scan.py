# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:18:34 2020
@author: yaoji
"""

import sys
sys.path.append("../Model/")
sys.path.append("../Scan/")

from PyQt5.QtWidgets import QApplication, QDesktopWidget, QMessageBox, QButtonGroup, QFileDialog, QUndoStack, QUndoCommand
from PyQt5 import QtGui
from PyQt5.QtCore import pyqtSignal, QRectF

from Scan_ import myScan_
from DigitalSignalProcessor import myDSP
from customGradientWidget import *
import pickle
import pyqtgraph as pg
from images import myImages
import cv2 as cv
import copy

class myScan(myScan_):
    close_signal = pyqtSignal()                                     # Close scan window signal
    seq_list_signal = pyqtSignal(str)                               # Open sequence list window signal
    open_track_signal = pyqtSignal(int)                             # Open Track window signal
    open_spc_signal = pyqtSignal(int)                               # Open Spectroscopy window signal
    open_dep_signal = pyqtSignal(int)                               # Open Deposition window signal
    point_editor_signal = pyqtSignal(bool)                          # Point editing mode signal
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
        # self.pushButton_Info_Scan.clicked.connect(self.open_info)                         # Open scan data info window
        # self.pushButton_Info_Scan.setEnabled(False)
        self.pushButton_LockIn_ScanControl.setEnabled(False)
        self.pushButton_Info_Scan.clicked.connect(lambda: self.edit_points(0))
        self.pushButton_SeqList_ScanControl.clicked.connect(self.open_seq_list)
        self.pushButton_Track.clicked.connect(self.open_track_win)
        self.pushButton_Deposition.clicked.connect(self.open_dep_win)
        self.pushButton_Spectrosocpy.clicked.connect(self.open_spc_win)

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

        self.undoStack = QUndoStack(self)

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
        self.dep.bias_ran_change(ran)   # Change related parts in deposition
        self.spc.bias_ran_change(ran)   # Change related parts in spectroscopy

    # Update ramp diagonal
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
        
    # Update plane fit parameters in track window
    def track_update_fit(self):
        self.track.spinBox_X_PlaneFit.setValue(self.tilt[0])    # Sync tilt X
        self.track.spinBox_Y_PlaneFit.setValue(self.tilt[1])    # Sync tilt y

    # Track update function
    def track_update(self, x, y):
        self.send_update(0x10, 0x1f, x, y)                                              # Update tip position
        # If out of track size
        stop_flag = (abs(self.track.x - x) > self.track.track_size)                     # X out of boundary
        stop_flag = stop_flag or (abs(self.track.y - y) > self.track.track_size)        # Y out of boundary
        if stop_flag and (not self.stop):       # Either X or Y out of boundary, avoid sending stop signal twice
            self.track.pushButton_Start_Track.setEnable(False)                          # Disable stop button to avoid sending stop signal twice
            self.stop_signal.emit()                                                     # Emit stop signal

    # Open scan information window
    def open_info(self):
        self.info.init_scanInfo(self.data)
        self.info.show()

    # Open sequence list window
    def open_seq_list(self):
        self.seq_list_signal.emit(self.data.seq.name)

    # Open Track window
    def open_track_win(self):
        self.open_track_signal.emit(-1)

    # Open Spectroscopy window
    def open_spc_win(self):
        self.trackVisible_ = self.track.isVisible()
        if self.trackVisible_:
            self.track.hide()
        self.open_spc_signal.emit(1)

    # Open Deposition window
    def open_dep_win(self):
        self.open_dep_signal.emit(2)

    # !!! Select pattern mode
    def select_pattern(self):
        pass

    # Edit points mode
    def edit_points(self, index):
        if index == 0:      # open point editor
            self.point_mode = True
            # Init Point Editor size and position
            screen = QDesktopWidget().screenGeometry()
            spacerHor = int(screen.height() * 0.01)
            sizeScan = self.frameGeometry()
            self.point_editor.resize(424, 320)
            self.point_editor.move(self.x() + sizeScan.width() + spacerHor, self.y())
            self.point_editor.show()
            self.init_point_mode(True)
            self.init_default_mode(False)
            # self.init_rescan_mode(False)
            # Set up viewBox in Scan window
            self.view_box.setMouseEnabled(x=False, y=False)
            xmin = int(self.last_xy[2] - (self.scan_size[0] * self.scan_size[1] / 2))
            ymin = int(self.last_xy[3] - (self.scan_size[0] * self.scan_size[1] / 2))
            xmax = xmin + self.scan_size[0] * self.scan_size[1]
            ymax = ymin + self.scan_size[0] * self.scan_size[1]
            self.view_box.setLimits(xMin=xmin, xMax=xmax, yMin=ymin, yMax=ymax, minXRange=3, maxXRange=xmax - xmin,
                                    minYRange=3, maxYRange=ymax - ymin)
            self.view_box.setRange(QRectF(xmin, xmax, self.scan_size[0] * self.scan_size[1], self.scan_size[0] * self.scan_size[1]), padding=0)
            # Set up Points related ROI
            self.select_point.setSize([(xmax-xmin)/20, (ymax-ymin)/20])
            # self.select_point.setPos([int((xmax + xmin)/2-(xmax-xmin)/40), int((ymax + ymin)/2-(ymax-ymin)/40)])
            self.select_point.setPos([0,0])
            self.select_point.maxBounds = QRectF(int(self.last_xy[2] - (self.scan_size[0] * self.scan_size[1] / 2)),
                                          int(self.last_xy[3] - (self.scan_size[0] * self.scan_size[1] / 2)),
                                          self.scan_size[0] * self.scan_size[1] ,
                                          self.scan_size[0] * self.scan_size[1] )
            self.points.movePoint(self.points.getHandles()[0],[self.last_xy[0]+self.last_xy[2],self.last_xy[1]+self.last_xy[3]])
            # Disable all widgets in Scan window except the viewBox
            self.enable_point(False)
            # Remember window states
            self.spcVisible = self.spc.isVisible()
            self.trackVisible = self.track.isVisible()
            # Activate Scan window and minimize other windows
            self.raise_()
            self.spc.hide()
            self.track.hide()
            self.point_editor_signal.emit(True)

        elif index == -1:   # close point editor
            self.point_mode = False
            self.init_point_mode(False)
            self.init_default_mode(True)
            # self.init_rescan_mode(False)
            self.view_box.setMouseEnabled(x=True, y=True)
            self.view_box.setRange(QRectF(-3276800, -3276800, 6553600, 6553600), padding=0)
            self.view_box.setLimits(xMin=-3481600, xMax=3481600, yMin=-3481600, yMax=3481600, \
                                    minXRange=3, maxXRange=6963200, minYRange=3, maxYRange=6963200)
            # Enable all widgets in Scan window except the viewBox
            self.enable_point(True)
            # Activate windows
            if self.trackVisible:
                self.track.showNormal()
            if self.spcVisible:
                self.spc.showNormal()
            self.point_editor_signal.emit(False)

    # Control ROIs visibility in default mode
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

    # Control ROIs visibility in point mode
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
            self.file_index = 0
        name_list = '0123456789abcdefghijklmnopqrstuvwxyz'                                      # Name list
        name = self.today + name_list[self.file_index // 36] + name_list[self.file_index % 36]    # Auto configure file name
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
                        self.file_index = name_list.index(save_name[6]) * 36 + name_list.index(save_name[7])
                    else:
                        self.file_index -= 1
                except:     # Otherwise do not consume file index
                    self.file_index -= 1
            self.saved = True                                               # Toggle saved flag
            self.setWindowTitle('Scan-' + save_name)                        # Change window title for saving status indication
            self.file_index += 1                                            # Consume 1 file index
            with open(fname, 'wb') as output:
                self.data.path = fname                                      # Save path
                pickle.dump(self.data, output, pickle.HIGHEST_PROTOCOL)     # Save data
    
    # Load
    def load(self):
        flag = self.saved        # If able to load flag
        if not flag:
            msg = QMessageBox.question(None, "Scan", "Imaged not saved, do you overwrite current data?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            flag = (msg == QMessageBox.Yes)
        if flag:                # Able to load
            # Set up file dialog for load
            self.dlg.setFileMode(QFileDialog.ExistingFile)
            self.dlg.setAcceptMode(QFileDialog.AcceptOpen)
            if self.remember_path != '':
                self.dlg.setDirectory(self.remember_path)
            if self.dlg.exec_():        # File selected
                fname = self.dlg.selectedFiles()[0]             # File path
                directory = self.dlg.directory().path()         # Directory path
                # Load data
                with open(fname, 'rb') as input:
                    self.data = pickle.load(input)
                    self.data.path = fname                      # Change file path
                self.saved = True
                self.remember_path = directory
                self.setWindowTitle('Scan-' + fname.replace(directory + '/', '').replace('.stm', ''))   # Change window title for saving status indication
                    
                # Set up scroll bars
                self.scrollBar_Xin_XY.setValue(self.data.lastdac[0] - 0x8000)
                self.scrollBar_Yin_XY.setValue(self.data.lastdac[15] - 0x8000)
                self.scrollBar_Xoffset_XY.setValue(self.data.lastdac[1] - 0x8000)
                self.scrollBar_Yoffset_XY.setValue(self.data.lastdac[14] - 0x8000)
                self.scrollBar_ScanSize_ScanControl.setValue(self.data.step_num)
                self.scrollBar_StepSize_ScanControl.setValue(self.data.step_size)
                # Plot image
                self.raw_img = copy.deepcopy(self.data.data[0])
                self.current_img = copy.deepcopy(self.raw_img)
                self.img_display.setImage(self.current_img)
                gain_dict = {8: 1, 9: 10, 10: 100}
                gain = gain_dict[self.data.preamp_gain]
                self.img_display.setRect(QRectF(
                    int(self.current_xy[2] - (self.scan_size[0] * self.scan_size[1] / 2) * (gain / self.imagine_gain)),
                    int(self.current_xy[3] - (self.scan_size[0] * self.scan_size[1] / 2) * (gain / self.imagine_gain)),
                    self.scan_size[0] * self.scan_size[1] * (gain / self.imagine_gain),
                    self.scan_size[0] * self.scan_size[1] * (gain / self.imagine_gain)))
                self.view_box.setRange(QRectF(
                    int(self.current_xy[2] - (self.scan_size[0] * self.scan_size[1] / 2) * (gain / self.imagine_gain)),
                    int(self.current_xy[3] - (self.scan_size[0] * self.scan_size[1] / 2) * (gain / self.imagine_gain)),
                    self.scan_size[0] * self.scan_size[1] * (gain / self.imagine_gain),
                    self.scan_size[0] * self.scan_size[1] * (gain / self.imagine_gain)), padding=0)

                self.pushButton_Info_Scan.setEnabled(True)
                # !!! if self.data.lockin_flag:
                if True:
                    self.pushButton_LockIn_ScanControl.setEnabled(True)
    
    # Pop out message
    def message(self, text):
        QMessageBox.warning(None, "Scan", text, QMessageBox.Ok)

    # Spectroscopy open track
    def open_track(self, state):
        if state == 0:  # if spc adv checkBox is unchecked
            self.track.closable = True
            self.track.comboBox_ReadCh_Track.setEnabled(True)
            self.track.pushButton_Start_Track.setEnabled(self.pushButton_Start_Scan.isEnabled())    # Reset enable based on succeed
            self.track.close()  # !!! not working
        elif state == 2:    # if spc adv checkBox is checked (0: unchecked, 1: partially checked, 2: checked)
            self.track.closable = False
            self.track.pushButton_Start_Track.setEnabled(False)
            self.track.comboBox_ReadCh_Track.setEnabled(False)
            # Init Track size and position
            self.track.resize(472, 274)
            self.track.move(11, 757)
            self.track.show()

    # pallet radioButton slot | gray, color
    def pallet_changed(self, index, status):
        '''Change color map for displayed image: afm-hot or gray.'''
        if status:
            self.update_display_signal.emit()

    # filter checkBox  slot | reverse, Illuminated and Plane fit
    def filter_changed(self, index, status):
        '''Process image based on checkBox signal: Reverse, Illuminated and Plane fit.'''
        if len(self.raw_img) != 0:
            # Set mutual exclusion of Illuminated and Plane fit
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

            # CheckBox status variable
            if_reverse = self.checkBox_Reverse_Scan.isChecked()
            if_illuminated = self.checkBox_Illuminated_Scan.isChecked()
            if_plane_fit = self.checkBox_PlaneFit_Scan.isChecked()

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
        if len(self.current_img) != 0:      # For real image display
            if self.radioButton_Gray_Scan.isChecked():
                psudo_gray_img = cv.cvtColor(self.current_img, cv.COLOR_GRAY2BGR)
                self.color_current_img = psudo_gray_img
                if self.checkBox_Reverse_Scan.isChecked():
                    self.pallet_bar.loadPreset('reverse_grey')
                else:
                    self.pallet_bar.loadPreset('grey')
            elif self.radioButton_Color_Scan.isChecked():
                color_img = self.myimg.color_map(self.current_img, 36)
                self.color_current_img = color_img
                if self.checkBox_Reverse_Scan.isChecked():
                    self.pallet_bar.loadPreset('reverse_thermal')
                else:
                    self.pallet_bar.loadPreset('thermal')
            self.img_display.setImage(self.color_current_img)
        else:                              # For no image display
            if self.radioButton_Gray_Scan.isChecked():
                if self.checkBox_Reverse_Scan.isChecked():
                    self.pallet_bar.loadPreset('reverse_grey')
                else:
                    self.pallet_bar.loadPreset('grey')
            elif self.radioButton_Color_Scan.isChecked():
                if self.checkBox_Reverse_Scan.isChecked():
                    self.pallet_bar.loadPreset('reverse_thermal')
                else:
                    self.pallet_bar.loadPreset('thermal')


    # Point Editor | update variable and draw points
    def points_update(self, index):
        # Update graphics
        if index == 0:

            # Update point_list variable
            self.point_list.clear()
            for point in self.point_editor.points:
                self.point_list += [(point[0]*self.imagine_gain+self.last_xy[2], point[1]*self.imagine_gain+self.last_xy[3])]

            # Draw points
            if len(self.points.getHandles()) <= len(self.point_list):
                for i in range(len(self.points.getHandles())):
                    self.points.movePoint(self.points.getHandles()[i], self.point_list[i])
                for i in range(len(self.points.getHandles()), len(self.point_list)):
                    self.points.addFreeHandle(self.point_list[i])
            elif len(self.points.getHandles()) > len(self.point_list):
                self.view_box.removeItem(self.points)
                self.points = pg.PolyLineROI([0, 0], closed=False, pen=self.serial_pen[0], movable=False)
                self.view_box.addItem(self.points)
                self.points.removeHandle(self.points.handles[0]['item'])
                self.points.getHandles()[0].pen.setWidth(2)
                purple_brush = pg.mkBrush('deaaff')
                self.points.getHandles()[0].pen.setBrush(purple_brush)
                # self.points.sigRegionChanged.connect(lambda: self.points_update(1))
                self.points.sigRegionChanged.connect(self.update_point_editor)
                self.points.movePoint(self.points.getHandles()[0], self.point_list[0])
                for i in range(1, len(self.point_list)):
                    self.points.addFreeHandle(self.point_list[i])

            # Draw dashed lines
            start = -1 if self.points.closed else 0
            for i in range(start, len(self.points.handles) - 1):
                self.points.addSegment(self.points.handles[i]['item'], self.points.handles[i + 1]['item'])
            self.points.update()

        # !!! Replaced with update_point_editor()
        # Update point editor
        elif index == 1:

            # Record other points position
            self.point_list_others = []
            x_others = []
            y_others = []
            for i in range(len(self.points.handles)):
                x_other = self.points.getHandles()[i].pos()[0]
                y_other = self.points.getHandles()[i].pos()[1]
                self.point_list_others += [(x_other, y_other)]
                x_others += [x_other]
                y_others += [y_other]

            # Set up size of select point roi
            index_xmin = x_others.index(min(x_others))
            index_ymin = y_others.index(min(y_others))
            index_xmax = x_others.index(max(x_others))
            index_ymax = y_others.index(max(y_others))
            self.select_point.sigRegionChanged.disconnect(self.points_overall)
            self.select_point.setPos([self.point_list_others[index_xmin][0], self.point_list_others[index_ymin][1]])
            self.select_point.setSize([self.point_list_others[index_xmax][0] - self.point_list_others[index_xmin][0], \
                                       self.point_list_others[index_ymax][1] - self.point_list_others[index_ymin][1]])
            self.select_point.sigRegionChanged.connect(self.points_overall)

            # Update table widget
            self.point_list_2table = []
            for handle in self.points.getHandles():
                x = int((handle.pos()[0] + self.points.pos()[0] - self.last_xy[2])/self.imagine_gain)
                y = int((handle.pos()[1] + self.points.pos()[1] - self.last_xy[3])/self.imagine_gain)
                self.point_list_2table += [(x, y)]
            self.point_editor.update_from_graphics(self.point_list_2table)

        # Special case for only one point left
        elif index == 2:

            # Update table widget
            self.point_list.clear()
            self.point_list += [(self.last_xy[0]+self.last_xy[2], self.last_xy[1]+self.last_xy[3])]
            self.point_editor.update_from_graphics(self.point_list)

            # Draw points
            self.view_box.removeItem(self.points)
            self.points = pg.PolyLineROI([0, 0], closed=False, pen=self.serial_pen[0], movable=False)
            self.view_box.addItem(self.points)
            self.points.removeHandle(self.points.handles[0]['item'])
            self.points.getHandles()[0].pen.setWidth(2)
            purple_brush = pg.mkBrush('deaaff')
            self.points.getHandles()[0].pen.setBrush(purple_brush)
            # self.points.sigRegionChanged.connect(lambda: self.points_update(1))
            self.points.sigRegionChanged.connect(self.update_point_editor)
            self.points.movePoint(self.points.getHandles()[0],
                                  [self.last_xy[0] + self.last_xy[2], self.last_xy[1] + self.last_xy[3]])

    # points_update index == 1
    def update_point_editor(self):
        # Record other points position
        self.point_list_others = []
        x_others = []
        y_others = []
        for i in range(len(self.points.handles)):
            x_other = self.points.getHandles()[i].pos()[0]
            y_other = self.points.getHandles()[i].pos()[1]
            self.point_list_others += [(x_other, y_other)]
            x_others += [x_other]
            y_others += [y_other]

        # Set up size of select point roi
        index_xmin = x_others.index(min(x_others))
        index_ymin = y_others.index(min(y_others))
        index_xmax = x_others.index(max(x_others))
        index_ymax = y_others.index(max(y_others))
        self.select_point.sigRegionChanged.disconnect(self.points_overall)
        self.select_point.setPos([self.point_list_others[index_xmin][0], self.point_list_others[index_ymin][1]])
        self.select_point.setSize([self.point_list_others[index_xmax][0] - self.point_list_others[index_xmin][0], \
                                   self.point_list_others[index_ymax][1] - self.point_list_others[index_ymin][1]])
        self.select_point.sigRegionChanged.connect(self.points_overall)

        # Update table widget
        self.point_list_2table = []
        for handle in self.points.getHandles():
            x = int((handle.pos()[0] + self.points.pos()[0] - self.last_xy[2]) / self.imagine_gain)
            y = int((handle.pos()[1] + self.points.pos()[1] - self.last_xy[3]) / self.imagine_gain)
            self.point_list_2table += [(x, y)]
        self.point_editor.update_from_graphics(self.point_list_2table)

    # Point Editor | overall displacement of points
    def points_overall(self):
        # Get points position
        self.point_list_overall = []
        x_overall = []
        y_overall = []
        for i in range(len(self.points.handles)):
            x = self.points.getHandles()[i].pos()[0]
            y = self.points.getHandles()[i].pos()[1]
            self.point_list_overall += [(x, y)]
            x_overall += [x]
            y_overall += [y]

        # Get border point position
        index_xmin = x_overall.index(min(x_overall))
        index_ymin = y_overall.index(min(y_overall))
        index_xmax = x_overall.index(max(x_overall))
        index_ymax = y_overall.index(max(y_overall))
        center_x = int((x_overall[index_xmax] + x_overall[index_xmin])/2)
        center_y = int((y_overall[index_ymax] + y_overall[index_ymin])/2)

        # Move points
        if len(self.point_list_overall) > 0:
            ref_position = (center_x, center_y)     # Set ref_position to center of all points
        for i in range(len(self.point_list_overall)):
            self.point_list_overall[i] = copy.deepcopy(
                (self.point_list_overall[i][0] - ref_position[0], self.point_list_overall[i][1] - ref_position[1]))
        origin_position = (int(self.select_point.pos()[0]+self.select_point.size()[0]/2), \
                          int(self.select_point.pos()[1]+self.select_point.size()[1]/2))    # Set origin_position to square center
        for i in range(len(self.point_list_overall)):
            self.points.movePoint(self.points.getHandles()[i], [self.point_list_overall[i][0] + origin_position[0],
                                                                self.point_list_overall[i][1] + origin_position[1]])

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

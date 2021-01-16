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

from Scan_ import myScan_
from DigitalSignalProcessor import myDSP

import conversion as cnv
import pickle

class myScan(myScan_):
    close_signal = pyqtSignal()
    seq_list_signal = pyqtSignal(str)
    gain_changed_signal = pyqtSignal(int, int)
    stop_signal = pyqtSignal()
    send_signal = pyqtSignal(int, int, int, int, int, int)
    scan_signal = pyqtSignal(int, int, int, int, int, int, int)

    def __init__(self):
        super().__init__()
        self.setup_UI()

    def setup_UI(self):      
        # Track signal
        self.track.pushButton_PlaneFit.clicked.connect(self.track_update_fit)
        
        # Spectroscopy signal
        self.spc.pushButton_EditPoints.clicked.connect(self.edit_points)
        self.spc.pushButton_LockIn_General.clicked.connect(self.lockin.show)
        self.spc.adv.pushButton_SelectPattern_Rescan.clicked.connect(self.select_pattern)
        self.spc.adv.checkBox_Tracking_Correction.stateChanged.connect(self.open_track)

        # PushButton | open windows
        self.pushButton_LockIn_ScanControl.clicked.connect(self.lockin.show)
        self.pushButton_ScanOptions_Scan.clicked.connect(self.scan_options.show)
        self.pushButton_SendOptions_Scan.clicked.connect(self.send_options.show)
        self.pushButton_Info_Scan.clicked.connect(self.open_info)
        
        # PushButton | file
        self.pushButton_SaveAll_Scan.clicked.connect(self.save)
        self.pushButton_Load_Scan.clicked.connect(self.load)

        # radioButton | X/Y gain
        self.XY_gain_group.buttonToggled[int, bool].connect(self.gain_changed_emit)

        # pushButton | send and zero
        self.pushButton_Send_XY.clicked.connect(lambda: self.send_emit(1))
        self.pushButton_Zero_XY.clicked.connect(lambda: self.send_emit(0))
        
        # pushButton | scan
        self.pushButton_Start_Scan.clicked.connect(self.scan_emit)


    # if X/Y gain is changed by user, emit signal
    def gain_changed_emit(self, gain, status):
        if status:
            self.gain_changed_signal.emit(0, gain)
            self.gain_changed_signal.emit(1, gain)

    # Emit send signal
    def send_emit(self, index):
        if self.idling:
            if not index:
                self.scrollBar_Xin_XY.setValue(0)
                self.scrollBar_Yin_XY.setValue(0)
            xoff = self.scrollBar_Xoffset_XY.value() + 0x8000
            yoff = self.scrollBar_Yoffset_XY.value() + 0x8000
            xin = self.scrollBar_Xin_XY.value() + 0x8000
            yin = self.scrollBar_Yin_XY.value() + 0x8000
            self.send_signal.emit(index, xin, yin, xoff, yoff, self.imagine_gain)
        else:
            self.pushButton_Zero_XY.setEnabled(False)
            self.pushButton_Send_XY.setEnabled(False)
            self.stop_signal.emit()
            
    # Emit scan signal
    def scan_emit(self):
        flag = False
        if self.saved:
            flag = True
        elif self.idling:
            msg = QMessageBox.question(None, "Scan", "Imaged not saved, do you want to continue?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            flag = (msg == QMessageBox.Yes)
        if self.idling and flag:
            xoff = self.scrollBar_Xoffset_XY.value() + 0x8000
            yoff = self.scrollBar_Yoffset_XY.value() + 0x8000
            xin = self.scrollBar_Xin_XY.value() + 0x8000
            yin = self.scrollBar_Yin_XY.value() + 0x8000
            step_size = self.scrollBar_StepSize_ScanControl.value()
            step_num = self.scrollBar_ScanSize_ScanControl.value()
            self.scan_signal.emit(xin, yin, xoff, yoff, self.imagine_gain, step_num, step_size)
        elif not self.idling:
            self.pushButton_Start_Scan.setEnabled(False)
            self.stop_signal.emit()

    # Bias range change slot
    def bias_ran_change(self, ran):
        self.dep.bias_ran_change(ran)
        self.spc.bias_ran_change(ran)

    # Update ramp diagnal
    def send_update(self, channels, channell, currents, currentl):
        func_dict = {0x10: self.label_Xin_XY, 0x1f: self.label_Yin_XY, 0x11: self.label_Xoffset_XY, 0x1e: self.label_Yoffset_XY}
        var_dict = {0x10: 0, 0x1f: 1, 0x11: 2, 0x1e: 3}
        gain_dict = {0x10: self.imagine_gain, 0x1f: self.imagine_gain, 0x11: 100, 0x1e: 100}
        
        func_dict[channels].setText(str(currents - 0x8000))
        func_dict[channell].setText(str(currentl - 0x8000))
        self.last_xy[var_dict[channels]] = (currents - 0x8000) * gain_dict[channels]
        self.last_xy[var_dict[channell]] = (currentl - 0x8000) * gain_dict[channell]
        # !!! update graphic view
            
    # Update scan
    def scan_update(self, rdata):
        self.data.updata_data(rdata)
        # !!! Update graphic view
        
    # Update plane fit paramenters in track window
    def track_update_fit(self):
        self.track.spinBox_X_PlaneFit.setValue(self.tilt[0])
        self.track.spinBox_Y_PlaneFit.setValue(self.tilt[1])

    # Track update function
    def track_update(self, x, y):
        self.send_update(0x10, 0x1f, x, y)
        # If out of track size
        stop_flag = False or (abs(self.track.x - x) > self.track.track_size)
        stop_flag = stop_flag or (abs(self.track.y - y) > self.track.track_size)
        if stop_flag and (not self.stop):
            self.track.pushButton_Start_Track.setEnable(False)
            self.stop_signal.emit()

    # !!!
    # Open scan information windwow
    def open_info(self):
        # self.info.init_info()
        self.info.show()

    # !!!
    # Select pattern mode
    def select_pattern(self):
        pass
    
    # !!!
    # Edit points mode
    def edit_points(self):
        pass
    
    # Save
    def save(self):
        self.dlg.setFileMode(QFileDialog.AnyFile)
        self.dlg.setAcceptMode(QFileDialog.AcceptSave)
        if self.data.time.strftime("%m%d%y") != self.today:
            self.today = self.data.time.strftime("%m%d%y")
            self.file_idex = [0, 0]
        fname = ''
        name_list = '0123456789abcdefghijklmnopqrstuvwxyz'
        name = self.today + name_list[self.file_idex[0]] + name_list[self.file_idex[1]]
        name = self.dlg.directory().path() + '/' + name + '.stm'
        self.dlg.selectFile(name)
        if self.dlg.exec_():
            fname = self.dlg.selectedFiles()[0]
            directory = self.dlg.directory()
            self.dlg.setDirectory(directory)

        if fname != '':                         # Savable
            with open(fname, 'wb') as output:
                self.data.path = fname                                          # Save path
                pickle.dump(self.data, output, pickle.HIGHEST_PROTOCOL)         # Save data
                self.saved = True
                self.setWindowTitle('Scan-' + fname.replace(directory.path() + '/', ''))
                self.file_idex[1] += 1
                if self.file_idex[1] > 35:
                    self.file_idex[0] += 1
                    self.file_idex[1] = 0
    
    # Load
    def load(self):
        flag = False
        if self.saved:
            flag = True
        elif self.idling:
            msg = QMessageBox.question(None, "Scan", "Imaged not saved, do you want to continue?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            flag = (msg == QMessageBox.Yes)
        if flag:
            fname = ''
            self.dlg.setFileMode(QFileDialog.ExistingFile)
            self.dlg.setAcceptMode(QFileDialog.AcceptOpen)
            if self.dlg.exec_():
                fname = self.dlg.selectedFiles()[0]
                directory = self.dlg.directory()

            if fname != '':                         # Savable
                with open(fname, 'rb') as input:
                    self.data = pickle.load(input)
                    self.data.path = fname
                    self.saved = True
                    self.setWindowTitle('Scan-' + fname.replace(directory.path() + '/', ''))
                    
                    # Set up view
                    self.scrollBar_Xin_XY.setvalue(self.data.lastdac[0] - 0x8000)
                    self.scrollBar_Yin_XY.setvalue(self.data.lastdac[15] - 0x8000)
                    self.scrollBar_Xoffset_XY.setvalue(self.data.lastdac[1] - 0x8000)
                    self.scrollBar_Yoffset_XY.setvalue(self.data.lastdac[14] - 0x8000)
                    self.scrollBar_ScanSize_ScanControl.setvalue(self.data.step_num)
                    self.scrollBar_StepSize_ScanControl.setvalue(self.data.step_size)
    
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

    # Emit close signal
    def closeEvent(self, event):
        if (self.mode == 0) and (self.idling):
            msg = QMessageBox.question(None, "Scan", "Really want to exit scan?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if msg == QMessageBox.Yes:
                self.track.close()
                self.close_signal.emit()
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

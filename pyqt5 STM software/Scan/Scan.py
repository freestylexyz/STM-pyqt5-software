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

from Scan_ import myScan_
from DigitalSignalProcessor import myDSP

import conversion as cnv

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
        # Sequence
        if self.scan_seq_selected < 0:
            flag = False
            QMessageBox.warning(None, "Scan", "No sequence selected!", QMessageBox.Ok)
        else:
            seq = self.scan_seq_list[self.scan_seq_selected]
            seq.validation(seq.ditherB, seq.ditherZ, seq.feedback, True)
            flag = seq.validated or (not seq.validation_required)
            if flag:
                QMessageBox.warning(None, "Scan", "Selected sequence is not valid!", QMessageBox.Ok)
            seq.build()

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
        
        # Scan basic
        # option_list = self.scan_options.configure_scan(seq.feedback)
        
        # def scan(self, channel_x, channel_y, step_size, step_num, move_delay, measure_delay, line_delay, \
        #      limit, tip_protect_data, seq, scan_protect_flag, tip_protection, dir_x):
        #     pass

    # Bias range change slot
    def bias_ran_change(self, ran):
        self.dep.bias_ran_change(ran)
        self.spc.bias_ran_change(ran)

    # !!!
    # Update ramp diagnal
    def send_update(self, channels, channell, currents, currentl):
        pass
    
    # !!!
    # Update scan
    def scan_update(self, rdata):
        pass
    # !!!
    # Update plane fit paramenters in track window
    def track_update_fit(self):
        self.track.spinBox_X_PlaneFit.setValue(self.tilt[0])
        self.track.spinBox_Y_PlaneFit.setValue(self.tilt[1])

    # !!! Track update function
    def track_update(self, x, y):
        # If out of track size
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

    # Spctroscopy open track
    def open_track(self, state):
        if state == 0:
            self.track.closable = True
            self.track.pushButton_Start_Track.setEnabled(False)
            self.track.close()
        elif state == 2:
            self.track.closable = False
            self.track.pushButton_Start_Track.setEnabled(self.pushButton_Start_Scan.isEnabled())    # Reset enable based on succeed
            self.track.show()

    # Emit close signal
    def closeEvent(self, event):
        if (self.mode == 0) and (self.idling):
            msg = QMessageBox.information(None, "Scan", "Really want to exit scan?", QMessageBox.Yes | QMessageBox.No)
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

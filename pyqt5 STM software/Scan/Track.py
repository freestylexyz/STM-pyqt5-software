# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:20:29 2020

@author: yaoji
"""

import sys
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication , QWidget, QMessageBox
from PyQt5.QtCore import pyqtSignal , Qt
from Track_ui import Ui_Track


class myTrack(QWidget, Ui_Track):
    track_signal = pyqtSignal(list)
    stop_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.tilt = [0.0, 0.0]          # Tilt x, tilt y
        self.track_size = 65535
        self.x = 0
        self.y = 0
        self.idling = True
        self.closable = True
        
        self.spinBox_TrackSize_Track.valueChanged.connect(self.scrollBar_TrackSize_Track.setValue)
        self.scrollBar_TrackSize_Track.valueChanged.connect(self.spinBox_TrackSize_Track.setValue)
        self.spinBox_StepSize_Track.valueChanged.connect(self.scrollBar_StepSize_Track.setValue)
        self.scrollBar_StepSize_Track.valueChanged.connect(self.spinBox_StepSize_Track.setValue)
        self.pushButton_Start_Track.clicked.connect(self.track)
    
    def init_track(self, succeed):
        self.pushButton_Start_Track.setEnabled(succeed)
    
    # Update track size and step size limit
    def update_limit(self, scan_size):
        self.spinBox_TrackSize_Track.setMaximum(scan_size[0] * scan_size[1])
        self.spinBox_StepSize_Track.setMaximum(scan_size[1])
    
    # Track emit function
    def track(self):
        if self.idling:
            self.idling = False
            self.track_size = int(self.scrollBar_TrackSize_Track.value() / 2)
            step = self.scrollBar_StepSize_Track.value()
            in_ch = (self.comboBox_ReadCh_Track.currentIndex() + 6) * 4 + 0xc0
            average = self.spinBox_Avg_Track.value()
            delay = self.spinBox_ScanDelay_Track.value()
            stay_delay = self.spinBox_StayDelay_Track.value()
            track_min = self.radioButton_Min_PlaneFit.isChecked()
            if self.groupBox_PlaneFit_Track.isChecked():
                tiltx = self.spinBox_X_PlaneFit.value()
                tilty = self.spinBox_Y_PlaneFit.value()
            else:
                tiltx = 0.0
                tilty = 0.0
            self.track_signal.emit([in_ch, delay, stay_delay, step, average, track_min, tiltx, tilty])
        else:
            self.pushButton_Start_Track.setEnable(False)
            self.stop_signal.emit()
    
    # Pop out message
    def message(self, text):
        QMessageBox.warning(None, "Track", text, QMessageBox.Ok)
    
    # Emit close signal
    def closeEvent(self, event):
        if self.idling and  self.closable:
            event.accept()
        else:
            self.message('Track on going')
            event.ignore()
        
    def enable_serial(self, enable):
        self.pushButton_Start_Track.setEnabled(enable)
        self.radioButton_Max_PlaneFit.setEnabled(enable)
        self.radioButton_Min_PlaneFit.setEnabled(enable)
        self.scrollBar_StepSize_Track.setEnabled(enable)
        self.scrollBar_TrackSize_Track.setEnabled(enable)
        self.spinBox_TrackSize_Track.setEnabled(enable)
        self.spinBox_StepSize_Track.setEnabled(enable)
        self.groupBox_PlaneFit_Track.setEnabled(enable)
        self.comboBox_ReadCh_Track.setEnabled(enable)
        self.spinBox_Avg_Track.setEnabled(enable)
        self.spinBox_ScanDelay_Track.setEnabled(enable)
        self.spinBox_StayDelay_Track.setEnabled(enable)
        



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myTrack()
    window.show()
    sys.exit(app.exec_())
# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/24 17:40:23
@Author   : milier00
@FileName : ScanOptions.py
"""
import sys

sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup
from PyQt5.QtCore import pyqtSignal, Qt
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
from ScanOptions_ui import Ui_ScanOptions
import numpy as np
import conversion as cnv
import functools as ft


class myScanOptions(QWidget, Ui_ScanOptions):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.spinBox_Retract_Tip.editingFinished.connect(lambda: self.scrollBar_Retract_tip.setValue(self.spinBox_Retract_Tip.value()))
        self.scrollBar_Retract_tip.valueChanged.connect(self.spinBox_Retract_Tip.setValue)
        
        self.spinBox_Advance_Feedabck.editingFinished.connect(lambda: self.scrollBar_Advance_Feedabck.setValue(self.spinBox_Advance_Feedabck.value()))
        self.scrollBar_Advance_Feedabck.valueChanged.connect(self.spinBox_Advance_Feedabck.setValue)

    def init_scanoptions(self):
        pass

    def configure_scan(self):
        if self.radioButton_YFirst_OrderandDirection.isChecked():
            channel_x, channel_y = 0x1f, 0x10
        else:
            channel_x, channel_y = 0x10, 0x1f
        dir_x = self.radioButton_ReadForward_OrderandDirection.isChecked()
        return channel_x, channel_y, dir_x
        
    def configure_delay(self, feedback):
        if self.radioButton_Fixed_Delay.isChecked():
            move_delay = self.spinBox_MoveControl_Delay.value()
            measure_delay = self.spinBox_ReadControl_Delay.value()
            line_delay = self.spinBox_LineControl_Delay.value()
        else:
            if feedback:
                move_delay = self.spinBox_MoveON_Delay.value()
                measure_delay = self.spinBox_ReadON_Delay.value()
                line_delay = self.spinBox_LineON_Delay.value()
            else:
                move_delay = self.spinBox_MoveOFF_Delay.value()
                measure_delay = self.spinBox_ReadOFF_Delay.value()
                line_delay = self.spinBox_LineOFF_Delay.value()
        return move_delay, measure_delay, line_delay
    
    def configure_scan_protect(self):
        if self.groupBox_Scan_ScanOptions.isChecked():
            if self.radioButton_Stop_Scan.isChecked():
                scan_protect_flag = 1
            elif self.radioButton_Auto0_Scan.isChecked():
                scan_protect_flag = 2
            elif self.radioButton_AutoPre_Scan.isChecked():
                scan_protect_flag = 3
        else:
            scan_protect_flag = 0
        limit = cnv.vb(self.spinBox_ZoutLmit_Scan.value(), 'a') - cnv.vb(0.0, 'a') 
        return scan_protect_flag, limit
        
    def configure_tip_protect(self, seq):
        if seq.feedback and (seq.ditherB or seq.ditherZ):
            tip_protection = False
        else:
            tip_protection = self.groupBox_Tip_ScanOptions.isChecked()
        tip_protect_data = self.scrollBar_Retract_tip.value()
        return tip_protection, tip_protect_data
            
    def configure_prescan(self, feedback):
        on_after = self.groupBox_Feedback_ScanOptions.isChecked() and (not feedback)
        match_curr = self.checkBox_MatchCurr_Feedback.isChecked() and on_after
        advance_bit = self.scrollBar_Advance_Feedabck.value() if on_after else 0
        return match_curr, advance_bit


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myScanOptions()
    window.show()
    sys.exit(app.exec_())
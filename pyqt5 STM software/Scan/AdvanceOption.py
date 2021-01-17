# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/24 17:30:19
@Author   : milier00
@FileName : AdvanceOption.py
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
from AdvanceOption_ui import Ui_AdvanceOption
import numpy as np
import conversion as cnv
import functools as ft

class myAdvanceOption(QWidget, Ui_AdvanceOption):
    seq_list_signal = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        pass

    def configure_prescan(self):
        feedback = self.radioButton_FeedbackON.isChecked()
        ditherB = self.radioButton_BiasON_AdvOption.isChecked()
        ditherZ = self.radioButton_ZON_AdvOption.isChecked()
        return feedback, ditherB, ditherZ
    
    def configure_delay(self):
        move_delay = self.spinBox_MoveDelay_Dealy.value()
        measure_delay = self.spinBox_MeasureDelay_Dealy.value()
        return move_delay, measure_delay
        
    def configure_measure(self):
        command_list = [0]
        data_list = [self.spinBox_Wait_Delay.value() * 1000]
        if self.radioButton_I_Measure.isChecked:
            command_list += [0xdc]          # 0xc0 + 4 * 7
            data_list += [self.spinBox_Avg_I.value()]
        if self.radioButton_Z_Measure.isChecked:
            command_list += [0xd8]          # 0xc0 + 4 * 6
            data_list += [self.spinBox_Avg_Z.value()]
        if self.checkBox_Ch2_Measure.isChecked:
            command_list += [0xc8]          # 0xc0 + 4 * 2
            data_list += [self.spinBox_Avg_CH2.value()]
        if self.checkBox_Ch3_Measure.isChecked:
            command_list += [0xcc]          # 0xc0 + 4 * 3
            data_list += [self.spinBox_Avg_CH3.value()]
        
        return command_list, data_list
    
    def configure_correction(self, feedback):
        corr_pass_num = self.spinBox_DoZCorrection_ZDrift.value()
        z_flag = self.groupBox_ZDrift_Correction.isChecked() and (not feedback)
        match_flag = self.checkBox_MatchCurr_ZDrift.isChecked() and z_flag
        feedback_delay = self.spinBox_Delay_ZDrift.value() if z_flag else 0
        track_flag = self.checkBox_Tracking_Correction.isChecked
        
        return corr_pass_num, z_flag, match_flag, feedback_delay, track_flag
        
    def configure_scan(self):
        forward = True
        backward = True
        average = False
        if self.radioButton_Forward_Dir.isChecked():
            backward = False
        elif self.radioButton_Bacward_Dir.isChecked():
            forward = False
        elif self.radioButton_AvgBoth_Dir.isChecked():
            average = True
        return forward, backward, average
    
    def configure_rescan(self):
        return self.spinBox_Pass_Rescan.value() if self.groupBox_Rescan.isChecked() else 0xffffffff
    


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myAdvanceOption()
    window.show()
    sys.exit(app.exec_())
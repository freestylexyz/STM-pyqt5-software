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
        wait_delay = self.spinBox_Wait_Delay.value() * 1000
        return move_delay, measure_delay, wait_delay
    
    def configure_rescan(self):
        if self.groupBox_Rescan.isChecked():
            return self.spinBox_Pass_Rescan.value()
        else:
            return 0xffffffff
        
    def configure_measure(self):
        command_list = []
        if self.radioButton_I_Measure.isChecked:
            command_list += [0xdc]          # 0xc0 + 4 * 7
        if self.radioButton_Z_Measure.isChecked:
            command_list += [0xd8]          # 0xc0 + 4 * 6
        if self.checkBox_Ch2_Measure.isChecked:
            command_list += [0xc8]          # 0xc0 + 4 * 2
        if self.checkBox_Ch3_Measure.isChecked:
            command_list += [0xcc]          # 0xc0 + 4 * 3
            
        data_list = len(command_list) * [self.spinBox_Avg_Measure.value()]
        
        return command_list, data_list
    
    def configure_correction(self, feedback):
        pass_num = self.spinBox_DoZCorrection_ZDrift.value()
        z_flag = self.groupBox_ZDrift_Correction.isChecked() and (not feedback)
        match_flag = self.checkBox_MatchCurr_ZDrift.isChecked() and z_flag
        feedback_delay = self.spinBox_Delay_ZDrift.value() if z_flag else 0
        track_flag = self.checkBox_Tracking_Correction.isChecked
        
        return pass_num, z_flag, match_flag, feedback_delay, track_flag
        
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
    
    def configure_autosave(self):
        autosave = self.groupBox_AutoSave_AdvOption.isChecked()
        every = self.checkBox_SaveEveryPasses_Autosave.isChecked() and autosave
        return autosave, every

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myAdvanceOption()
    window.show()
    sys.exit(app.exec_())
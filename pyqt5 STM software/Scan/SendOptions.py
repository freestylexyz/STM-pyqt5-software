# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/24 17:41:01
@Author   : milier00
@FileName : SendOptions.py
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
from SendOptions_ui import Ui_SendOptions
import numpy as np
import conversion as cnv
import functools as ft


class mySendOptions(QWidget, Ui_SendOptions):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
    
    # Configure send options
    def configure_send(self):
        delay = self.spinBox_MoveDelay_SendOptions.value()  # Delay between steps
        step = self.spinBox_StepSize_SendOptions.value()    # Step size
        
        # Configure limit for crash protection
        if self.groupBox_Crash_SendOptions.isChecked():
            limit = cnv.vb(self.spinBox_Limit_Crash.value(), 'a') - cnv.vb(0.0, 'a')
        else:
            limit = 0       # No crash protection
            
        return delay, step, limit
    


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySendOptions()
    window.show()
    sys.exit(app.exec_())
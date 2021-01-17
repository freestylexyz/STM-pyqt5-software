# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:20:38 2020

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
from PyQt5.QtWidgets import QApplication , QWidget
from PyQt5.QtCore import pyqtSignal , Qt
from LockIn_ui import Ui_LockIn



class myLockIn(QWidget, Ui_LockIn):

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.load()
        
    def load(self):
        self.osc_type = self.lineEdit_Type.text()
        self.params = []
        self.params += [self.spinBox_OSC_RMS.value()]
        self.params += [self.spinBox_OSC_Freq.value()]
        self.params += [self.spinBox_LockIn_Freq.value()]
        self.params += [self.spinBox_Phase1.value()]
        self.params += [self.spinBox_Phase2.value()]
        self.params += [self.spinBox_Sen1.value()]
        self.params += [self.spinBox_Sen2.value()]
        self.params += [self.spinBox_Offset1.value()]
        self.params += [self.spinBox_Offset2.value()]
        self.params += [self.spinBox_TC1.value()]
        self.params += [self.spinBox_TC2.value()]
    
    # Emit close signal
    def closeEvent(self, event):
        self.load()
        event.accept()
        
if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myLockIn()
    window.show()
    sys.exit(app.exec_())
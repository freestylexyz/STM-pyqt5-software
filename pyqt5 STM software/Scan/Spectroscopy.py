# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:19:02 2020

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
from Spectroscopy_ui import Ui_Spectroscopy
from AdvanceOption import myAdvanceOption
from Correction import myCorrection


class mySpc(QWidget, Ui_Spectroscopy):
    # Control signal
    close_signal = pyqtSignal()
    spectroscopy_signal = pyqtSignal()
    stop_signal = pyqtSignal()


    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()


    def init_UI(self):
        self.adv = myAdvanceOption()
        self.corr = myCorrection()
        
        self.bias_dac = False
        self.bias_ran = 9
        
        self.idling = True
    
    def init_spc(self, succeed, bias_dac, bias_ran):
        self.pushButton_Scan.seEnabled(succeed)
        self.bias_dac = bias_dac
        self.bias_ran = bias_ran
    
    # Update spectroscopy data
    def update_spc(self, rdata):
        pass
    
    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit()
        event.accept()
        
    def enable_serial(self, enable):
        pass

    def bias_ran_change(self, ran):
        pass


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySpc()
    window.show()
    sys.exit(app.exec_())
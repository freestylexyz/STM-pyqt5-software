# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:20:55 2020

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
from DepositionInfo_ui import Ui_DepositionInfo



class myDepositionInfo(QWidget, Ui_DepositionInfo):
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)

    def init_UI(self):
        pass
    
    def init_depInfo(self):
        pass
    
    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit()
        event.accept()
        
    def enable_serial(self, enable):
        pass



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myDepositionInfo()
    window.show()
    sys.exit(app.exec_())
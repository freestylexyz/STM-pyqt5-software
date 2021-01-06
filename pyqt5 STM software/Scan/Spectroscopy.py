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
from BetweenPasses import myBetweenPasses


class mySpc(QWidget, Ui_Spectroscopy):
    close_signal = pyqtSignal()
    seq_list_signal = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.adv = myAdvanceOption()
        self.bp = myBetweenPasses()

    def init_UI(self):
        pass
    
    def init_spc(self):
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
# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:49:07
@Author   : milier00
@FileName : TipApproach.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication , QWidget
from PyQt5.QtCore import pyqtSignal , Qt
from TipApproach_ui import Ui_TipApproach



class myTipApproach(QWidget, Ui_TipApproach):
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)

    def init_UI(self):
        pass
    
    def init_tipAppr(self):
        pass
    
    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit()
        event.accept()
    
    # Enable serial
    def enable_serial(self, enable):
        pass



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myTipApproach()
    window.show()
    sys.exit(app.exec_())
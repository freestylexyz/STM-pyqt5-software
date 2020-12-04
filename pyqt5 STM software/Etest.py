# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:46:12
@Author   : milier00
@FileName : Etest.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication , QWidget
from PyQt5.QtCore import pyqtSignal , Qt
from Etest_ui import Ui_ElectronicTest



class myEtest(QWidget, Ui_ElectronicTest):
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)

    def init_UI(self):
        pass
    
    def init_etest(self):
        pass
    
    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit()
        event.accept()
        
    def enable_serial(self, enable):
        pass



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myEtest()
    window.show()
    sys.exit(app.exec_())
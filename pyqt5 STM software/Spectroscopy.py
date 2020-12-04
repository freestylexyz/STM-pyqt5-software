# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:19:02 2020

@author: yaoji
"""

import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication , QWidget
from PyQt5.QtCore import pyqtSignal , Qt



class mySpc(QWidget):
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        # self.setupUi(self)

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



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySpc()
    window.show()
    sys.exit(app.exec_())
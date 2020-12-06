# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:18:34 2020

@author: yaoji
"""

import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication , QWidget
from PyQt5.QtCore import pyqtSignal , Qt
from Spectroscopy import mySpc
from Deposition import myDeposition
from Track import myTrack
from Hop import myHop
from Manipulation import myManipulation



class myScan(QWidget):
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        # self.setupUi(self)
        self.spc = mySpc()
        self.depostion = myDeposition()
        self.track = myTrack()
        self.hop = myHop()
        self.manipulation = myManipulation()
        
        self.mode = 0       # Scan mode: Scan(0), Spectroscopy(1), Deposition(2)
                            # Track(3), Hop(4), Manipulation(5)

    def init_UI(self):
        self.close_signal.clicked(self.spc.close_signal)
        self.close_signal.clicked(self.depostion.close_signal)
        self.close_signal.clicked(self.track.close_signal)
        self.close_signal.clicked(self.hop.close_signal)
        self.close_signal.clicked(self.manipulation.close_signal)
    
    def init_scan(self):
        self.spc.init_spc()
        self.depostion.init_deposition()
        self.track.init_track()
        self.hop.init_hop()
        self.manipulation.init_manipulation()
    
    # Emit close signal
    def closeEvent(self, event):
        if self.mode == 0:
            # !!! pop window to double check
            self.close_signal.emit()
            event.accept()
        else:
            # !!! pop message close other windows
            event.ignore()
        
    def enable_serial(self, enable):
        pass



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myScan()
    window.show()
    sys.exit(app.exec_())
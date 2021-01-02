# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:19:17 2020

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
from Deposition_ui import Ui_Deposition
from sequence import mySequence


class myDeposition(QWidget, Ui_Deposition):
    close_signal = pyqtSignal()
    seq_list_signal = pyqtSignal()
    do_it_signal = pyqtSignal()
    stop_signal = pyqtSignal()
    

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.seq = mySequence([], [], False)
        self.bias_dac = False
        self.bias_ran = 9
        self.poke_command = [0x42, 0x63, 0x8d]                                  # Feedback, Shift Z offset, Aout bias
        self.poke_data = [0x00000000, 0x80000000, 0x00008000]                   # OFF, Forward 0 bit, output zero voltage
        self.poke_seq = mySequence(self.poke_command, self.poke_data, False)    # Initial poke sequence
    
    # Called with int_scan
    def init_deposition(self, bias_dac, bias_ran):
        self.bias_dac = bias_dac
        self.bias_ran = bias_ran
    
    # def 
    
    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit()
        event.accept()
        
    def enable_serial(self, enable):
        pass



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myDeposition()
    window.show()
    sys.exit(app.exec_())
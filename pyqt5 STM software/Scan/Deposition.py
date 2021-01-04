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
import functools as ft


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
        self.idling = True
        self.seq = mySequence([], [], False)
        self.bias_dac = False
        self.bias_ran = 9
        self.poke_command = [0x42, 0x63, 0x8d]                                  # Feedback, Shift Z offset, Aout bias
        self.poke_data = [0x00000000, 0x80000000, 0x00008000]                   # OFF, Forward 0 bit, output zero voltage
        self.poke_seq = mySequence(self.poke_command, self.poke_data, False)    # Initial poke sequence
        
        # Connect signal
        self.groupBox_ReadNSample_Deposition.toggled.connect(ft.partial(self.read_mode, True))
        self.groupBox_ReadUntil_Deposition.toggled.connect(ft.partial(self.read_mode, False))
        self.groupBox_Seq_Deposition.toggled.connect(self.seq_mode)
        self.spinBox_DeltaZ_PokeTip.valueChanged.connect(self.scrollBar_PokeTip.setValue)
        self.scrollBar_PokeTip.valueChanged.connect(self.spinBox_DeltaZ_PokeTip.setValue)
        self.pushButton_SeqEditor_Deposition.clicked.connect(self.seq_list_signal)
        self.pushButton_DoIt_Deposition.clicked.connect(self.do_it)
        self.pushButton_Save_Deposition.clicked.connect(self.save)

    
    # Called with int_scan
    def init_deposition(self, succeed, bias_dac, bias_ran, seq):
        self.enable(succeed)
        self.bias_dac = bias_dac
        self.bias_ran = bias_ran
        self.seq = seq
        
        self.label_Seq_Deposition.setText(self.eq.name)
        
    # Read mode slot
    def read_mode(self, mode, state):
        if mode:
            if state:
                self.groupBox_ReadUntil_Deposition.setChecked(False)
        else:
            if state:
                self.groupBox_ReadNSample_Deposition.setChecked(False)
    
    # Sequence mode slot
    def seq_mode(self, state):
        self.groupBox_Poke_Deposition.setEnabled(not state)
    
    # Enable serial realted components
    def enable(self, enable):
        self.groupBox_Seq_Deposition.setEnabled(enable)
        self.groupBox_Read_Deposition.setEnabled(enable)
        self.groupBox_ReadPulse_Deposition.setEnabled(enable)
        self.pushButton_DoIt_Deposition.setEnabled(enable)
        self.pushButton_Save_Deposition.setEnabled(enable)
        mode = not self.groupBox_Poke_Deposition.isChecked()
        self.groupBox_Poke_Deposition.setEnabled(enable and mode)
            
    # Do it slot
    def do_it(self):
        pass
    
    # Save data slot
    def save(self):
        pass
    
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
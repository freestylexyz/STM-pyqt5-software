# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/24 17:45:33
@Author   : milier00
@FileName : SequenceList.py
"""
import sys

sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup, QFileDialog
from PyQt5.QtCore import pyqtSignal, Qt
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
from SequenceList_ui import Ui_SequenceList
import numpy as np
import conversion as cnv
import functools as ft
from SequenceEditor import mySequenceEditor
from sequence import mySequence
import pickle


class mySequenceList(QWidget, Ui_SequenceList):
    close_signal = pyqtSignal()

    def __init__(self, mode = True):
        super().__init__()
        self.idling = True
        self.setupUi(self)
        self.init_UI(mode)

    def init_UI(self, mode):
        self.seqED = mySequenceEditor()     # Instantiate sequence editor window
        
        self.seqlist = []                   # Default empty sequence list
        self.namelist = []                  # Default empty sequence name list
        self.selected = -1                  # Default no selected sequence
        
        self.mode = mode                        # True for read sequence, False for deposition sequence
                                                # Default read sequence
        self.bias_dac = False                   # Default 16 bitDAC for bias
        self.dacrange = [10, 10, 14] + ([10] * 9) + [14, 9] + ([10] * 3)  # Default DAC range
        self.preamp_gain = 9                     # Default preamp gain
        self.lastdac = [0x8000] * 16            # Default DAC last output
        self.lastdac[13] = 0x828f               # Default 16bit bias last output
        self.last20bitdac = 0x828f5
        
        # Button signal connection
        self.pushButton_Load_SeqList.clicked.connect(lambda: self.load(True, -1))
        self.pushButton_Build_SeqList.clicked.connect(lambda: self.open_editor(False))
        self.pushButton_Edit_SeqList.clicked.connect(lambda: self.open_editor(True))
        self.pushButton_Remove_SeqList.clicked.connect(self.remove)
        self.pushButton_Select_SeqList.clicked.connect(self.select)
        # Sequence editor signal connection
        self.seqED.add_signal.connect(ft.partial(self.load, False))

    # Configure sequence
    def config_seq(self, seq):
        seq.configure(self.bias_dac, self.preamp_gain, self.dacrange, self.lastdac, self.last20bitdac)
        return seq

    # Seqence list window initialization
    def init_seqlist(self, seq_list, selected_name, mode, bias_dac, preamp_gain, dacrange, lastdac, last20bitdac):
        self.seqlist = []                   # Empty sequence list
        self.namelist = []                  # Empty name list
        self.listWidget_SeqList.clear()     # Clear all item
        
        # # Configure new sequence
        # self.new_seq.configure(mode, bias_dac, preamp_gain, dacrange, lastdac, last20bitdac, feedback, ditherB, ditherZ)
        # Set sequence list status
        self.mode = mode
        self.bias_dac = bias_dac
        self.dacrange = dacrange
        self.preamp_gain = preamp_gain
        self.lastdac = lastdac
        self.last20bitdac = last20bitdac
        
        # Load sequence name list and add list widget items
        for seq in seq_list:
            self.seqlist =+ [self.config_seq(seq)]
            self.listWidget_SeqList.addItem(seq.name)
            self.namelist += [seq.name]
        
        # Set selected sequence
        if selected_name not in self.namelist:
            self.selected = -1
        else:
            self.selected = self.namelist.index(selected_name)
        
        self.listWidget_SeqList.setCurrentRow(self.selected)
    
        
    # Load sequence from file
    def load(self, opt, num):
        flag = True
        if opt:
            fname = QFileDialog.getOpenFileName(self, 'Open file', '', 'SEQ Files (*.seq)')[0]  # Open file and get file name
        
            # If file name is not empty
            if fname != '':
                with open(fname, 'rb') as input:
                    seq = self.config_seq(pickle.load(input))
            else:
                flag = False
        else:
            seq = self.seqED.seq
            
        if flag:
            if seq.mode != self.mode:
                QMessageBox.information(None, "Sequence compiler", "Sequence mode not compatible", QMessageBox.Ok)
            elif seq.name not in self.namelist:
                if num < 0:
                    self.seqlist += [seq]
                    self.namelist += [seq.name]
                    self.listWidget_SeqList.addItem(seq.name)
                else:
                    self.seqlist[num] = seq
                    self.namelist[num] = seq.name
                    self.listWidget_SeqList.item(num).setText(seq.name)
                if not opt:
                    self.seqED.close()
            else:
                if opt:
                    QMessageBox.information(None, "Sequence compiler", "Sequence is already in list!", QMessageBox.Ok)
                else:
                    self.seqED.message()
    
    # Open sequence editor
    def open_editor(self, opt):
        if opt:
            row = self.listWidget_SeqList.currentRow()
            seq = self.seqlist[row]
        else:
            row = -1
            seq = self.config_seq(mySequence([], [], self.mode))
        self.seqED.load_seq(seq, False, row)
        self.seqED.edit = opt
        self.seqED.show()
        
    # Remove selected sequence from list
    def remove(self):
        row = self.listWidget_SeqList.currentRow()
        if row >= 0:                                    # If there is a selected row
            self.listWidget_SeqList.takeItem(row)       # Remove selected item from list widget
            self.seqlist.remove( self.seqlist[row])     # Remove selected sequence from sequence list
            self.namelist.remove( self.namelist[row])   # Remove selected sequence name from sequence name list
    
    # Change selected squence index
    def select(self):
        current_index = self.listWidget_SeqList.currentRow()
        if current_index >= 0:
            self.selected = current_index
            self.close()
        else:
            QMessageBox.warning(None, "Sequence compiler", "No sequence selected!", QMessageBox.Ok)

    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit()
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySequenceList()
    window.show()
    sys.exit(app.exec_())
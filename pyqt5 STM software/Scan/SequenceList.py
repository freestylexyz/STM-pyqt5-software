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

    def __init__(self):
        super().__init__()
        self.idling = True
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.seqED = mySequenceEditor()     # Instantiate sequence editor window
        
        self.seqlist = []                   # Default empty sequence list
        self.namelist = []                  # Default empty sequence name list
        self.selected = -1                  # Default no selected sequence
        
        # Button signal connection
        self.pushButton_Load_SeqList.clicked.connect(self.load)
        self.pushButton_Build_SeqList.clicked.connect(self.build_new)
        self.pushButton_Edit_SeqList.clicked.connect(self.edit_exit)
        self.pushButton_Remove_SeqList.clicked.connect(self.remove)
        self.pushButton_Select_SeqList.clicked.connect(self.select)

    # Seqence list window initialization
    def init_seqlist(self, seq_list, selected_name):
        self.seqlist = seq_list             # Load sequence list
        self.namelist = []                  # Init empty name list
        self.listWidget_SeqList.clear()     # Clear all item
        
        # Load sequence name list and add list widget items
        for seq in self.seqlist:
            self.listWidget_SeqList.addItem(seq.name)
            self.namelist += [seq.name]
        
        # Set selected sequence
        if selected_name not in self.namelist:
            self.selected = -1
        else:
            self.selected = self.namelist.index(selected_name)
        
        self.listWidget_SeqList.setCurrentRow(self.selected)
    
    # Load sequence from file
    def load(self):
        fname = QFileDialog.getOpenFileName(self, 'Open file', '', 'SEQ Files (*.seq)')[0]  # Open file and get file name
        
        # If file name is not empty
        if fname != '':
            with open(fname, 'rb') as input:
                seq = pickle.load(input)
            if seq.name not in self.namelist:
                self.seqlist += [seq]
                self.namelist += [seq.name]
                self.listWidget_SeqList.addItem(seq.name)
            else:
                QMessageBox.information(None, "Sequence compiler", "Sequence is already in list!", QMessageBox.Ok)
                
    # Open sequence editor and start building a new sequence
    def build_new(self):
        self.seqED.show()
    
    # Open sequence editor and start current sequence
    def edit_exit(self):
        row = self.listWidget_SeqList.currentRow()
        seq = self.seqlist[row]
        self.seqED.show()
        # self.listWidget_SeqList.clear()
        
    # Remove selected sequence from list
    def remove(self):
        row = self.listWidget_SeqList.currentRow()
        if row >= 0:                                    # If there is a selected row
            self.listWidget_SeqList.takeItem(row)       # Remove selected item from list widget
            self.seqlist.remove( self.seqlist[row])     # Remove selected sequence from sequence list
            self.namelist.remove( self.namelist[row])   # Remove selected sequence name from sequence name list
    
    # Change selected squence index
    def select(self):
        self.selected = self.listWidget_SeqList.currentRow()
        if self.selected >= 0:
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
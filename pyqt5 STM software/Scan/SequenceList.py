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
from PyQt5.QtWidgets import QApplication, QWidget, QMessageBox, QFileDialog
from PyQt5.QtCore import pyqtSignal, Qt
from PyQt5.QtGui import QBrush, QColor
from SequenceList_ui import Ui_SequenceList
import functools as ft
from SequenceEditor import mySequenceEditor
from sequence import mySequence
import pickle
import copy


class mySequenceList(QWidget, Ui_SequenceList):
    close_signal = pyqtSignal(int)             # Close signal

    def __init__(self, mode = True):
        super().__init__()
        self.idling = True
        self.setupUi(self)
        self.init_UI(mode)

    def init_UI(self, mode):
        self.index = 0
        self.seqED = mySequenceEditor()         # Instantiate sequence editor window
        
        self.seqlist = []                       # Default empty sequence list
        self.namelist = []                      # Default empty sequence name list
        self.selected = -1                      # Default no selected sequence
        
        self.mode = mode                        # True for read sequence, False for deposition sequence
                                                # Default read sequence
        self.bias_dac = False                   # Default 16 bitDAC for bias
        self.dacrange = [10, 10, 14] + ([10] * 9) + [14, 9] + ([10] * 3)  # Default DAC range
        self.preamp_gain = 9                    # Default preamp gain
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

    # Sequence list window initialization
    def init_seqlist(self, index, seq_list, selected_name, mode, bias_dac, preamp_gain, dacrange, lastdac, last20bitdac):
        self.index = index
        self.seqlist = []                   # Empty sequence list
        self.namelist = []                  # Empty name list
        self.listWidget_SeqList.clear()     # Clear all item
        
        # Set sequence list status
        self.mode = mode
        self.bias_dac = bias_dac
        self.dacrange = dacrange
        self.preamp_gain = preamp_gain
        self.lastdac = lastdac
        self.last20bitdac = last20bitdac
        
        # Load sequence name list and add list widget items
        for seqq in seq_list:
            seq = copy.deepcopy(seqq)
            self.seqlist += [self.config_seq(seq)]
            self.listWidget_SeqList.addItem(seq.name)
            self.namelist += [seq.name]
        
        # Set selected sequence
        if selected_name not in self.namelist:
            self.selected = -1
        else:
            self.selected = self.namelist.index(selected_name)
            item = self.listWidget_SeqList.item(self.selected)
            item.setBackground(QBrush(QColor(233, 196, 106)))

        self.listWidget_SeqList.setCurrentRow(self.selected)

    # Load sequence from file
    def load(self, opt, num):
        flag = True
        if opt:                     # Load from file
            fname = QFileDialog.getOpenFileName(self, 'Open file', '', 'SEQ Files (*.seq)')[0]  # Open file and get file name
        
            # If file name is not empty
            if fname != '':
                with open(fname, 'rb') as input:
                    seqq = self.config_seq(pickle.load(input))
                    seqq.path = fname
            else:
                flag = False        # Not able to load
        else:
            seqq = self.seqED.seq   # Load from sequence editor
            
        name_list = self.namelist.copy()    # Get a copy of name list
        # If able to load
        if flag:
            seq = copy.deepcopy(seqq)       # Make a copy
            if (num >= 0) and (self.namelist[num] == seq.name):
                del name_list[num]          # Delete original sequence name if updating and name not changed
                
            # Take care of mode conflict
            if seq.mode != self.mode:
                QMessageBox.information(None, "Sequence compiler", "Sequence mode not compatible", QMessageBox.Ok)
                
            # Take care of name duplication
            elif seq.name not in name_list:
                # Load new sequence
                if num < 0:
                    self.seqlist += [seq]
                    self.namelist += [seq.name]
                    self.listWidget_SeqList.addItem(seq.name)
                # Update sequence
                else:
                    self.seqlist[num] = seq
                    self.namelist[num] = seq.name
                    self.listWidget_SeqList.item(num).setText(seq.name)
                if not opt:
                    self.seqED.close()
            else:
                if opt:     # Load from file
                    QMessageBox.information(None, "Sequence compiler", "Sequence is already in list!", QMessageBox.Ok)
                else:       # Load from sequence editor
                    self.seqED.message()
    
    # Open sequence editor
    def open_editor(self, opt):
        flag = True
        if opt:     # Edit selected sequence
            row = self.listWidget_SeqList.currentRow()
            if row >= 0:            # Valid row number
                seq = self.seqlist[row]
                flag = seq.validation_required              # Validation not required sequences are not editable
                if not flag:
                    QMessageBox.information(None, "Sequence compiler", "Selected sequence is not editable!", QMessageBox.Ok)
            else:                   # Invalid row number
                flag = False                                # Not able to open editor
                QMessageBox.information(None, "Sequence compiler", "No sequence selected!", QMessageBox.Ok)
        else:       # Build new sequence
            row = -1
            seq = self.config_seq(mySequence([], [], self.mode))
            
        if flag:
            self.seqED.load_seq(seq, False, row)
            self.seqED.show()            
        
    # Remove selected sequence from list
    def remove(self):
        row = self.listWidget_SeqList.currentRow()
        if row >= 0:                                    # If there is a selected row
            self.listWidget_SeqList.takeItem(row)       # Remove selected item from list widget
            del self.seqlist[row]                       # Remove selected sequence from sequence list
            del self.namelist[row]                      # Remove selected sequence name from sequence name list 
    
    # Change selected sequence index
    def select(self):
        current_index = self.listWidget_SeqList.currentRow()
        if current_index >= 0:                   # Valid row number
            self.selected = current_index        # Store selected number (-1 stands for no selected sequence)
            self.close()
        else:                                    # Invalid row number
            QMessageBox.warning(None, "Sequence compiler", "No sequence selected!", QMessageBox.Ok)

    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit(self.index)
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySequenceList()
    window.show()
    sys.exit(app.exec_())
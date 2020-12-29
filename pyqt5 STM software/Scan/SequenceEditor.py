# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/24 17:43:11
@Author   : milier00
@FileName : SequenceEditor.py
"""
import sys

sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import *
from PyQt5.QtCore import pyqtSignal, Qt
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
from SequenceEditor_ui import Ui_SequenceEditor
import numpy as np
import conversion as cnv
import functools as ft
from sequence import mySequence
import pickle


class mySequenceEditor(QWidget, Ui_SequenceEditor):
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.idling = True
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        # set up table widget
        self.table_Content_SeqEditor.setColumnWidth(0, 100)
        self.table_Content_SeqEditor.setColumnWidth(1, 100)
        self.table_Content_SeqEditor.setColumnWidth(2, 100)
        self.table_Content_SeqEditor.setColumnWidth(3, 100)
        self.table_Content_SeqEditor.setColumnWidth(4, 100)
        self.table_Content_SeqEditor.setColumnWidth(5, 30)

        self.path = ''
        self.seq = mySequence([], [])
        self.command_combo_list = []
        self.channel_widget_list = []
        self.option1_widget_list = []
        self.option2_widget_list = []
        self.data_widget_list = []
        self.unit_widget_list = []
        
        self.pushButton_AddStep_Edit.clicked.connect(self.add_step)
        self.pushButton_Remove_Edit.clicked.connect(self.remove_step)
        self.pushButton_Insert_Edit.clicked.connect(self.insert_step)
        self.pushButton_Clear_Edit.clicked.connect(self.clear_all)
        self.pushButton_Save_Edit.clicked.connect(self.save_seq)
        self.pushButton_SaveAs_Edit.clicked.connect(self.saveas_seq)
        self.pushButton_Validate_Edit.clicked.connect(self.validate)
        self.pushButton_Simulation_Edit.clicked.connect(self.simulate)
        self.pushButton_AddtoList_Edit.clicked.connect(self.add_to_list)
        self.pushButton_Cancel_Edit.clicked.connect(self.cancel)

    def edit_seq(self, seq):
        self.seq = seq

    def new_seq(self):
        self.path = ''
        self.seq = mySequence([], [])
        
    def change_command(self, seq_num, index):
        setup_dict = {0: self.setup_wait, 1: self.setup_match, 2: self.setup_dout, 3: self.setup_shift, \
                      4: self.setup_aout, 5: self.setup_ramp, 6: self.setup_read, 7: self.setup_sramp}
        setup_dict[index](seq_num)

    
    def setup_wait(self, seq_num):
        print('Wait')
    
    def setup_match(self, seq_num):
        print('Match')
    
    def setup_dout(self, seq_num):
        # Channel widget
        channel = QComboBox()
        channel.addItem('DitherB')
        channel.addItem('DitherZ')
        channel.addItem('Feedback')
        self.channel_widget_list[seq_num] = channel
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 1)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 1, self.channel_widget_list[seq_num])
        
        # Data widget
        data = QComboBox()
        data.addItem('OFF')
        data.addItem('ON')
        self.data_widget_list[seq_num] = data
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 5)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 5, self.data_widget_list[seq_num])
        unit = QTableWidgetItem('bool')
        self.unit_widget_list[seq_num] = unit
        print('Dout')
    
    def setup_shift(self, seq_num):
        print('Shift')
        
    def setup_aout(self, seq_num):
        print('Aout')

    def setup_ramp(self, seq_num):
        print('Ramp')

    def setup_read(self, seq_num):
        print('Read')
    
    def setup_sramp(self, seq_num):
        print('ShiftRamp')

    def add_step(self):
        self.table_Content_SeqEditor.insertRow(self.seq.seq_num)
        
        # Command widget
        command = QComboBox()
        command.addItem('Wait')
        command.addItem('Match')
        command.addItem('Dout')
        command.addItem('Shift')
        command.addItem('Aout')
        command.addItem('Ramp')
        command.addItem('Read')
        command.addItem('ShiftRamp')
        command.currentIndexChanged.connect(ft.partial(self.change_command, self.seq.seq_num))
        # command.setStyleSheet("QComboBox{margin:3px};")
        self.command_combo_list += [command]
        self.table_Content_SeqEditor.setCellWidget(self.seq.seq_num, 0, self.command_combo_list[self.seq.seq_num])
        
        # Channel widget
        channel = QLabel()
        channel.setText('NA')
        self.channel_widget_list += [channel]
        self.table_Content_SeqEditor.setCellWidget(self.seq.seq_num, 1, self.channel_widget_list[self.seq.seq_num])
        
        # Option1 widget
        option1 = QLabel()
        option1.setText('NA')
        self.option1_widget_list += [option1]
        self.table_Content_SeqEditor.setCellWidget(self.seq.seq_num, 2, self.option1_widget_list[self.seq.seq_num])
        
        # Option2 widget
        option2 = QLabel()
        option2.setText('NA')
        self.option2_widget_list += [option2]
        self.table_Content_SeqEditor.setCellWidget(self.seq.seq_num, 3, self.option2_widget_list[self.seq.seq_num])
        
        # Data widget
        data = QSpinBox()
        data.setMaximum(2 ** 31 - 1)
        data.setMinimum(0)
        self.data_widget_list += [data]
        self.table_Content_SeqEditor.setCellWidget(self.seq.seq_num, 4, self.data_widget_list[self.seq.seq_num])
        
        # Unit widget
        unit = QLabel()
        unit.setText('us')
        self.unit_widget_list += [unit]
        self.table_Content_SeqEditor.setCellWidget(self.seq.seq_num, 5, self.unit_widget_list[self.seq.seq_num])
        
        self.seq.seq_num += 1
        self.seq.command += ['Wait']
        self.seq.channel += [' ']
        self.seq.option1 += [0]
        self.seq.option2 += [0]
        self.seq.data += ['0']

    
    def remove_step(self):
        pass

    def insert_step(self):
        pass

    def clear_all(self):
        pass

    def save_seq(self):
        if self.path == '':
            self.saveas_seq
        else:
            pass

    def saveas_seq(self):
        fname = QFileDialog.getSaveFileName(self, 'Save file', '', 'SEQ Files (*.seq)')[0]  # Save file and get file name
        if fname != '':
            self.path = fname

    def validate(self):
        pass

    def simulate(self):
        print(self.path)

    def add_to_list(self):
        pass

    def cancel(self):
        pass    

    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit()
        event.accept()




if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySequenceEditor()
    window.show()
    sys.exit(app.exec_())
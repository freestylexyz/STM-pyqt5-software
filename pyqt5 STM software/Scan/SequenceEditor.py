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
from PyQt5.QtWidgets import QWidget, QComboBox, QCheckBox, QLabel, QSpinBox, QDoubleSpinBox, QApplication, QFileDialog
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
        
    def change_command(self, seq_num):
        self.setup_channel(seq_num)
        self.setup_option1(seq_num)
        self.setup_option2(seq_num)
        self.setup_data_unit(seq_num)
        
    def change_channel(self, seq_num):
        self.setup_option1(seq_num)
        self.setup_option2(seq_num)
        self.setup_data_unit(seq_num)
    
    def original(self, seq_num):
        enable = not self.option1_widget_list[seq_num].isChecked()
        self.data_widget_list[seq_num].setEnabled(enable)
        
    def setup_channel(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        if command_index <= 1:
            channel = QLabel()
            channel.setText('NA')
        elif command_index == 2:
            channel = QComboBox()
            channel.addItem('DitherB')
            channel.addItem('DitherZ')
            channel.addItem('Feedback')
        elif command_index == 6:
            channel = QComboBox()
            channel.addItem('PREAMP')
            channel.addItem('ZOUT')
            channel.addItem('AIN0')
            channel.addItem('AIN1')
            channel.addItem('AIN2')
            channel.addItem('AIN3')
            channel.addItem('AIN4')
            channel.addItem('AIN5')
        else:
            channel = QComboBox()
            channel.addItem('Z offset fine')
            channel.addItem('Z offset')
            channel.addItem('Iset')
            channel.addItem('DAC6')
            channel.addItem('DAC7')
            channel.addItem('DAC8')
            channel.addItem('DAC9')
            channel.addItem('DAC11')
            channel.addItem('Bias')
            channel.currentIndexChanged.connect(lambda: self.change_channel(seq_num))
        self.channel_widget_list[seq_num] = channel
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 1)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 1, self.channel_widget_list[seq_num])
    
    def setup_option1(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        if command_index == 1:
            option1 = QComboBox()
            option1.addItem('SLOW')
            option1.addItem('FAST')
        elif (command_index == 3) or (command_index == 7):
            option1 = QComboBox()
            option1.addItem('DOWN')
            option1.addItem('UP')
        elif (command_index == 4) or (command_index == 5):
            option1 = QCheckBox('Origin')
            option1.stateChanged.connect(lambda: self.original(seq_num))
        else:
            option1 = QLabel()
            option1.setText('NA')
        self.option1_widget_list[seq_num] = option1
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 2)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 2, self.option1_widget_list[seq_num])
    
    def setup_option2(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        if command_index == 5:
            option2 = QSpinBox()
            option2.setMaximum(2 ** 12 -1)
            option2.setMinimum(0)
        elif command_index == 7:
            option2 = QSpinBox()
            option2.setMaximum(2 ** 11 -1)
            option2.setMinimum(0)
        else:
            option2 = QLabel()
            option2.setText('NA')
        self.option2_widget_list[seq_num] = option2
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 3)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 3, self.option2_widget_list[seq_num])
        
    def setup_data_unit(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        unit = QLabel()
        if command_index == 0:
            data = QSpinBox()
            data.setMaximum(2 ** 31 -1)
            data.setMinimum(0)
            unit.setText('us')
        elif command_index == 1:
            data = QLabel()
            data.setText('NA')
            unit.setText('NA')
        elif command_index == 2:
            data = QComboBox()
            data.addItem('OFF')
            data.addItem('ON')
            unit.setText('NA')
        elif command_index == 6:
            data = QSpinBox()
            data.setMaximum(2 ** 16 -1)
            data.setMinimum(0)
            unit.setText('times')
        else:
            channel_index = self.channel_widget_list[seq_num].currentIndex()
            if channel_index <= 1:
                data = QSpinBox()
                data.setMaximum(2 ** 16 -1)
                data.setMinimum(0)
                unit.setText('bits')
            elif channel_index == 2:
                data = QDoubleSpinBox()
                data.setMaximum(0.01)
                data.setMinimum(100.0)
                unit.setText('nA')
            else:
                data = QDoubleSpinBox()
                data.setMaximum(-10.0)
                data.setMinimum(10.0)
                unit.setText('Volts')

        self.data_widget_list[seq_num] = data
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 4)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 4, self.data_widget_list[seq_num])

        self.unit_widget_list[seq_num] = unit
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 5)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 5, self.unit_widget_list[seq_num])

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

        # Expand all widget list space
        self.channel_widget_list += [' ']
        self.option1_widget_list += [' ']
        self.option2_widget_list += [' ']
        self.data_widget_list += [' ']
        self.unit_widget_list += [' ']

        self.change_command(self.seq.seq_num)
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
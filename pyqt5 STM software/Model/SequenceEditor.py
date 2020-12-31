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
from PyQt5.QtWidgets import QWidget, QComboBox, QCheckBox, QLabel, QSpinBox, QDoubleSpinBox, QApplication, QFileDialog, QButtonGroup, QMessageBox
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
    add_signal = pyqtSignal(int)

    def __init__(self):
        super().__init__()
        self.idling = True
        self.setupUi(self)
        self.seq = mySequence()
        self.load_seq(self.seq, True, False)
        self.init_UI()
        
    def init_UI(self):
        # set up table widget
        # self.table_Content_SeqEditor.setColumnWidth(0, 100)
        # self.table_Content_SeqEditor.setColumnWidth(1, 100)
        # self.table_Content_SeqEditor.setColumnWidth(2, 100)
        # self.table_Content_SeqEditor.setColumnWidth(3, 100)
        # self.table_Content_SeqEditor.setColumnWidth(4, 100)
        # self.table_Content_SeqEditor.setColumnWidth(5, 30)

        # Radio button group
        self.mode_group = QButtonGroup()
        self.mode_group.addButton(self.radioButton_Deposition_Mode, 0)
        self.mode_group.addButton(self.radioButton_ReadSeq_Mode, 1)
        
        self.bias_group = QButtonGroup()
        self.bias_group.addButton(self.radioButton_16bit_Bias, 0)
        self.bias_group.addButton(self.radioButton_20bit_Bias, 1)
        
        self.preamp_group = QButtonGroup()
        self.preamp_group.addButton(self.radioButton_Gain8_Preamp, 8)
        self.preamp_group.addButton(self.radioButton_Gain9_Preamp, 9)
        self.preamp_group.addButton(self.radioButton_Gain10_Preamp, 10)
        
        # Connect signal
        # Mode
        self.mode_group.buttonToggled.connect(self.mode_change)
        self.bias_group.buttonToggled.connect(self.bias_mode_change)
        self.preamp_group.buttonToggled.connect(self.preamp_change)
        # Check boxes
        self.checkBox_FeedbackON__SeqEditor.stateChanged.connect(ft.partial(self.digital_change, 2))
        self.checkBox_DitherB__SeqEditor.stateChanged.connect(ft.partial(self.digital_change, 0))
        self.checkBox_DitherZ__SeqEditor.stateChanged.connect(ft.partial(self.digital_change, 1))
        # Name
        self.lineEdit_Name_SeqEditor.editingFinished.connect(self.name_change)
        # Pushbuttons
        self.pushButton_AddStep_Edit.clicked.connect(lambda: self.add_step(self.seq.seq_num, True))
        self.pushButton_Remove_Edit.clicked.connect(self.remove_step)
        self.pushButton_Insert_Edit.clicked.connect(lambda: self.add_step(-1, True))
        self.pushButton_Clear_Edit.clicked.connect(self.clear_all)
        self.pushButton_Save_Edit.clicked.connect(lambda: self.save_seq(False))
        self.pushButton_SaveAs_Edit.clicked.connect(lambda: self.save_seq(True))
        self.pushButton_Validate_Edit.clicked.connect(self.validate)
        self.pushButton_AddtoList_Edit.clicked.connect(self.add_to_list)
        self.pushButton_Simulation_Edit.clicked.connect(self.simulate)
        self.pushButton_Cancel_Edit.clicked.connect(self.cancel)

    def load_seq(self, seq, enable, edit):
        self.edit = edit
        if edit >= 0:
            self.pushButton_AddtoList_Edit.setText('Update')
        else:
            self.pushButton_AddtoList_Edit.setText('Add To List')
        self.clear_all()
        self.seq = seq
        self.saved = (self.seq.path != '')
        if not self.saved:
            self.setWindowTitle('Sequence Editor*')
        
        for i in range(self.seq.seq_num):
            self.add_step(i, False)

        self.setup_status(enable)
        
    def setup_status(self, enable):
        self.lineEdit_Name_SeqEditor.setText(self.seq.name)
        self.checkBox_FeedbackON__SeqEditor.setChecked(self.seq.feedback)
        self.checkBox_DitherB__SeqEditor.setChecked(self.seq.ditherB)
        self.checkBox_DitherZ__SeqEditor.setChecked(self.seq.ditherZ)
        
        self.radioButton_ReadSeq_Mode.setChecked(self.seq.mode)
        self.radioButton_Deposition_Mode.setChecked(not self.seq.mode)
        
        self.radioButton_20bit_Bias.setChecked(self.seq.bias_dac)
        self.radioButton_16bit_Bias.setChecked(not self.seq.bias_dac)
        
        if self.seq.preamp_gain == 8:
            self.radioButton_Gain8_Preamp.setChecked(True)
        if self.seq.preamp_gain == 9:
            self.radioButton_Gain9_Preamp.setChecked(True)
        if self.seq.preamp_gain == 10:
            self.radioButton_Gain10_Preamp.setChecked(True)
        
        self.radioButton_ReadSeq_Mode.setEnabled(enable)
        self.radioButton_Deposition_Mode.setEnabled(enable)
        self.radioButton_20bit_Bias.setEnabled(enable)
        self.radioButton_16bit_Bias.setEnabled(enable)
        self.radioButton_Gain8_Preamp.setEnabled(enable)
        self.radioButton_Gain9_Preamp.setEnabled(enable)
        self.radioButton_Gain10_Preamp.setEnabled(enable)
        
    def preamp_change(self, index, status):
        self.seq.preamp_gain = index
        
    def mode_change(self, index, status):
        self.seq.mode = (index == 1)
        self.saved = False
        self.setWindowTitle('Sequence Editor*')
        
    def bias_mode_change(self, index, status):
        self.seq.bias_dac = (index == 1)
        
    def digital_change(self, ch, status):
        if ch == 0:
            self.seq.ditherB = (status != 0)
        elif ch == 1:
            self.seq.ditherZ = (status != 0)
        elif ch == 2:
            self.seq.feedback = (status != 0)
        self.saved = False
        self.setWindowTitle('Sequence Editor*')
        
    def name_change(self):
        self.seq.name = self.lineEdit_Name_SeqEditor.text()
        self.saved = False
        self.setWindowTitle('Sequence Editor*')
        
    def command_default(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        channel_default_list = [' ', ' ', 'Feedback', 'Bias', 'Bias', 'Bias', 'PREAMP', 'Bias']
        option1_default_list = [0] * 8
        option2_default_list = [0] * 8
        data_default_list = ['0', '0', '0', '0.0', '0.0', '0.0', '0', '0.0']
        self.seq.channel[seq_num] = channel_default_list[command_index]
        self.seq.option1[seq_num] = option1_default_list[command_index]
        self.seq.option2[seq_num] = option2_default_list[command_index]
        self.seq.data[seq_num] = data_default_list[command_index]
        
    def channel_default(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        channel_index = self.channel_widget_list[seq_num].currentIndex()
        data_default_list = ['0', '0', '0.1', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0']
        flag = (command_index == 3) or (command_index == 4) or (command_index == 5) or (command_index == 7)
        if flag:
            self.seq.data[seq_num] = data_default_list[channel_index]
    
    def change_command(self, seq_num, opt):
        if seq_num < 0:
            seq_num = self.table_Content_SeqEditor.currentRow()
        if opt:
            self.command_default(seq_num)
        self.setup_channel(seq_num)
        self.setup_option1(seq_num)
        self.setup_option2(seq_num)
        self.setup_data_unit(seq_num)
        self.saved = False
        self.setWindowTitle('Sequence Editor*')
        
    def change_channel(self, opt):
        seq_num = self.table_Content_SeqEditor.currentRow()
        if opt:
            self.channel_default(seq_num)
        self.seq.channel[seq_num] = self.channel_widget_list[seq_num].currentText()
        self.setup_option1(seq_num)
        self.setup_option2(seq_num)
        self.setup_data_unit(seq_num)
        self.saved = False
        self.setWindowTitle('Sequence Editor*')
    
    def change_option1(self):
        seq_num = self.table_Content_SeqEditor.currentRow()
        self.seq.option1[seq_num] = self.option1_widget_list[seq_num].currentIndex()
        self.saved = False
        self.setWindowTitle('Sequence Editor*')
    
    def change_option2(self):
        seq_num = self.table_Content_SeqEditor.currentRow()
        self.seq.option2[seq_num] = self.option2_widget_list[seq_num].value()
        self.saved = False
        self.setWindowTitle('Sequence Editor*')
        
    def change_data(self):
        seq_num = self.table_Content_SeqEditor.currentRow()
        if self.command_combo_list[seq_num].currentIndex() == 2:
            self.seq.data[seq_num] = str(self.data_widget_list[seq_num].currentIndex())
        else:
            self.seq.data[seq_num] = str(self.data_widget_list[seq_num].value())
        self.saved = False
        self.setWindowTitle('Sequence Editor*')
    
    def original(self, state):
        seq_num = self.table_Content_SeqEditor.currentRow()
        if state == 2:
            self.data_widget_list[seq_num].setEnabled(False)
            self.seq.data[seq_num] = 'Origin'
        elif state == 0:
            self.data_widget_list[seq_num].setEnabled(True)
            self.seq.data[seq_num] = str(self.data_widget_list[seq_num].value())
        self.saved = False
        self.setWindowTitle('Sequence Editor*')

        
    def setup_channel(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        channel_digital_list = ['DitherB', 'DitherZ', 'Feedback']
        channel_adc_list = ['PREAMP', 'ZOUT', 'AIN0', 'AIN1', 'AIN2', 'AIN3', 'AIN4', 'AIN5']
        channel_dac_list = ['Z offset fine', 'Z offset', 'Iset', 'DAC6', 'DAC7', 'DAC8', 'DAC9', 'DAC11', 'Bias']
        if command_index <= 1:
            channel = QLabel()
            channel.setText('NA')
            self.seq.channel[seq_num] = ' '
        elif command_index == 2:
            channel = QComboBox()
            channel.addItems(channel_digital_list)
            channel.setCurrentIndex(channel_digital_list.index(self.seq.channel[seq_num]))
        elif command_index == 6:
            channel = QComboBox()
            channel.addItems(channel_adc_list)
            channel.setCurrentIndex(channel_adc_list.index(self.seq.channel[seq_num]))
        else:
            channel = QComboBox()
            channel.addItems(channel_dac_list)
            channel.setCurrentIndex(channel_dac_list.index(self.seq.channel[seq_num]))
        if command_index > 1:
            channel.currentIndexChanged.connect(lambda: self.change_channel(True))
        self.channel_widget_list[seq_num] = channel
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 1)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 1, self.channel_widget_list[seq_num])
    
    def setup_option1(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        if command_index == 1:
            option1 = QComboBox()
            option1.addItems(['SLOW', 'FAST'])
            option1.setCurrentIndex(self.seq.option1[seq_num])
            option1.currentIndexChanged.connect(self.change_option1)
        elif (command_index == 3) or (command_index == 7):
            option1 = QComboBox()
            option1.addItems(['DOWN', 'UP'])
            option1.setCurrentIndex(self.seq.option1[seq_num])
            option1.currentIndexChanged.connect(self.change_option1)
        elif (command_index == 4) or (command_index == 5):
            option1 = QCheckBox('Origin')
            option1.setChecked(self.seq.data[seq_num] == 'Origin')
            option1.stateChanged.connect(self.original)
            self.seq.option1[seq_num] = 0
        else:
            option1 = QLabel()
            option1.setText('NA')
            self.seq.option1[seq_num] = 0
        self.option1_widget_list[seq_num] = option1
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 2)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 2, self.option1_widget_list[seq_num])
    
    def setup_option2(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        if command_index == 5:
            option2 = QSpinBox()
            option2.setMaximum(2 ** 12 -1)
            option2.setMinimum(0)
            option2.setValue(self.seq.option2[seq_num])
            option2.valueChanged.connect(self.change_option2)
        elif command_index == 7:
            option2 = QSpinBox()
            option2.setMaximum(2 ** 11 -1)
            option2.setMinimum(0)
            option2.setValue(self.seq.option2[seq_num])
            option2.valueChanged.connect(self.change_option2)
        else:
            option2 = QLabel()
            option2.setText('NA')
            self.seq.option2[seq_num] = 0
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
            data.setValue(int(self.seq.data[seq_num]))
            data.valueChanged.connect(self.change_data)
            unit.setText('us')
        elif command_index == 1:
            data = QLabel()
            data.setText('NA')
            unit.setText('NA')
            self.seq.data[seq_num] = '0'
        elif command_index == 2:
            data = QComboBox()
            data.addItem('OFF')
            data.addItem('ON')
            unit.setText('NA')
            data.setCurrentIndex(int(self.seq.data[seq_num]))
            data.currentIndexChanged.connect(self.change_data)
        elif command_index == 6:
            data = QSpinBox()
            data.setMaximum(2 ** 16 -1)
            data.setMinimum(0)
            data.setValue(int(self.seq.data[seq_num]))
            data.valueChanged.connect(self.change_data)
            unit.setText('times')
        else:
            channel_index = self.channel_widget_list[seq_num].currentIndex()
            if channel_index <= 1:
                data = QSpinBox()
                data.setMaximum(2 ** 16 -1)
                data.setMinimum(0)
                if self.seq.data[seq_num] != 'Origin':
                    data.setValue(int(self.seq.data[seq_num]))
                data.valueChanged.connect(self.change_data)
                unit.setText('bits')
            elif channel_index == 2:
                data = QDoubleSpinBox()
                data.setMaximum(100.0)
                data.setMinimum(0.01)
                data.setDecimals(5)
                if self.seq.data[seq_num] != 'Origin':
                    data.setValue(float(self.seq.data[seq_num]))
                data.valueChanged.connect(self.change_data)
                unit.setText('nA')
            else:
                data = QDoubleSpinBox()
                data.setMaximum(10.0)
                data.setMinimum(-10.0)
                data.setDecimals(5)
                if self.seq.data[seq_num] != 'Origin':
                    data.setValue(float(self.seq.data[seq_num]))
                data.valueChanged.connect(self.change_data)
                unit.setText('Volts')

        self.data_widget_list[seq_num] = data
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 4)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 4, self.data_widget_list[seq_num])

        self.unit_widget_list[seq_num] = unit
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 5)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 5, self.unit_widget_list[seq_num])

    def add_step(self, seq_num, opt):
        if seq_num < 0:
            seq_num = self.table_Content_SeqEditor.currentRow()
            if seq_num < 0:
                seq_num = self.seq.seq_num
        self.table_Content_SeqEditor.insertRow(seq_num)
        if opt:
            # Expand all raw data list space
            self.seq.command.insert(seq_num, 'Wait')
            self.seq.channel.insert(seq_num, ' ')
            self.seq.option1.insert(seq_num, 0)
            self.seq.option2.insert(seq_num, 0)
            self.seq.data.insert(seq_num, '0')
            self.seq.seq_num += 1
            self.saved = False
            self.setWindowTitle('Sequence Editor*')
        
        # Command widget
        command = QComboBox()
        command_list = ['Wait', 'Match', 'Dout', 'Shift', 'Aout', 'Ramp', 'Read', 'ShiftRamp']
        command.addItems(command_list)
        command.setCurrentIndex(command_list.index(self.seq.command[seq_num]))
        command.currentIndexChanged.connect(lambda: self.change_command(-1, True))
        
        # command.setStyleSheet("QComboBox{margin:3px};")
        self.command_combo_list.insert(seq_num, command)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 0, self.command_combo_list[seq_num])

        # Expand all widget list space
        self.channel_widget_list.insert(seq_num, 'channel')
        self.option1_widget_list.insert(seq_num, 'option1')
        self.option2_widget_list.insert(seq_num, 'option2')
        self.data_widget_list.insert(seq_num, 'data')
        self.unit_widget_list.insert(seq_num, 'unit')
        
        self.change_command(seq_num, opt)
        self.table_Content_SeqEditor.setCurrentCell(seq_num, 0)

    def remove_step(self):
        seq_num = self.table_Content_SeqEditor.currentRow()
        if seq_num >= 0:
            self.table_Content_SeqEditor.removeRow(seq_num)
            
            # Remove correspoding element in widget list
            del self.command_combo_list[seq_num]
            del self.channel_widget_list[seq_num]
            del self.option1_widget_list[seq_num]
            del self.option2_widget_list[seq_num]
            del self.data_widget_list[seq_num]
            del self.unit_widget_list[seq_num]
            
            # Remove corresponding element in raw data list
            del self.seq.command[seq_num]
            del self.seq.channel[seq_num]
            del self.seq.option1[seq_num]
            del self.seq.option2[seq_num]
            del self.seq.data[seq_num]
            
            # Decrease total sequence number by 1
            self.seq.seq_num -= 1


    def clear_all(self):
        self.table_Content_SeqEditor.setRowCount(0)
        self.command_combo_list = []
        self.channel_widget_list = []
        self.option1_widget_list = []
        self.option2_widget_list = []
        self.data_widget_list = []
        self.unit_widget_list = []
        self.seq.load_data([], [], [], [], [])

    def message(self):
        QMessageBox.information(None, "Sequence compiler", "Duplicated sequence name!", QMessageBox.Ok)

    # Save current sequence
    def save_seq(self, opt):
        fname = ''
        if opt or (self.seq.path == ''):
            fname = QFileDialog.getSaveFileName(self, 'Save file', '', 'SEQ Files (*.seq)')[0]  # Save file and get file name
        else:
            fname = self.seq.path

        if fname != '':
            with open(fname, 'wb') as output:
                pickle.dump(self.seq, output, pickle.HIGHEST_PROTOCOL)
                self.seq.path = fname
                self.saved = True
                self.setWindowTitle('Sequence Editor')

    
    # !!! Not finished
    def validate(self):
        error = self.seq.validation(self.seq.feedback)
        if error == 0:
            # !!! pop out message to confirm compile
            self.seq.build()

    # !!! Not finished
    def simulate(self):
        self.setWindowTitle('title') 
        print(self.seq.path)

    # Add to list (self.edit = -1) or update list (self.edit >= 0)
    def add_to_list(self):
        if not self.seq.validated:
            QMessageBox.information(None, "Sequence compiler", "Validate first", QMessageBox.Ok)
        elif not self.saved:
            QMessageBox.information(None, "Sequence compiler", "Save first", QMessageBox.Ok)
        else:
            self.add_signal.emit(self.edit)
    
    # Do nothing
    def cancel(self):
        self.close()    


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySequenceEditor()
    window.show()
    sys.exit(app.exec_())
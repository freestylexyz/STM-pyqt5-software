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
from PyQt5.QtCore import pyqtSignal
import pandas as pd
from SequenceEditor_ui import Ui_SequenceEditor
import functools as ft
from sequence import mySequence
import pickle
import copy


class mySequenceEditor(QWidget, Ui_SequenceEditor):
    add_signal = pyqtSignal(int)            # Add sequence to list signal

    def __init__(self):
        super().__init__()
        self.idling = True
        self.setupUi(self)
        self.seq = mySequence()
        self.load_seq(self.seq, True, -1)   # Load an empty sequence
        self.init_UI()
        
    # UI initialization
    def init_UI(self):
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
        # Pushbutton
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

    # Load a sequence
    def load_seq(self, seq, enable, edit):
        # For load form list use (-1 stands for new sequence)

        self.edit = edit
        if self.edit >= 0:
            self.pushButton_AddtoList_Edit.setText('Update list')
        else:
            self.pushButton_AddtoList_Edit.setText('Add To List')
        
        self.clear_all()                # Clear all lists and table widgets
        self.seq = copy.deepcopy(seq)   # Make a copy of sequence and load

        # Loop over steps to add rows
        for i in range(self.seq.seq_num):
            self.add_step(i, False)
            
        self.setup_status(enable)       # Figure out status of other widgets
        
        # Deduce if it's a saved sequence and update view
        self.saved = (self.seq.path != '')
        if self.saved:
            self.setWindowTitle('Sequence Editor')
        else:
            self.setWindowTitle('Sequence Editor*')
        
    # Set up widgets other than the table
    def setup_status(self, enable):
        self.lineEdit_Name_SeqEditor.setText(self.seq.name)                 # Name input
        self.checkBox_FeedbackON__SeqEditor.setChecked(self.seq.feedback)   # Feedback
        self.checkBox_DitherB__SeqEditor.setChecked(self.seq.ditherB)       # Bias dither
        self.checkBox_DitherZ__SeqEditor.setChecked(self.seq.ditherZ)       # Z dither
        
        # Mode radiobutton
        self.radioButton_ReadSeq_Mode.setChecked(self.seq.mode)
        self.radioButton_Deposition_Mode.setChecked(not self.seq.mode)
        
        # Bias DAC radiobutton
        self.radioButton_20bit_Bias.setChecked(self.seq.bias_dac)
        self.radioButton_16bit_Bias.setChecked(not self.seq.bias_dac)
        
        # Preamp gain radio buttons
        if self.seq.preamp_gain == 8:
            self.radioButton_Gain8_Preamp.setChecked(True)
        if self.seq.preamp_gain == 9:
            self.radioButton_Gain9_Preamp.setChecked(True)
        if self.seq.preamp_gain == 10:
            self.radioButton_Gain10_Preamp.setChecked(True)
        
        # Enable
        self.radioButton_ReadSeq_Mode.setEnabled(enable)
        self.radioButton_Deposition_Mode.setEnabled(enable)
        self.radioButton_20bit_Bias.setEnabled(enable)
        self.radioButton_16bit_Bias.setEnabled(enable)
        self.radioButton_Gain8_Preamp.setEnabled(enable)
        self.radioButton_Gain9_Preamp.setEnabled(enable)
        self.radioButton_Gain10_Preamp.setEnabled(enable)
        
    # Preamp gian change slot
    def preamp_change(self, index, status):
        self.seq.preamp_gain = index
        
    # Mode change slot
    def mode_change(self, index, status):
        self.seq.mode = (index == 1)
        self.saved = False
        self.setWindowTitle('Sequence Editor*')
        
    # Bias DAC selection change
    def bias_mode_change(self, index, status):
        self.seq.bias_dac = (index == 1)
        
    # Feedback, Bias dither and Z dither change slot
    def digital_change(self, ch, status):
        if ch == 0:
            self.seq.ditherB = (status != 0)
        elif ch == 1:
            self.seq.ditherZ = (status != 0)
        elif ch == 2:
            self.seq.feedback = (status != 0)
        self.change()
        
    # Sequence name change slot (no need to validate)
    def name_change(self):
        self.seq.name = self.lineEdit_Name_SeqEditor.text()
        self.saved = False
        self.setWindowTitle('Sequence Editor*')
        
    # Store changed command and set default for this command
    def command_default(self, seq_num):
        self.seq.command[seq_num] = self.command_combo_list[seq_num].currentText()                  # Store changed command
        command_index = self.command_combo_list[seq_num].currentIndex()                             # Get command
        # Default lists
        channel_default_list = [' ', ' ', 'Feedback', 'Bias', 'Bias', 'Bias', 'PREAMP', 'Bias']
        option1_default_list = [0] * 8
        option2_default_list = [0] * 5 + [0, 1, 0]
        data_default_list = ['0', '0', '0', '0.0', '0.0', '0.0', '0', '0.0']
        # Set default based on command
        self.seq.channel[seq_num] = channel_default_list[command_index]
        self.seq.option1[seq_num] = option1_default_list[command_index]
        self.seq.option2[seq_num] = option2_default_list[command_index]
        self.seq.data[seq_num] = data_default_list[command_index]
        
    # Set default data a analog output command when change channel
    def channel_default(self, seq_num):
        channel_index = self.channel_widget_list[seq_num].currentIndex()                    # Get channel
        data_default_list = ['0', '0', '0.1', '0.0', '0.0', '0.0', '0.0', '0.0', '0.0']     # Default data list
        
        # Only update data for analog output command
        command_index = self.command_combo_list[seq_num].currentIndex()
        flag = (command_index == 3) or (command_index == 4) or (command_index == 5) or (command_index == 7)
        if flag:
            self.seq.data[seq_num] = data_default_list[channel_index]
    
    # If anything changed
    def change(self):
        self.saved = False                          # Toggle saved flag
        self.setWindowTitle('Sequence Editor*')     # Change title for indication
        self.seq.validated = False                  # Toggle validation flag
    
    # Command change slot
    # It also initial row setup
    def change_command(self, seq_num, opt):
        if seq_num < 0:                     # Initial row setup
            seq_num = self.table_Content_SeqEditor.currentRow()
        if opt:                             # Command change
            self.command_default(seq_num)   # Load default data
            self.change()
        
        # Set up the entire row
        self.setup_channel(seq_num)
        self.setup_option1(seq_num)
        self.setup_option2(seq_num)
        self.setup_data_unit(seq_num)
        
    # Channel change slot
    def change_channel(self):
        seq_num = self.table_Content_SeqEditor.currentRow()                             # Current row
        self.channel_default(seq_num)                                                   # Default data for analog output
        self.seq.channel[seq_num] = self.channel_widget_list[seq_num].currentText()     # Update channel list
        
        # Set up rest of the row
        self.setup_option1(seq_num)
        self.setup_option2(seq_num)
        self.setup_data_unit(seq_num)
        self.change()
    
    # Option1 change slot
    def change_option1(self):
        seq_num = self.table_Content_SeqEditor.currentRow()
        self.seq.option1[seq_num] = self.option1_widget_list[seq_num].currentIndex()    # Update option1 list
        self.change()
    
    # Option2 change slot
    def change_option2(self):
        seq_num = self.table_Content_SeqEditor.currentRow()
        self.seq.option2[seq_num] = self.option2_widget_list[seq_num].value()           # Update option2 list
        self.change()
        
    # Data change slot
    def change_data(self):
        seq_num = self.table_Content_SeqEditor.currentRow()
        # Update data list
        if self.command_combo_list[seq_num].currentIndex() == 2:
            self.seq.data[seq_num] = str(self.data_widget_list[seq_num].currentIndex())
        else:
            self.seq.data[seq_num] = str(self.data_widget_list[seq_num].value())
        self.change()
    
    # Origin checkbox slot
    def original(self, state):
        seq_num = self.table_Content_SeqEditor.currentRow()
        
        # Update data list and change view
        if state == 2:      # Checked
            self.data_widget_list[seq_num].setEnabled(False)
            self.seq.data[seq_num] = 'Origin'
        elif state == 0:    # Unchecked
            self.data_widget_list[seq_num].setEnabled(True)
            self.seq.data[seq_num] = str(self.data_widget_list[seq_num].value())
        self.change()

    # Set up channel widgets
    def setup_channel(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        
        # Combo box list for different cases
        channel_digital_list = ['DitherB', 'DitherZ', 'Feedback']                                                   # Digital output
        channel_adc_list = ['PREAMP', 'ZOUT', 'AIN0', 'AIN1', 'AIN2', 'AIN3', 'AIN4', 'AIN5']                       # Analog input
        channel_dac_list = ['Z offset fine', 'Z offset', 'Iset', 'DAC6', 'DAC7', 'DAC8', 'DAC9', 'DAC11', 'Bias']   # Analog output
        
        # Set up channel widget
        if command_index <= 1:
            channel = QLabel()      # Other cases, no channel selection
            channel.setText('NA')
            self.seq.channel[seq_num] = ' '
        elif command_index == 2:    # Digital output
            channel = QComboBox()
            channel.addItems(channel_digital_list)
            channel.setCurrentIndex(channel_digital_list.index(self.seq.channel[seq_num]))
        elif command_index == 6:    # Analog input
            channel = QComboBox()
            channel.addItems(channel_adc_list)
            channel.setCurrentIndex(channel_adc_list.index(self.seq.channel[seq_num]))
        else:                       # Analog output
            channel = QComboBox()
            channel.addItems(channel_dac_list)
            channel.setCurrentIndex(channel_dac_list.index(self.seq.channel[seq_num]))
            
        # Connect signal if needed
        if command_index > 1:
            channel.currentIndexChanged.connect(self.change_channel)
            
        # Add widget to list and table
        self.channel_widget_list[seq_num] = channel
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 1)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 1, self.channel_widget_list[seq_num])
    
    # Set up option1 widget
    def setup_option1(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        
        # Set up option1 widget
        if command_index == 1:                                  # Match current
            option1 = QComboBox()
            option1.addItems(['SLOW', 'FAST'])
            option1.setCurrentIndex(self.seq.option1[seq_num])
            option1.currentIndexChanged.connect(self.change_option1)
        elif (command_index == 3) or (command_index == 7):      # Shift and shift ramp
            option1 = QComboBox()
            option1.addItems(['DOWN', 'UP'])
            option1.setCurrentIndex(self.seq.option1[seq_num])
            option1.currentIndexChanged.connect(self.change_option1)
        elif (command_index == 4) or (command_index == 5):      # Aout and ramp
            option1 = QCheckBox('Origin')
            option1.setChecked(self.seq.data[seq_num] == 'Origin')
            option1.stateChanged.connect(self.original)
            self.seq.option1[seq_num] = 0
        else:                                                   # Others
            option1 = QLabel()
            option1.setText('NA')
            self.seq.option1[seq_num] = 0
            
        # Add widget to list and table
        self.option1_widget_list[seq_num] = option1
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 2)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 2, self.option1_widget_list[seq_num])
    
    # Set up option2 widget
    def setup_option2(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        
        # Set up option2 widget
        if command_index == 5:          # Ramp
            option2 = QSpinBox()
            option2.setMaximum(2 ** 12 -1)
            option2.setMinimum(1)
            option2.setValue(self.seq.option2[seq_num])
            option2.valueChanged.connect(self.change_option2)
        elif command_index == 7:        # Shift ramp
            option2 = QSpinBox()
            option2.setMaximum(2 ** 11 -1)
            option2.setMinimum(1)
            option2.setValue(self.seq.option2[seq_num])
            option2.valueChanged.connect(self.change_option2)
        else:                           # Others
            option2 = QLabel()
            option2.setText('NA')
            self.seq.option2[seq_num] = 0
            
        # Add widget to list and table
        self.option2_widget_list[seq_num] = option2
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 3)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 3, self.option2_widget_list[seq_num])
        
    # Set up data and unit widget
    def setup_data_unit(self, seq_num):
        command_index = self.command_combo_list[seq_num].currentIndex()
        
        # Set up data and unit widget
        unit = QLabel()
        if command_index == 0:      # Wait
            data = QSpinBox()
            data.setMaximum(2 ** 31 -1)
            data.setMinimum(0)
            data.setValue(int(self.seq.data[seq_num]))
            data.valueChanged.connect(self.change_data)
            unit.setText('us')
        elif command_index == 1:    # Match
            data = QLabel()
            data.setText('NA')
            unit.setText('NA')
            self.seq.data[seq_num] = '0'
        elif command_index == 2:    # Dout
            data = QComboBox()
            data.addItem('OFF')
            data.addItem('ON')
            unit.setText('NA')
            data.setCurrentIndex(int(self.seq.data[seq_num]))
            data.currentIndexChanged.connect(self.change_data)
        elif command_index == 6:    # Read
            data = QSpinBox()
            data.setMaximum(2 ** 16 -1)
            data.setMinimum(0)
            data.setValue(int(self.seq.data[seq_num]))
            data.valueChanged.connect(self.change_data)
            unit.setText('times')
        else:                       # Analog output
            channel_index = self.channel_widget_list[seq_num].currentIndex()
            if channel_index <= 1:      # Bit data
                data = QSpinBox()
                data.setMaximum(2 ** 16 -1)
                data.setMinimum(0)
                if self.seq.data[seq_num] != 'Origin':          # Not back to origin
                    data.setValue(int(self.seq.data[seq_num]))
                else:                                           # Back to origin
                    self.option1_widget_list[seq_num].setChecked(True)
                data.valueChanged.connect(self.change_data)
                unit.setText('bits')
            elif channel_index == 2:    # nA data
                data = QDoubleSpinBox()
                data.setMaximum(100.0)
                data.setMinimum(0.01)
                data.setDecimals(5)
                if self.seq.data[seq_num] != 'Origin':
                    data.setValue(float(self.seq.data[seq_num]))
                else:
                    self.option1_widget_list[seq_num].setChecked(True)
                data.valueChanged.connect(self.change_data)
                unit.setText('nA')
            else:                       # Volts data
                data = QDoubleSpinBox()
                data.setMaximum(10.0)
                data.setMinimum(-10.0)
                data.setDecimals(5)
                if self.seq.data[seq_num] != 'Origin':
                    data.setValue(float(self.seq.data[seq_num]))
                else:
                    self.option1_widget_list[seq_num].setChecked(True)
                data.valueChanged.connect(self.change_data)
                unit.setText('Volts')

        # Add widget to list and table
        self.data_widget_list[seq_num] = data
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 4)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 4, self.data_widget_list[seq_num])

        self.unit_widget_list[seq_num] = unit
        self.table_Content_SeqEditor.removeCellWidget(seq_num, 5)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 5, self.unit_widget_list[seq_num])

    # Add step
    def add_step(self, seq_num, opt):
        
        # seq_num = -1 use current row
        if seq_num < 0:
            seq_num = self.table_Content_SeqEditor.currentRow()
            if seq_num < 0:                 # No current row
                seq_num = self.seq.seq_num  # Use last row
                
        self.table_Content_SeqEditor.insertRow(seq_num)     # Insert row
        
        # Add new step instead load step
        if opt:
            # Expand all raw data list space
            self.seq.command.insert(seq_num, 'Wait')
            self.seq.channel.insert(seq_num, ' ')
            self.seq.option1.insert(seq_num, 0)
            self.seq.option2.insert(seq_num, 0)
            self.seq.data.insert(seq_num, '0')
            self.seq.seq_num += 1
            self.change()
        
        # Set up command widget
        command = QComboBox()
        command_list = ['Wait', 'Match', 'Dout', 'Shift', 'Aout', 'Ramp', 'Read', 'ShiftRamp']
        command.addItems(command_list)
        command.setCurrentIndex(command_list.index(self.seq.command[seq_num]))
        command.currentIndexChanged.connect(lambda: self.change_command(-1, True))
        
        # Add to list and table
        self.command_combo_list.insert(seq_num, command)
        self.table_Content_SeqEditor.setCellWidget(seq_num, 0, self.command_combo_list[seq_num])

        # Expand all widget list space
        self.channel_widget_list.insert(seq_num, 'channel')
        self.option1_widget_list.insert(seq_num, 'option1')
        self.option2_widget_list.insert(seq_num, 'option2')
        self.data_widget_list.insert(seq_num, 'data')
        self.unit_widget_list.insert(seq_num, 'unit')
        
        # Set up rest of the cells
        self.change_command(seq_num, opt)
        self.table_Content_SeqEditor.setCurrentCell(seq_num, 0)

    # Remove current step
    def remove_step(self):
        seq_num = self.table_Content_SeqEditor.currentRow()
        
        if seq_num >= 0:                                        # Valid row number
            self.table_Content_SeqEditor.removeRow(seq_num)
            
            # Remove corresponding element in widget list
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
            self.change()

    # Clear all steps
    def clear_all(self):
        self.table_Content_SeqEditor.setRowCount(0)
        self.command_combo_list = []
        self.channel_widget_list = []
        self.option1_widget_list = []
        self.option2_widget_list = []
        self.data_widget_list = []
        self.unit_widget_list = []
        self.seq.load_data([], [], [], [], [])

    # Duplicate name message, called by sequence list
    def message(self):
        QMessageBox.information(None, "Sequence compiler", "Duplicated sequence name!", QMessageBox.Ok)

    # Save current sequence
    def save_seq(self, opt):
        fname = ''
        if opt or (self.seq.path == ''):        # No path or save as
            fname = QFileDialog.getSaveFileName(self, 'Save file', '', 'SEQ Files (*.seq)')[0]  # Save file and get file name
        else:                                   # Direct save
            fname = self.seq.path

        if fname != '':                         # Savable
            with open(fname, 'wb') as output:
                pickle.dump(self.seq, output, pickle.HIGHEST_PROTOCOL)      # Save sequence
                self.seq.path = fname                                       # Save path
                self.saved = True
                self.setWindowTitle('Sequence Editor')

    # !!! Not finished
    def validate(self):
        if not self.saved:
            QMessageBox.information(None, "Sequence compiler", "Save first", QMessageBox.Ok)
        else:
            error = self.seq.validation(self.seq.ditherB, self.seq.ditherZ, self.seq.feedback, self.seq.mode)
            self.save_seq(False)
            # QMessageBox.information(None, "Sequence compiler", "Save first", QMessageBox.Ok)

    # !!! Not finished
    def simulate(self):
        print('save', self.saved)
        print('Name', self.seq.name)
        print('Mode', self.seq.mode)
        print('Sequence number:', self.seq.seq_num)
        print('Read number:', self.seq.read_num)
        print('Feedback:', self.seq.feedback)
        print('DitherB:', self.seq.ditherB)
        print('DitherZ:', self.seq.ditherZ)
        print('Validation:', self.seq.validated)
        print('Path:', self.seq.path)
        # print(len(self.seq.command), len(self.seq.option1), len(self.seq.option2), len(self.seq.data), len(self.seq.command_list), len(self.seq.data_list))
        
        if self.seq.validated:
            d = {'Command': self.seq.command, 'Channel': self.seq.channel, 'Option1': self.seq.option1, 'Option2': self.seq.option2, 'Data': self.seq.data,\
                 'Command list': self.seq.command_list, 'Data list': self.seq.data_list}
        else:
            d = {'Command': self.seq.command, 'Channel': self.seq.channel, 'Option1': self.seq.option1, 'Option2': self.seq.option2, 'Data': self.seq.data}
        print(pd.DataFrame(data = d))

    # Add to list (self.edit = -1) or update list (self.edit >= 0)
    def add_to_list(self):
        if not self.seq.validated:
            QMessageBox.information(None, "Sequence compiler", "Validate first", QMessageBox.Ok)
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
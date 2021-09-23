# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:19:17 2020

@author: yaoji
"""

import sys
sys.path.append("../ui/")
sys.path.append("../Model/")
sys.path.append("../Scan/")
from PyQt5.QtWidgets import QApplication , QWidget, QMessageBox, QFileDialog
from PyQt5.QtCore import pyqtSignal, QRectF
from Deposition_ui import Ui_Deposition
from DepositionInfo import myDepositionInfo
from DataStruct import DepData
import functools as ft
import conversion as cnv
import pickle
from datetime import datetime

class myDeposition(QWidget, Ui_Deposition):
    close_signal = pyqtSignal()                     # Close deposition window signal
    seq_list_signal = pyqtSignal(str)               # Open sequence list window signal
    do_it_signal = pyqtSignal(list, list, list)     # Do deposition signal
    stop_signal = pyqtSignal()                      # Stop continues reading signal
    
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.info = myDepositionInfo()          # Deposition data information window
        self.dlg = QFileDialog()                # File dialog window
        self.data = DepData()                   # Spectroscopy data
        
        # For automatically assign file name
        self.today = datetime.now().strftime("%m%d%y")
        self.file_index = 0
        
        # System status
        self.bias_dac = False   # Bias DAC selection
        self.bias_ran = 9       # Bias DAC range
        self.idling = True      # Idling status
        self.saved = True       # Saved status
        
        # For data update
        self.limit = 65536      # Limit for change detection
        self.stop_num = 0       # Stop number storage
        self.rdata = []         # Continuous data storage
        self.count = 0          # Count after change detected

        # Set up file dialog window
        self.dlg.setNameFilter('DEP Files (*.dep)')
        self.dlg.setFileMode(QFileDialog.AnyFile)
        self.dlg.setAcceptMode(QFileDialog.AcceptSave)
        
        # Connect signal
        
        # Mode change
        self.groupBox_ReadNSample_Deposition.toggled.connect(ft.partial(self.read_mode, True))
        self.groupBox_ReadUntil_Deposition.toggled.connect(ft.partial(self.read_mode, False))
        self.radioButton_Continuous_ReadUntil.toggled.connect(ft.partial(self.read_mode, False))
        self.radioButton_NSample_ReadUntil.toggled.connect(ft.partial(self.read_mode, False))
        self.groupBox_Seq_Deposition.toggled.connect(self.seq_mode)
        
        # Conversions
        self.spinBox_DeltaZ_PokeTip.valueChanged.connect(self.scrollBar_DeltaZ_PokeTip.setValue)
        self.scrollBar_DeltaZ_PokeTip.valueChanged.connect(self.spinBox_DeltaZ_PokeTip.setValue)
        self.spinBox_Bias_PokeTip.valueChanged.connect(ft.partial(self.bias_cnv, True))
        self.scrollBar_Bias_PokeTip.valueChanged.connect(ft.partial(self.bias_cnv, False))
        
        # Buttons
        self.pushButton_DoIt_Deposition.clicked.connect(self.do_it)                 # Do it button
        self.pushButton_Save_Deposition.clicked.connect(self.save)                  # Save data
        self.pushButton_SeqEditor_Deposition.clicked.connect(self.seq_list_slot)    # Open sequence list window
        self.pushButton_Info_Deposition.clicked.connect(self.open_Info)             # Open data info window

        # graphicsView
        self.plot_before = self.graphicsView_Before_Deposition.addPlot()
        self.plot_during = self.graphicsView_During_Deposition.addPlot()
        self.plot_after = self.graphicsView_After_Deposition.addPlot()

        self.view_box_before = self.plot_before.getViewBox()
        self.view_box_during = self.plot_during.getViewBox()
        self.view_box_after = self.plot_after.getViewBox()

        self.view_box_before.setMouseEnabled(x=False, y=False)
        self.view_box_before.setMenuEnabled(False)
        self.view_box_before.register('before')
        self.before_bounds = self.view_box_before.autoRange()

        self.view_box_during.setMouseEnabled(x=True, y=True)
        self.view_box_during.setMenuEnabled(False)

        self.view_box_after.setMouseEnabled(x=False, y=False)
        self.view_box_after.setMenuEnabled(False)
        self.view_box_after.register('after')
        self.after_bounds = self.view_box_after.autoRange()

        self.plot_during.setClipToView(True)

        self.before_curve = self.plot_before.plot()
        self.during_curve = self.plot_during.plot()
        self.after_curve = self.plot_after.plot()

    # Open sequence list signal
    def seq_list_slot(self):
        self.seq_list_signal.emit(self.label_Seq_Deposition.text())
        
    # Called with int_scan
    def init_deposition(self, succeed, bias_dac, bias_ran, seq_list, selected):
        # Variables
        self.bias_dac = bias_dac
        self.bias_ran = bias_ran

        # Enable buttons based on succeed
        self.pushButton_DoIt_Deposition.setEnabled(succeed)
        # self.pushButton_Save_Deposition.setEnabled(not self.data.data)
        # self.pushButton_Info_Deposition.setEnabled(not self.data.data)

        # Setup controls, make sure delta bias is within range & config value is recorded
        config_deltaB_val = self.spinBox_Bias_PokeTip.value()
        if bias_dac:
            self.scrollBar_Bias_PokeTip.setMaximum(0xfffff)
            config_deltaB_bit = cnv.vb(self.spinBox_Bias_PokeTip.value(), '20')
            if config_deltaB_bit > 0xfffff:
                self.scrollBar_Bias_PokeTip.setValue(0x80000)
            else:
                self.scrollBar_Bias_PokeTip.setValue(config_deltaB_bit)
            self.spinBox_Bias_PokeTip.setMaximum(5.0)
            self.spinBox_Bias_PokeTip.setMinimum(-5.0)
            if config_deltaB_bit > 0xfffff:
                self.spinBox_Bias_PokeTip.setValue(0.0)
            else:
                self.spinBox_Bias_PokeTip.setValue(config_deltaB_val)
        else:
            self.scrollBar_Bias_PokeTip.setMaximum(0xffff)
            config_deltaB_bit = cnv.vb(config_deltaB_val, 'd', self.bias_ran)
            if config_deltaB_bit > 0xffff:
                self.scrollBar_Bias_PokeTip.setValue(0x8000)
            else:
                self.scrollBar_Bias_PokeTip.setValue(config_deltaB_bit)
            self.bias_ran_change(bias_ran)

        # Set selected sequence name
        seq_name = '' if selected < 0 else seq_list[selected].name
        self.label_Seq_Deposition.setText(seq_name)
        
    # Read mode slot
    # Can not have multiple read mode on
    def read_mode(self, mode, state):
        if mode:    # Read N-sample
            if state:
                self.groupBox_ReadUntil_Deposition.setChecked(False)
                self.label_sampling_delay.setText('Sampling delay (us)')
                self.spinBox_Delay_Pulse.setMinimum(0)
                self.spinBox_Delay_Pulse.setValue(0)
        else:       # Read until change happen
            if state:
                self.groupBox_ReadNSample_Deposition.setChecked(False)
                if self.radioButton_Continuous_ReadUntil.isChecked():   # Continuous
                    self.label_sampling_delay.setText('Sampling delay (ms)')
                    self.spinBox_Delay_Pulse.setMinimum(2)
                else:   # N-sample
                    self.label_sampling_delay.setText('Sampling delay (us)')
                    self.spinBox_Delay_Pulse.setMinimum(0)
                    self.spinBox_Delay_Pulse.setValue(0)
    
    # Sequence mode slot
    def seq_mode(self, state):
        self.groupBox_Poke_Deposition.setEnabled(not state)
    
    # Enable serial related components
    def enable_serial(self, enable):
        self.groupBox_Seq_Deposition.setEnabled(enable)
        self.groupBox_Read_Deposition.setEnabled(enable)
        self.groupBox_ReadPulse_Deposition.setEnabled(enable)
        self.pushButton_DoIt_Deposition.setEnabled(enable)
        # self.pushButton_Save_Deposition.setEnabled(enable and (not self.data.data))
        # self.pushButton_Info_Deposition.setEnabled(enable and (not self.data.data))
        mode = not self.groupBox_Poke_Deposition.isChecked()
        self.groupBox_Seq_Deposition.setEnabled(enable and mode)   # Poke group box is also determined by if using sequence
        self.view_box_before.setMouseEnabled(x=enable, y=enable)   # Enable and disable mouse event
        self.view_box_during.setMouseEnabled(x=enable, y=enable)  # Enable and disable mouse event
        self.view_box_after.setMouseEnabled(x=enable, y=enable)  # Enable and disable mouse event
    
    # Bias conversion slot
    def bias_cnv(self, flag, value):
        if flag:    # Spinbox to scrollbar
            if self.bias_dac:
                out = cnv.vb(value, '20')
            else:
                out = cnv.vb(value, 'd', self.bias_ran)
            self.scrollBar_Bias_PokeTip.setValue(out)
        else:       # Scrollbar to spin box
            if self.bias_dac:
                out = cnv.bv(value, '20')
            else:
                out = cnv.bv(value, 'd', self.bias_ran)
            self.spinBox_Bias_PokeTip.setValue(out)
            
    # Configure read before options
    def configure_read_before(self):
        # Read before and after
        if self.groupBox_Read_Deposition.isChecked():
            read_before_ch = (self.comboBox_Ch_Read.currentIndex() + 6) * 4 + 0xc0          # Read channel
            read_before_avg = self.spinBox_AvgNum_Read.value()                              # Average number for each sample
            read_before_num = self.spinBox_Num_Read.value()                                 # Number of samples
            read_before_delay = self.spinBox_Delay_Read.value()                             # Delay between 2 neighbouring samples
            return [read_before_ch, read_before_num, read_before_avg, read_before_delay]
        else:
            return []
    
    # Configure read options
    def configure_read(self):
        # Read pulse
        read_ch = (self.comboBox_Ch_Pulse.currentIndex() + 6) * 4 + 0xc0            # Read channel
        read_delay = self.spinBox_Wait_Pulse.value()                                # Delay waited before executing reading procedure
        read_avg = self.spinBox_AvgNum_Pulse.value()                                # Average number for each sample
        read_delay2 = self.spinBox_Delay_Pulse.value()                              # Delay between 2 neighbouring samples
        read_num = self.spinBox_Num_ReadNSample.value()                             # Number of samples
        read_change = cnv.vb((self.spinBox_Change_ReadUntil.value() - 10.24), 'a')  # Limit for change detection
        read_stop_num = self.spinBox_StopNum_ReadUntil.value()                      # Stop number
        
        # Configure read mode
        if not (self.groupBox_ReadNSample_Deposition.isChecked() or self.groupBox_ReadUntil_Deposition.isChecked()):
            read_mode = 0   # No read
        elif self.groupBox_ReadNSample_Deposition.isChecked():
            read_mode = 3   # Read N samples
        elif self.radioButton_NSample_ReadUntil.isChecked():
            read_mode = 2   # Read N samples until
        elif self.radioButton_Continuous_ReadUntil.isChecked():
            read_mode = 1   # Continuous read
        
        # Init continuous data storage
        self.limit = read_change
        self.stop_num = read_stop_num
        self.count = 0
            
        return [read_ch, read_mode, read_delay, read_delay2, read_num, read_avg, read_change, read_stop_num]
    
    # Configure sequence
    def configure_sequence(self):
        # Sequence
        if not self.groupBox_Seq_Deposition.isChecked():
            deltaZ = self.scrollBar_DeltaZ_PokeTip.value()
            # Feedback OFF, Shift Z offset, Output bias voltage
            return [0, 0x80000000 * (deltaZ >= 0) + abs(deltaZ), self.scrollBar_Bias_PokeTip.value()]
        else:
            return []

    # Do it slot
    def do_it(self):
        flag = self.saved        # If able to start flag
        
        # Ask if overwrite data if not saved
        if self.idling and (not flag):
            msg = QMessageBox.question(None, "Deposition", "Data not saved, do you want to continue?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            flag = (msg == QMessageBox.Yes)
            
        read_before = self.configure_read_before()
        read = self.configure_read()
        poke_data = self.configure_sequence()

        if self.idling and flag:                                    # Start case if idling and able to start
            self.do_it_signal.emit(read_before, read, poke_data)    # Emit spectroscopy start signal
            self.rdata = []
        elif not self.idling:                                       # Stop case if not idling
            self.pushButton_DoIt_Deposition.setEnabled(False)       # Disable stop button to avoid sending stop signal twice
            self.stop_signal.emit()                                 # Emit stop signal

    # Bias range change slot
    def bias_ran_change(self, ran):
        self.bias_ran = ran
        val = self.spinBox_Bias_PokeTip.value()
        ran_dict = {9: 5.0, 10: 10.0, 15: 2.5}                                  # Voltage limit for 16bit bias DAC
        if not self.bias_dac:
            self.spinBox_Bias_PokeTip.setMaximum(ran_dict[self.bias_ran])       # Set maximum
            self.spinBox_Bias_PokeTip.setMinimum(-ran_dict[self.bias_ran])      # Set minimum
            self.bias_cnv(True, val)                                            # Set stored value

    # Open Information window
    def open_Info(self):
        self.info.init_depInfo(self.data)
        self.info.show()

    # Save data slot
    def save(self):
        if self.data.time.strftime("%m%d%y") != self.today:             # New day, init file_index
            self.today = self.data.time.strftime("%m%d%y")
            self.file_index = 0
        name_list = '0123456789abcdefghijklmnopqrstuvwxyz'              # Name list
        name = self.today + name_list[self.file_index // 36] + name_list[self.file_index % 36]  # Auto configure file name
        self.dlg.selectFile(self.dlg.directory().path() + '/' + name + '.dep')                  # Set default file name as auto configured
        
        if self.dlg.exec_():                                            # File selected
            fname = self.dlg.selectedFiles()[0]                                                 # File path
            directory = self.dlg.directory().path()                                             # Directory path
            self.dlg.setDirectory(directory)                                                    # Set directory path for next call
            save_name = fname.replace(directory + '/', '').replace('.dep', '')                  # Get the real file name
            
            # If default file name is not used
            if save_name != name:
                try:                                        # See if current file name is in our naming system
                    if save_name[0:6] == self.today:        # Reset file index if match
                        self.file_index = name_list.index(save_name[6]) * 36 + name_list.index(save_name[7])
                    else:
                        self.file_index -= 1
                except:                                     # Otherwise do not consume file index
                    self.file_index -= 1
                    
            self.saved = True                                               # Toggle saved flag
            self.setWindowTitle('Deposition-' + save_name)                  # Change window title for saving status indication
            self.file_index += 1                                            # Consume 1 file index
            with open(fname, 'wb') as output:
                self.data.path = fname                                      # Save path
                pickle.dump(self.data, output, pickle.HIGHEST_PROTOCOL)     # Save data

    # Pop out message
    def message(self, text):
        QMessageBox.warning(None, "Deposition", text, QMessageBox.Ok)

    # Update N samples data
    def update_N(self, rdata, index):
        if index == 0:                          # Update read before
            self.before_curve.setData(rdata)    # Plot read before data
            self.before_bounds = self.view_box_before.autoRange()
        elif index == 1:                        # Update N sample measurement
            self.during_curve.setData(rdata)    # Plot N sample data
        elif index == 2:                        # Update read after
            self.after_curve.setData(rdata)     # Plot read after data
            self.after_bounds = self.view_box_after.autoRange()
        self.autoRange()

    # N-sample | convert bits to volts
    def cnv2volt(self, rdata, ch_range):
        volt_data = [cnv.bv(data, 'a', ch_range) for data in rdata]
        return volt_data

    # N-sample | convert bits to volts
    def autoRange(self):
        if (self.before_bounds is not None) and (self.after_bounds is not None):
            left, bottom = min(self.before_bounds.x(), self.after_bounds.x()), min(self.before_bounds.y(), self.after_bounds.y())
            width = max(self.before_bounds.width() + self.before_bounds.x(), self.after_bounds.width() + self.after_bounds.x()) - left
            height = max(self.before_bounds.height() + self.before_bounds.y(), self.after_bounds.height() + self.after_bounds.y()) - bottom
            new_bounds = QRectF(left, bottom, width, height)
            self.view_box_before.setRange(new_bounds, padding=None)
            self.view_box_after.setRange(new_bounds, padding=None)

    # Update continuous data
    def update_C(self, rdata):
        self.rdata += [rdata]                                                   # Add read data to storage
        if (abs(rdata - self.rdata[0]) > self.limit) or (self.count != 0):      # Change detected or change already detected
            self.count += 1                                                     # Increment count
            if self.count >= self.stop_num:                                     # Emit stop signal if have enough data
                self.stop_signal.emit()

        self.during_curve.setData(self.rdata)                                   # Plot continuous measure data
        self.during_bounds = self.view_box_during.autoRange()
        new_bounds = QRectF(self.during_bounds.x()+self.during_bounds.width()-500, self.during_bounds.y(), 500, self.during_bounds.height())
        self.view_box_during.setRange(new_bounds, padding=None)                 # Reserve 500 data points in view
    
    # Emit close signal
    def closeEvent(self, event):
        if self.idling:
            self.close_signal.emit()
            event.accept()
        else:
            self.message('Process ongoing')
            event.ignore()
        


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myDeposition()
    window.show()
    sys.exit(app.exec_())
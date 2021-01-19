# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:19:02 2020

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
from PyQt5.QtWidgets import QApplication , QWidget, QFileDialog, QMessageBox
from PyQt5.QtCore import pyqtSignal , Qt
from Spectroscopy_ui import Ui_Spectroscopy
from AdvanceOption import myAdvanceOption
from SpectroscopyInfo import mySpectroscopyInfo
from DataStruct import SpcData

import conversion as cnv
import functools as ft
import pickle
from datetime import datetime

class mySpc(QWidget, Ui_Spectroscopy):
    # Control signal
    close_signal = pyqtSignal()         # Close window signal
    spectroscopy_signal = pyqtSignal()  # Spectroscopy start scan signal
    stop_signal = pyqtSignal()          # Spectroscopy stop scan signal
    seq_list_signal = pyqtSignal(str)   # Open sequence list signal

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()
    
    # Setup UI
    def init_UI(self):
        self.adv = myAdvanceOption()        # Advance option window
        self.info = mySpectroscopyInfo()    # Spectroscopy data information window
        self.dlg = QFileDialog()            # File dialog window
        self.data = SpcData()               # Spectroscopy data
        
        # Set up file dialog window
        self.dlg.setFileMode(QFileDialog.AnyFile)
        self.dlg.setAcceptMode(QFileDialog.AcceptSave)
        self.dlg.setNameFilter('SPC Files (*.spc)')
        
        # For automatically assign file name
        self.today = datetime.now().strftime("%m%d%y")
        self.file_idex = 0
        
        # System status
        self.bias_dac = False   # Bias DAC selection
        self.bias_ran = 9       # Bias DAC range
        self.idling = True      # Idling status
        self.saved = False      # Saved status
        
        # Open window
        self.pushButton_Info.clicked.connect(self.open_info)
        self.pushButton_Advance.clicked.connect(self.adv.show)
        
        # Conversion
        # Spinboxes
        self.spinBox_Min_General.editingFinished.connect(lambda: self.min_cnv(True, 0))
        self.spinBox_Max_General.editingFinished.connect(lambda: self.max_cnv(True, 0))
        self.spiBox_StepSize_General.editingFinished.connect(lambda: self.step_cnv(True, 0))
        self.spinBox_Bias_Delta.editingFinished.connect(lambda: self.bias_cnv(True, 0))
        self.spinBox_Delta_Z.editingFinished.connect(lambda: self.scrollBar_Z_Delta.setValue(self.spinBox_Delta_Z.value()))
        # Scrollbars
        self.scrollBar_Min_General.valueChanged.connect(ft.partial(self.min_cnv, False))
        self.scrollBar_Max_General.valueChanged.connect(ft.partial(self.max_cnv, False))
        self.scrollBar_StepSize_General.valueChanged.connect(ft.partial(self.step_cnv, False))
        self.scrollBar_Bias_Delta.valueChanged.connect(ft.partial(self.bias_cnv, False))
        self.scrollBar_Z_Delta.valueChanged.connect(self.spinBox_Delta_Z.setValue)
        
        # Status change
        self.comboBox_RampCh_General.currentIndexChanged.connect(self.channel_change)
        
        # Buttons
        self.pushButton_Save.clicked.connect(self.save)
        self.pushButton_Scan.clicked.connect(self.scan_emit)
        
        # Sequence
        self.adv.pushButton_SeqEditor.clicked.connect(self.open_seq_list)
        
        # Graph
        self.scrollBar_Channel_Graph.valueChanged.connect(lambda: self.label_Channel_Graph.setText(str(self.scrollBar_Channel_Graph.value())))
        self.scrollBar_point_Graph.valueChanged.connect(lambda: self.label_point_Graph.setText(str(self.scrollBar_point_Graph.value())))

    # Initial spectroscopy window
    def init_spc(self, succeed, bias_dac, bias_ran):
        # Variables
        self.bias_dac = bias_dac
        self.bias_ran = bias_ran
        
        # Controls
        self.pushButton_Scan.setEnabled(succeed)
        self.setup_spin(0)
        self.setup_scroll(0)
        self.min_cnv(False, 0)
        self.max_cnv(False, 0xfffff)
        self.step_cnv(False, 1)
        
    # Emit open sequence list signal
    def open_seq_list(self):
        self.seq_list_signal.emit(self.adv.label_Seq.text())    # Get current selected sequence name
        
    # Emit scan signal
    def scan_emit(self):
        flag = self.saved        # If able to start flag
        
        # Ask if overwrite data if not saved
        if self.idling and (not flag):
            msg = QMessageBox.question(None, "Spectroscopy", "Spectrum not saved, do you want to continue?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            flag = (msg == QMessageBox.Yes)
            
        if self.idling and flag:    # Start case if idling and able to start
            self.spectroscopy_signal.emit()         # Emit spectroscopy start signal
        elif not self.idling:       # Stop case if not idling
            self.pushButton_Scan.setEnabled(False)  # Disable stop button to avoid sending stop signal twice
            self.stop_signal.emit()                 # Emit stop signal
        
    # !!!
    # Update spectroscopy data
    def update_spc_(self, rdata):
        pass
    
    # !!!
    # Update spectroscopy data
    def update_spc(self):
        pass

    # !!!
    # Open spectroscopy information window
    def open_info(self):
        # self.info.init_info()
        self.info.show()
    
    # !!!
    # Clear graph
    def clear_graph(self):
        pass
    
    # Emit close signal
    def closeEvent(self, event):
        if self.idling:
            self.close_signal.emit()
            event.accept()
        else:
            self.message('Process ongoing')
            event.ignore()
    
    # Pop out message
    def message(self, text):
        QMessageBox.warning(None, "Spectroscopy", text, QMessageBox.Ok)
    
    # Enable serial
    def enable_serial(self, enable):
        self.General.setEnabled(enable)             # Enable general options groupbox
        self.pushButton_Save.setEnabled(enable)     # Enable save button
        self.pushButton_Scan.setEnabled(enable)     # Enable scan button
        self.pushButton_Info.setEnabled(enable)     # Enable info button

    # Bias range changing slot
    def bias_ran_change(self, bias_ran):
        self.bias_ran = bias_ran    # Set up storage variable
        
        # Set up spinboxes and scrollbars appearance
        ch = self.comboBox_RampCh_General.currentIndex()
        if ch == 0:                 # Only reset controls when dealing with bias ramping
            self.channel_change(0)
    
    # Change ramp channel slot
    def channel_change(self, ch):
        self.setup_spin(ch)             # Set up spinboxes range
        self.setup_scroll(ch)           # Set up scrollbars range
        self.min_cnv(False, 0)          # Set minimum scrollbar to 0
        self.max_cnv(False, 0xfffff)    # Set maximum scrollbar to 0xfffff
        self.step_cnv(False, 1)         # Set step to scrollbar 1
    
    # Set scrollbars range
    def setup_scroll(self, ch):
        if (not ch) and self.bias_dac:  # If ramp 20bit bias DAC
            scroll_max = 0xfffff    # 20bit maximum
        else:
            scroll_max = 0xffff     # 16bit maximum
                
        self.scrollBar_Max_General.setMaximum(scroll_max)       # Set the maximum of 'Max'
        self.scrollBar_Min_General.setMaximum(scroll_max - 1)   # Set the maximum of 'Min' (Min need always smaller than Max)
        
        # Start bias for delta channel
        if self.bias_dac:   # 20bit DAC
            self.scrollBar_Bias_Delta.setMaximum(0xfffff)
        else:               # 16bit DAC
            self.scrollBar_Bias_Delta.setMaximum(0xffff)
            
    # Set spinboxes range
    def setup_spin(self, ch):
        ran_dict = {9: 5.0, 10: 10.0, 15: 2.5}                      # Voltage limit for 16bit bias DAC
        ran = 5.0 if self.bias_dac else ran_dict[self.bias_ran]     # 20bit always +/- 5.0V
            
        if ch:      # Ramp Z offset fine
            # Use double spinboxes as spinboxes
            self.spinBox_Min_General.setDecimals(0)
            self.spinBox_Max_General.setDecimals(0)
            self.spiBox_StepSize_General.setDecimals(0)
            
            self.spinBox_Min_General.setMinimum(-0x8000)    # Set the minimum of 'Min'
            self.spinBox_Max_General.setMaximum(0x7fff)     # Set the maximum of 'Max'
            self.spiBox_StepSize_General.setMinimum(1)      # Set the minimum of 'step'
        else:       # Ramp bias
            # Determine percision based on 20bit DAC
            self.spinBox_Min_General.setDecimals(7)
            self.spinBox_Max_General.setDecimals(7)
            self.spiBox_StepSize_General.setDecimals(7)
            
            self.spinBox_Min_General.setMinimum(-ran)       # Set the minimum of 'Min'
            self.spinBox_Max_General.setMaximum(ran)        # Set the maximum of 'Max'
            self.spiBox_StepSize_General.setMinimum(0)      # Set the minimum of 'step'
            
        # Set up delta channel range
        self.spinBox_Bias_Delta.setMinimum(-ran)
        self.spinBox_Bias_Delta.setMaximum(ran)
        
    # Min spin box to scroll bar
    def min_cnv(self, flag, value):
        ch = self.comboBox_RampCh_General.currentIndex()    # Determin ramp channel
        bias_flag = '20' if self.bias_dac else 'd'          # Conversion flag
        bias_mid = 0x80000 if self.bias_dac else 0x8000     # Different bias mid scale
        step = self.scrollBar_StepSize_General.value()      # Current step size
        if ch:      # Ramp Z offset fine
            if flag:    # Spinbox to scrollbar
                value = self.spinBox_Min_General.value() + 0x8000                                       # Get value from spinbox
            value = min(value, max(self.scrollBar_Max_General.value() - step, 0))                       # Manually limit (at least one step)
            self.spinBox_Min_General.setValue(value - 0x8000)                                           # Set spinbox
            self.scrollBar_Min_General.setValue(int(value))                                             # Set scrollbar
            self.spinBox_Max_General.setMinimum(self.scrollBar_Min_General.value() - 0x8000 + step)     # Set the minimum of 'Max' (at least one step)
            
            step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()          # Set the maximum of 'Step' (at least one step)
            self.spiBox_StepSize_General.setMaximum(step_max)   
            data_num = int(step_max / self.scrollBar_StepSize_General.value())                          # Display data number
            self.label_DataNum_General.setText(str(data_num))
        else:       # Ramp bias
            if flag:    # Spinbox to scrollbar
                value = cnv.vb(self.spinBox_Min_General.value(), bias_flag, self.bias_ran)
            value = min(value, max(self.scrollBar_Max_General.value() - step, 0))
            self.spinBox_Min_General.setValue(cnv.bv(value, bias_flag, self.bias_ran))
            self.scrollBar_Min_General.setValue(int(value))
            self.spinBox_Max_General.setMinimum(cnv.bv(self.scrollBar_Min_General.value() + step, bias_flag, self.bias_ran))
            
            step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
            self.spiBox_StepSize_General.setMaximum(cnv.bv(step_max + bias_mid, bias_flag, self.bias_ran))
            data_num = int(step_max / self.scrollBar_StepSize_General.value())
            self.label_DataNum_General.setText(str(data_num))
            
    # Max spin box to scroll bar
    def max_cnv(self, flag, value):
        ch = self.comboBox_RampCh_General.currentIndex()
        bias_flag = '20' if self.bias_dac else 'd'
        bias_max = 0xfffff if self.bias_dac else 0xffff
        bias_mid = 0x80000 if self.bias_dac else 0x8000
        step = self.scrollBar_StepSize_General.value()
        if ch:      # Ramp Z offset fine
            if flag:    # Spinbox to scrollbar
                value = self.spinBox_Max_General.value() + 0x8000
            value = max(value, min(self.scrollBar_Min_General.value() + step, 0xffff))    
            self.spinBox_Max_General.setValue(value - 0x8000)
            self.scrollBar_Max_General.setValue(int(value))
            self.spinBox_Min_General.setMaximum(self.scrollBar_Max_General.value() - 0x8000 - step)
            
            step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
            self.spiBox_StepSize_General.setMaximum(step_max)
        else:       # Ramp bias
            if flag:    # Spinbox to scrollbar
                value = cnv.vb(self.spinBox_Max_General.value(), bias_flag, self.bias_ran)
            value = max(value, min(self.scrollBar_Min_General.value() + step, bias_max))
            self.spinBox_Max_General.setValue(cnv.bv(value, bias_flag, self.bias_ran))
            self.scrollBar_Max_General.setValue(int(value))
            self.spinBox_Min_General.setMaximum(cnv.bv(self.scrollBar_Max_General.value() - step, bias_flag, self.bias_ran))
            
            step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
            self.spiBox_StepSize_General.setMaximum(cnv.bv(step_max + bias_mid, bias_flag, self.bias_ran))
        data_num = int(step_max / self.scrollBar_StepSize_General.value())
        self.label_DataNum_General.setText(str(data_num))
            
    # Step change slot
    def step_cnv(self, flag, value):
        ch = self.comboBox_RampCh_General.currentIndex()
        bias_flag = '20' if self.bias_dac else 'd'
        bias_mid = 0x80000 if self.bias_dac else 0x8000
        step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
        if ch:      # Ramp Z offset fine
            if flag:    # Spinbox to scrollbar
                value = self.spiBox_StepSize_General.value()
            value = min(max(step_max, 1), value)
            self.spiBox_StepSize_General.setValue(value)
            self.scrollBar_StepSize_General.setValue(int(value))
            self.spinBox_Max_General.setMinimum(self.scrollBar_Min_General.value() - 0x8000 + value)
            self.spinBox_Min_General.setMaximum(self.scrollBar_Max_General.value() - 0x8000 - value)
        else:       # Ramp bias
            if flag:    # Spinbox to scrollbar
                value = cnv.vb(self.spiBox_StepSize_General.value(), bias_flag, self.bias_ran) - bias_mid
            value = min(max(step_max, 1), value)
            self.scrollBar_StepSize_General.setValue(value)
            self.spiBox_StepSize_General.setValue(cnv.bv(value + bias_mid, bias_flag, self.bias_ran))
            self.spinBox_Max_General.setMinimum(cnv.bv(self.scrollBar_Min_General.value() + value, bias_flag, self.bias_ran))
            self.spinBox_Min_General.setMaximum(cnv.bv(self.scrollBar_Max_General.value() - value, bias_flag, self.bias_ran))

        step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
        data_num = int(step_max / self.scrollBar_StepSize_General.value())
        self.label_DataNum_General.setText(str(data_num))
            
    # Start bias conversion slot
    def bias_cnv(self, flag, value):
        bias_flag = '20' if self.bias_dac else 'd'
        if flag:    # Spinbox to scrollbar
            value = self.spinBox_Bias_Delta.value()
            self.scrollBar_Bias_Delta.setValue(cnv.vb(value, bias_flag, self.bias_ran))
        else:       # Scrollbar to spinbox
            self.spinBox_Bias_Delta.setValue(cnv.bv(value, bias_flag, self.bias_ran))
            
    # Configure spectroscopy general options
    def configure_spc(self, dsp):
        start = self.scrollBar_Min_General.value()
        final = self.scrollBar_Max_General.value()
        step = self.scrollBar_StepSize_General.value()
        data_num = int((final - start) / step)
        pass_num = self.spinBox_Pass_General.value()
        delta_flag = self.groupBox_Delta_General.isChecked()      
        if self.comboBox_RampCh_General.currentIndex():             # Ramp Z offset fine
            ramp_ch = 0x12                                                              # Ramp channel address
            delta_ch = 0x20 if self.bias_dac else 0x1d                                  # Delta channel address
            b = dsp.lasdac[13] if self.bias_dac else dsp.last20bit                      # Current bias data
            delta_data = (self.scrollBar_Bias_Delta.value() - b) if delta_flag else 0   # Calculate delta data
        else:                                                      # Ramp bias
            ramp_ch = 0x20 if self.bias_dac else 0x1d
            delta_ch = 0x12
            delta_data = self.scrollBar_Z_Delta.value() if delta_flag else 0            # Collect delta data

        return start, step, data_num, pass_num, ramp_ch, delta_ch, delta_data
    
    # Save data slot
    def save(self):
        name, flag = self.atuo_save_window()    # Pop out file dialog for saving
        if flag:                                # If savable
            self.auto_save(name, 0)             # Save averaged data
            
    # Confgure auto save      
    def configure_autosave(self):
        # Auto save is only enabled when auto save enabled AND setup proper file name
        autosave_name = self.atuo_save_window() if self.adv.groupBox_AutoSave_AdvOption.isChecked() else ''
        # Save every pass will not be enabled is auto save is not enabled
        every = self.adv.checkBox_SaveEveryPasses_Autosave.isChecked() and autosave_name
        return autosave_name, every

    # Auto save pop window
    def atuo_save_window(self):
        # Re-init file index if date change
        if self.data.time.strftime("%m%d%y") != self.today:
            self.today = self.data.time.strftime("%m%d%y")
            self.file_idex = 0
        name_list = '0123456789abcdefghijklmnopqrstuvwxyz'                                      # Name lise
        name = self.today + name_list[self.file_idex // 36] + name_list[self.file_idex % 36]    # Auto configure file name
        self.dlg.selectFile(self.dlg.directory().path() + '/' + name + '.spc')                  # Set default file name as auto configured
        if self.dlg.exec_():        # File selected
            directory = self.dlg.directory().path()                                                     # Get path
            self.dlg.setDirectory(directory)                                                            # Set path for next call
            save_name = self.dlg.selectedFiles()[0].replace(directory + '/', '').replace('.spc', '')    # Get the real file name
            
            # If default file name is not used
            if save_name != name:
                try:    # See if current file name is in our naming system
                    if save_name[0:6] == self.today:
                        self.file_idex = name_list.index(save_name[6]) * 36 + name_list.index(save_name[7])
                    else:
                        self.file_idex -= 1
                except: # Otherwise do not consume file index
                    self.file_idex -= 1
                name = save_name
        else:                       # No file selected
            name = ''   # Empty file name
        return name

    # Auto save function
    def auto_save(self, name, pass_num):
        if pass_num:        # Save single pass
            fname = self.dlg.directory().path() + '/' + name + '_pass' + str(pass_num) + '.spc'     # Single pass file name
        else:               # Save averaged pass data
            fname = self.dlg.directory().path() + '/' + name + '.spc'   # Averaged data file name
            self.file_index += 1                                        # Consume 1 file index
            self.saved = True                                           # Toggle saved flag
            self.setWindowTitle('Spectroscopy-' + name)                 # Chage window title for saving status indication
        with open(fname, 'wb') as output:
            self.data.path = fname                                      # Save path
            pickle.dump(self.data, output, pickle.HIGHEST_PROTOCOL)     # Save data


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySpc()
    window.init_spc(False, False, 9)
    window.show()
    sys.exit(app.exec_())
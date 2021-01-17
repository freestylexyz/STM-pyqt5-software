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
    close_signal = pyqtSignal()
    spectroscopy_signal = pyqtSignal()
    stop_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()
        
    def init_UI(self):
        self.adv = myAdvanceOption()
        self.info = mySpectroscopyInfo()
        
        self.data = SpcData()
        self.data_ = SpcData()
        self.file_idex = 0
        self.dlg = QFileDialog()
        self.today = datetime.now().strftime("%m%d%y")
        self.dlg.setFileMode(QFileDialog.AnyFile)
        self.dlg.setAcceptMode(QFileDialog.AcceptSave)
        self.dlg.setNameFilter('SPC Files (*.spc)')
        
        self.bias_dac = False
        self.bias_ran = 9
        self.idling = True
        
        # Window
        self.pushButton_Info.clicked.connect(self.open_info)
        self.pushButton_Advance.clicked.connect(self.adv.show)
        
        # Conversion
        self.spinBox_Min_General.editingFinished.connect(lambda: self.min_cnv(True, 0))
        self.spinBox_Max_General.editingFinished.connect(lambda: self.max_cnv(True, 0))
        self.spiBox_StepSize_General.editingFinished.connect(lambda: self.step_cnv(True, 0))
        self.spinBox_Bias_Delta.editingFinished.connect(lambda: self.bias_cnv(True, 0))
        self.spinBox_Delta_Z.editingFinished.connect(lambda: self.scrollBar_Z_Delta.setValue(self.spinBox_Delta_Z.value()))
        
        self.scrollBar_Min_General.valueChanged.connect(ft.partial(self.min_cnv, False))
        self.scrollBar_Max_General.valueChanged.connect(ft.partial(self.max_cnv, False))
        self.scrollBar_StepSize_General.valueChanged.connect(ft.partial(self.step_cnv, False))
        self.scrollBar_Bias_Delta.valueChanged.connect(ft.partial(self.bias_cnv, False))
        self.scrollBar_Z_Delta.valueChanged.connect(self.spinBox_Delta_Z.setValue)
        
        # Status change
        self.comboBox_RampCh_General.currentIndexChanged.connect(self.channel_change)
        
        # Buttons
        self.pushButton_Save.clicked.connect(self.save)
        
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
        self.General.setEnabled(enable)
        self.pushButton_Save.setEnabled(enable)
        self.pushButton_Scan.setEnabled(enable)
        self.pushButton_Info.setEnabled(enable)

    # Bias range changing slot
    def bias_ran_change(self, bias_ran):
        self.bias_ran = bias_ran
        ch = self.comboBox_RampCh_General
        if ch == 0:
            self.setup_spin(0)
            self.setup_scroll(0)
            self.min_cnv(False, 0)
            self.max_cnv(False, 0xfffff)
            self.step_cnv(False, 1)
    
    # Change ramp channel slot
    def channel_change(self, ch):
        self.setup_spin(ch)
        self.setup_scroll(ch)
        self.min_cnv(False, 0)
        self.max_cnv(False, 0xfffff)
        self.step_cnv(False, 1)
    
    # Set scroll bar range
    def setup_scroll(self, ch):
        if (not ch) and self.bias_dac:
            scroll_max = 0xfffff
        else:
            scroll_max = 0xffff
                
        self.scrollBar_Max_General.setMaximum(scroll_max)
        self.scrollBar_Min_General.setMaximum(scroll_max - 1)
        
        if self.bias_dac:
            self.scrollBar_Bias_Delta.setMaximum(0xfffff)
        else:
            self.scrollBar_Bias_Delta.setMaximum(0xffff)
            
    # Set spin box range
    def setup_spin(self, ch):
        ran_dict = {9: 5.0, 10: 10.0, 15: 2.5}
        if self.bias_dac:
            ran = 5.0
        else:
            ran = ran_dict[self.bias_ran]
        if ch:
            self.spinBox_Min_General.setDecimals(0)
            self.spinBox_Max_General.setDecimals(0)
            self.spiBox_StepSize_General.setDecimals(0)
            
            self.spinBox_Min_General.setMinimum(-0x8000)
            self.spinBox_Max_General.setMaximum(0x7fff)
            self.spiBox_StepSize_General.setMinimum(1)
        else:
            self.spinBox_Min_General.setDecimals(7)
            self.spinBox_Max_General.setDecimals(7)
            self.spiBox_StepSize_General.setDecimals(7)
            
            self.spinBox_Min_General.setMinimum(-ran)
            self.spinBox_Max_General.setMaximum(ran)
            self.spiBox_StepSize_General.setMinimum(0)
            
        self.spinBox_Bias_Delta.setMinimum(-ran)
        self.spinBox_Bias_Delta.setMaximum(ran)
        
    # Min spin box to scroll bar
    def min_cnv(self, flag, value):
        ch = self.comboBox_RampCh_General.currentIndex()
        bias_flag = '20' if self.bias_dac else 'd'
        bias_mid = 0x80000 if self.bias_dac else 0x8000
        step = self.scrollBar_StepSize_General.value()
        if ch:
            if flag:
                value = self.spinBox_Min_General.value() + 0x8000
            value = min(value, max(self.scrollBar_Max_General.value() - step, 0))
            self.spinBox_Min_General.setValue(value - 0x8000)
            self.scrollBar_Min_General.setValue(int(value))
            self.spinBox_Max_General.setMinimum(self.scrollBar_Min_General.value() - 0x8000 + step)
            
            step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
            self.spiBox_StepSize_General.setMaximum(step_max)
            data_num = int(step_max / self.scrollBar_StepSize_General.value())
            self.label_DataNum_General.setText(str(data_num))
        else:
            if flag:
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
        if ch:
            if flag:
                value = self.spinBox_Max_General.value() + 0x8000
            value = max(value, min(self.scrollBar_Min_General.value() + step, 0xffff))    
            self.spinBox_Max_General.setValue(value - 0x8000)
            self.scrollBar_Max_General.setValue(int(value))
            self.spinBox_Min_General.setMaximum(self.scrollBar_Max_General.value() - 0x8000 - step)
            
            step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
            self.spiBox_StepSize_General.setMaximum(step_max)
        else:
            if flag:
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
        if ch:
            if flag:
                value = self.spiBox_StepSize_General.value()
            value = min(max(step_max, 1), value)
            self.spiBox_StepSize_General.setValue(value)
            self.scrollBar_StepSize_General.setValue(int(value))
            self.spinBox_Max_General.setMinimum(self.scrollBar_Min_General.value() - 0x8000 + value)
            self.spinBox_Min_General.setMaximum(self.scrollBar_Max_General.value() - 0x8000 - value)
        else:
            if flag:
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
        if flag:
            value = self.spinBox_Bias_Delta.value()
            self.scrollBar_Bias_Delta.setValue(cnv.vb(value, bias_flag, self.bias_ran))
        else:
            self.spinBox_Bias_Delta.setValue(cnv.bv(value, bias_flag, self.bias_ran))
            
    # Configure spectroscopy general options
    def configure_spc(self, dsp):
        start = self.scrollBar_Min_General.value()
        final = self.scrollBar_Max_General.value()
        step = self.scrollBar_StepSize_General.value()
        data_num = int((final - start) / step)
        pass_num = self.spinBox_Pass_General.value()
        delta_flag = self.groupBox_Delta_General.isChecked()
        if self.comboBox_RampCh_General.currentIndex():
            ramp_ch = 0x12
            delta_ch = 0x20 if self.bias_dac else 0x1d
            b = dsp.lasdac[13] if self.bias_dac else dsp.last20bit
            delta_data = (self.scrollBar_Bias_Delta.value() - b) if delta_flag else 0
        else:
            ramp_ch = 0x20 if self.bias_dac else 0x1d
            delta_ch = 0x12
            delta_data = self.scrollBar_Z_Delta.value() if delta_flag else 0

        return start, step, data_num, pass_num, ramp_ch, delta_ch, delta_data
    
    # Save data slot
    def save(self):
        name, flag = self.atuo_save_window()
        if flag:
            self.auto_save(name, 0)
            
    # Confgure auto save      
    def configure_autosave(self):
        autosave_name = self.atuo_save_window() if self.adv.groupBox_AutoSave_AdvOption.isChecked() else ''
        every = self.adv.checkBox_SaveEveryPasses_Autosave.isChecked() and autosave_name
        return autosave_name, every

    # Auto save pop window
    def atuo_save_window(self):
        if self.data.time.strftime("%m%d%y") != self.today:
            self.today = self.data.time.strftime("%m%d%y")
            self.file_idex = 0
        name_list = '0123456789abcdefghijklmnopqrstuvwxyz'
        name = self.today + name_list[self.file_idex // 36] + name_list[self.file_idex % 36]
        self.dlg.selectFile(self.dlg.directory().path() + '/' + name + '.spc')
        if self.dlg.exec_():
            directory = self.dlg.directory().path()
            self.dlg.setDirectory(directory)
            save_name = self.dlg.selectedFiles()[0].replace(directory + '/', '').replace('.spc', '')
            if save_name != name:
                try:
                    self.file_idex = name_list.index(save_name[6]) * 36 + name_list.index(save_name[7])
                except:
                    self.file_idex -= 1
                name = save_name
        else:
            name = ''
        return name

    # Auto save function
    def auto_save(self, name, pass_num):
        if pass_num:
            data = self.data_
            fname = self.dlg.directory().path() + '/' + name + '_pass' + str(pass_num) + '.spc'
        else:
            data = self.data
            fname = self.dlg.directory().path() + '/' + name + '.spc'
            self.file_index += 1
            self.saved = True
            self.setWindowTitle('Spectroscopy-' + name)
        with open(fname, 'wb') as output:
            data.path = fname                                          # Save path
            pickle.dump(data, output, pickle.HIGHEST_PROTOCOL)         # Save data


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySpc()
    window.init_spc(False, False, 9)
    window.show()
    sys.exit(app.exec_())
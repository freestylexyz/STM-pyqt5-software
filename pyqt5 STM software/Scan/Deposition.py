# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:19:17 2020

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
from PyQt5.QtWidgets import QApplication , QWidget, QMessageBox, QFileDialog
from PyQt5.QtCore import pyqtSignal
from Deposition_ui import Ui_Deposition
from DepositionInfo import myDepositionInfo
from sequence import mySequence
from DataStruct import DepData

import functools as ft
import conversion as cnv
import pickle


class myDeposition(QWidget, Ui_Deposition):
    close_signal = pyqtSignal()
    seq_list_signal = pyqtSignal(str)
    do_it_signal = pyqtSignal(list, list, int)
    stop_signal = pyqtSignal()
    

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.data = DepData()
        self.info = myDepositionInfo()
        self.idling = True
        self.bias_dac = True
        self.bias_ran = 9
        self.saved = True
        self.limit = 65536
        self.stop_num = 0
        self.rdata = []
        self.count = 0

        self.poke_seq = mySequence([0x42, 0x63, 0x8d], [0x00000000, 0x80000000, 0x00008000] , False)    # Initial poke sequence
        
        # Connect signal
        self.groupBox_ReadNSample_Deposition.toggled.connect(ft.partial(self.read_mode, True))
        self.groupBox_ReadUntil_Deposition.toggled.connect(ft.partial(self.read_mode, False))
        self.groupBox_Seq_Deposition.toggled.connect(self.seq_mode)
        
        self.spinBox_DeltaZ_PokeTip.valueChanged.connect(self.scrollBar_DeltaZ_PokeTip.setValue)
        self.scrollBar_DeltaZ_PokeTip.valueChanged.connect(self.spinBox_DeltaZ_PokeTip.setValue)
        self.spinBox_Bias_PokeTip.valueChanged.connect(ft.partial(self.bias_cnv, True))
        self.scrollBar_Bias_PokeTip.valueChanged.connect(ft.partial(self.bias_cnv, False))
        
        self.pushButton_SeqEditor_Deposition.clicked.connect(self.seq_list_slot)
        self.pushButton_DoIt_Deposition.clicked.connect(self.do_it)
        self.pushButton_Save_Deposition.clicked.connect(self.save)
        
        self.pushButton_Info_Deposition.clicked.connect(self.open_Info)

    # Open sequence list signal
    def seq_list_slot(self):
        self.seq_list_signal.emit(self.label_Seq_Deposition.text())
        
    # Called with int_scan
    def init_deposition(self, succeed, bias_dac, bias_ran, seq_list, selected):
        # Enable serial buttons
        self.pushButton_DoIt_Deposition.setEnabled(succeed)
        self.pushButton_Save_Deposition.setEnabled(not self.data.data)
        self.pushButton_Info_Deposition.setEnabled(not self.data.data)
        
        self.bias_dac = bias_dac
        self.bias_ran = bias_ran
        if bias_dac:
            self.scrollBar_Bias_PokeTip.setMaximum(0xfffff)
            self.scrollBar_Bias_PokeTip.setValue(0x80000)
            self.spinBox_Bias_PokeTip.setMaximum(5.0)
            self.spinBox_Bias_PokeTip.setMinimum(-5.0)
            self.spinBox_Bias_PokeTip.setValue(0.0)
        else:
            self.scrollBar_Bias_PokeTip.setMaximum(0xffff)
            self.scrollBar_Bias_PokeTip.setValue(0x8000)
            self.bias_ran_change(bias_ran)
            
        seq_name = '' if selected < 0 else seq_list[selected].name
        self.label_Seq_Deposition.setText(seq_name)
        
    # Read mode slot
    def read_mode(self, mode, state):
        if mode:
            if state:
                self.groupBox_ReadUntil_Deposition.setChecked(False)
        else:
            if state:
                self.groupBox_ReadNSample_Deposition.setChecked(False)
    
    # Sequence mode slot
    def seq_mode(self, state):
        self.groupBox_Poke_Deposition.setEnabled(not state)
    
    # Enable serial realted components
    def enable_serial(self, enable):
        self.groupBox_Seq_Deposition.setEnabled(enable)
        self.groupBox_Read_Deposition.setEnabled(enable)
        self.groupBox_ReadPulse_Deposition.setEnabled(enable)
        self.pushButton_DoIt_Deposition.setEnabled(enable)
        self.pushButton_Save_Deposition.setEnabled(enable and (not self.data.data))
        self.pushButton_Info_Deposition.setEnabled(enable and (not self.data.data))
        mode = not self.groupBox_Poke_Deposition.isChecked()
        self.groupBox_Poke_Deposition.setEnabled(enable and mode)
    
    # Bias conversion slot
    def bias_cnv(self, flag, value):
        if flag:
            if self.bias_dac:
                out = cnv.vb(value, '20')
            else:
                out = cnv.vb(value, 'd', self.bias_ran)
            self.scrollBar_Bias_PokeTip.setValue(out)
        else:
            if self.bias_dac:
                out = cnv.bv(value, '20')
            else:
                out = cnv.bv(value, 'd', self.bias_ran)
            self.spinBox_Bias_PokeTip.setValue(out)
    
    # Do it slot
    def do_it(self):
        if self.idling:
            self.idling = False
            # Read before and after
            if self.groupBox_Read_Deposition.isChecked():
                read_before_ch = (self.comboBox_Ch_Read.currentIndex() + 6) * 4 + 0xc0
                read_before_avg = self.spinBox_AvgNum_Read.value()
                read_before_num = self.spinBox_Num_Read.value()
                read_before_delay = self.spinBox_Delay_Read.value()
                read_before = [read_before_ch, read_before_num, read_before_avg, read_before_delay]
            else:
                read_before = []
            
            # Read pulse
            read_ch = (self.comboBox_Ch_Pulse.currentIndex() + 6) * 4 + 0xc0
            read_delay = self.spinBox_Wait_Pulse.value()
            read_avg = self.spinBox_AvgNum_Pulse.value()
            read_delay2 = self.spinBox_Delay_Pulse.value()
            
            read_num = self.spinBox_Num_ReadNSample.value()
            
            read_change = cnv.vb((self.spinBox_Change_ReadUntil.value() - 10.24), 'a')
            
            read_stop_num = self.spinBox_StopNum_ReadUntil.value()
            
            self.limit = read_change
            self.stop_num = read_stop_num
            self.rdata = []
            self.count = 0
            
            
            if not (self.groupBox_ReadNSample_Deposition.isChecked() or self.groupBox_ReadUntil_Deposition.isChecked()):
                read_mode = 0
            elif self.groupBox_ReadNSample_Deposition.isChecked():
                read_mode = 3
            elif self.radioButton_NSample_ReadUntil.isChecked():
                read_mode = 2
            elif self.radioButton_Continuous_ReadUntil.isChecked():
                read_mode = 1
            
            read = [read_ch, read_mode, read_delay, read_delay2, read_num, read_avg, read_change, read_stop_num]
            self.data = DepData()
            if read_mode != 0:
                self.saved = False
                self.data.read = read

            # Sequence
            if not self.groupBox_Seq_Deposition.isChecked():
                poke_command = [0x42, 0x63, 0x8d]                                  # Feedback, Shift Z offset, Aout bias
                poke_data = [0x00000000, 0x80000000, 0x00008000]                   # OFF, Forward 0 bit, output zero voltage
                poke_data[2] = self.scrollBar_Bias_PokeTip.value()
                deltaZ = self.scrollBar_DeltaZ_PokeTip.value()
                if deltaZ >= 0:
                    poke_data[1] = 0x80000000 + deltaZ
                else:
                    poke_data[1] = 0x80000000 - deltaZ
            
                self.poke_seq = mySequence(poke_command, poke_data, False)
                index = False
            else:
                index = True
                
            if self.saved:
                flag = True
            else:
                msg = QMessageBox.information(None, "Depostion", "Data is unsaved. Do you want to continue?", QMessageBox.Yes | QMessageBox.No)
                if msg == QMessageBox.Yes:
                    flag = True
                else:
                    flag = False
            if flag:
                self.do_it_signal.emit(read_before, read, index)
        else:
            self.pushButton_DoIt_Deposition.setEnabled(False)
            self.stop_signal.emit()
                
    # Bias range change slot
    def bias_ran_change(self, ran):
        self.bias_ran = ran
        val = self.spinBox_Bias_PokeTip.value()
        if not self.bias_dac:
            if ran == 9:
                Max = 5.0
                Min = -5.0
            elif ran == 10:
                Max = 10.0
                Min = -10.0
            elif ran == 14:
                Max = 2.5
                Min = -2.5
            self.spinBox_Bias_PokeTip.setMaximum(Max)
            self.spinBox_Bias_PokeTip.setMinimum(Min)
            self.bias_cnv(True, val)


    # !!!
    # Open Inforamtion window
    def open_Info(self):
        # self.info.init_info()
        self.info.show()

    # Save data slot
    # !!! need to know how to figure out file name automatically
    def save(self):
        fname = QFileDialog.getSaveFileName(self, 'Save file', '', 'DEP Files (*.dep)')[0]  # Save file and get file name
        if fname != '':                         # Savable
            with open(fname, 'wb') as output:
                pickle.dump(self.data, output, pickle.HIGHEST_PROTOCOL)         # Save data
                self.data.path = fname                                          # Save path
                self.saved = True
                self.setWindowTitle('Deposition')
    
    # Pop out message
    def message(self, text):
        QMessageBox.warning(None, "Depostion", text, QMessageBox.Ok)
    
    # !!! Not finished
    # Update N samples data
    def update_N(self, index):
        pass
    
    # !!! Not finished
    # Update continuous data
    def update_C(self, rdata):
        self.rdata += [rdata]
        if abs(rdata - self.rdata[0])> self.limit:
            self.count += 1
            if self.count >= self.stop_num:
                self.stop_signal.emit()
                
    # Call information window
    def info(self):
        pass
    
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
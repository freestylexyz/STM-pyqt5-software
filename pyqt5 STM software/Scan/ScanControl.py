# -*- coding: utf-8 -*-
"""
Created on Thu Dec  3 11:11:38 2020

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
from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSignal, Qt, QMetaObject, QSettings
from Setting import mySetting
from TipApproach import myTipApproach
from Etest import myEtest
from MainMenu import myMainMenu

import conversion as cnv
import threading
import copy

class myScanControl(myMainMenu):
    # Scan initial operation
    def enter_scan(self):
        pass
    
    # Exit scan operation
    def exit_scan(self):
        pass
    
    # Sequence list close signal
    def close_seq_list(self, index):
        list_dict = {0: self.scan.scan_seq_list, 1: self.scan.dep_seq_list, 2: self.scan.spc_seq_list}
        select_dict = {0: self.scan.scan_seq_selected, 1: self.scan.dep_seq_selected, 2: self.scan.spc_seq_selected}
        label_dict = {0: self.scan.label_Seq_ScanControl.setText, 1: self.scan.dep.label_Seq_Deposition.setText, \
                     2: self.scan.spc.adv.label_Seq_AdvOption.setText}
        if self.scan.seq_list.selected < 0:
            name = ''
        else:
            name = self.scan.seq_list.seqlist[self.scan.seq_list.selected].name
        
        select_dict[index] = self.scan.seq_list.seqlist
        label_dict[index](name)
        list_dict[index] = []
        
        for seq in self.scan.seq_list.seqlist:
            list_dict[index] += copy.deepcopy(seq)
        
    # Open sequence list window
    def open_seq_list(self, index, selected_name):
        if index == 0:
            seq_list = self.scan.scan_seq_list
            mode = True
        elif index == 1:
            seq_list = self.scan.dep_seq_list
            mode = False
        elif index == 2:
            seq_list = self.scan.spc_seq_list
            mode = True

        self.scan.seq_list.init_seqlist(index, seq_list, selected_name, mode, self.bias_dac, self.preamp_gain, self.dsp.dacrange, \
                                   self.dsp.lastdac, self.dsp.last20bit)
        self.scan.seq_list.show()

    # send setup
    def send(self, index, xin, yin, xoffset, yoffset):
        '''def rampDiag(self, channels, channell, targets, targetl, step, delay, limit, checkstop):'''
        if self.scan.idling:
            self.scan.enable_serial(False)
            self.scan.idling = False
            step = self.scan.send_options.spinBox_StepSize_SendOptions.value()
            delay = self.scan.send_options.spinBox_MoveDelay_SendOptions.value()
            if self.scan.send_options.groupBox_Crash_SendOptions.isEnabled():
                limit = self.scan.send_options.spinBox_Limit_Crash.value()
            else:
                limit = 0       #!!! limit init value
            if index == 0:  # zero
                self.pushButton_Zero_XY.setText("Stop")
                self.pushButton_Zero_XY.setEnabled(True)
                self.dsp.rampDiag(0+16, 15+16, 0, 0, step, delay, limit, True)
            else:           # send
                self.pushButton_Send_XY.setText("Stop")
                self.pushButton_Send_XY.setEnabled(True)
                self.dsp.rampDiag(0+16, 15+16, xin, yin, step, delay, limit, True)
                self.dsp.rampDiag(1+16, 14+16, xoffset, yoffset, step, delay, limit, True)
            self.scan.idling = True
            self.scan.enable_serial(True)
            self.pushButton_Zero_XY.setText("Zero")
            self.pushButton_Send_XY.setText("Send")

    # send signal slot
    def send_slot(self, index, xin, yin, xoffset, yoffset):
        threading.Thread(target=(lambda: self.send(index, xin, yin, xoffset, yoffset))).start()

    # dsp rampDiag_signal slot
    def xy_indication_slot(self, channels, channell, currents, currentl):
        '''self.rampDiag_signal.emit(channels, channell, currents, currentl)'''
        if channels == 0+16 and channell == 15+16:      # X/Y in
            self.scan.spinBox_XinIndication_XY.setValue(currents-32768)
            self.scan.spinBox_YinIndication_XY.setValue(currentl-32768)
            self.scan.current_xy[0] = currents - 32768
            self.scan.current_xy[1] = currentl - 32768
        elif channels == 1+16 and channell == 14+16:    # X/Y offset
            self.scan.spinBox_XoffsetIndication_XY.setValue(currents-32768)
            self.scan.spinBox_YoffsetIndication_XY.setValue(currentl-32768)
            self.scan.current_xy[2] = currents - 32768
            self.scan.current_xy[3] = currentl - 32768

    # Depostion slot
    def deposition(self, read_before, read, index):
        pass

    # Scan related stop slot
    def scan_stop(self):
        self.scan.stop = True
        self.dsp.stop = True
    

        
        
        
        
        
        
        
        
        
        
        
        
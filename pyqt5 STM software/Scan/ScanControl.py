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
    # !!!
    # Scan initial operation
    def enter_scan(self):
        pass
    # !!!
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

    # Send execution
    def send_excu(self, index, xin, yin, xoffset, yoffset):
        '''def rampDiag(self, channels, channell, targets, targetl, step, delay, limit, checkstop):'''
        if self.scan.idling:
            self.scan.enable_mode_serial(False)
            self.scan.idling = False
            step = self.scan.send_options.spinBox_StepSize_SendOptions.value()
            delay = self.scan.send_options.spinBox_MoveDelay_SendOptions.value()
            if self.scan.send_options.groupBox_Crash_SendOptions.isEnabled():
                limit = cnv.vb(self.scan.send_options.spinBox_Limit_Crash.value(), 'a') - cnv.vb(0.0, 'a')
            else:
                limit = 0
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
            self.scan.enable_mode_serial(True)
            self.pushButton_Zero_XY.setText("Zero")
            self.pushButton_Send_XY.setText("Send")

    # Send signal slot
    def send_thread(self, index, xin, yin, xoffset, yoffset):
        threading.Thread(target=(lambda: self.send_excu(index, xin, yin, xoffset, yoffset))).start()
        
    # !!!
    # Scan execution
    def scan_excu(self):
        if self.scan.idling:
            self.scan.idling = False
            self.scan.enable_mode_serial(False)
            # self.scan.track.pushButton_Start_Track.setText('Stop')
            # self.scan.track.pushButton_Start_Track.setEnable(False)
            # self.dsp.track(track[0], track[1], track[2], track[3], track[4], track[5], track[6], track[7])
            self.scan.enable_mode_serial(True)
            self.scan.idling = True

    # !!!
    # Scan signal slot
    def scan_thread(self):
        threading.Thread(target=(lambda: self.scan_excu())).start()

    # !!!
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
    def deposition_thread(self, read_before, read, index):
        if index:
            if self.scan.dep_seq_selected < 0:
                self.scan.dep.message('No sequence selected')
                flag = False
            else:
                seq =  self.scan.dep_seq_list[self.scan.dep_seq_selected]
                seq.configure(self.bias_dac, self.preamp_gain, self.dsp.dacrange, self.dsp.lastdac, self.dsp.last20bit)
                seq.validation(self.seq.ditherB, self.seq.ditherZ, self.seq.feedback, self.seq.mode)
                seq.build()
                flag = seq.validated
                if not seq.validated:
                    self.scan.dep.message('Sequence not validated')
        else:
            seq = self.scan.dep.poke_seq
            flag = True
        if flag:
            threading.Thread(target = (lambda: self.deposition_excu(read_before, read, seq))).start()

    # Depostion execution
    def deposition_excu(self, read_before, read, seq):
        if self.scan.idling:
            self.scan.idling = False
            self.scan.dep.idling = False
            self.scan.enable_mode_serial(False)
            if read_before:
                rdata = self.dsp.osc_N(read_before[0], read_before[1], read_before[2], read_before[3])
                self.scan.dep.update_N(rdata, 0)
            
            rdata = self.dsp.depostion(read[0], read[1], read[2], read[3], read[4], read[5], read[6], read[7], seq)
            if read[1] == 1:
                self.scan.dep.pushButton_DoIt_Deposition.setText('Stop')
                self.scan.dep.pushButton_DoIt_Deposition.setEnabled(True)
            if (read[1] == 2) and (read[1] == 3):
                self.scan.dep.update_N(rdata, 1)

            if read_before:
                rdata = self.dsp.osc_N(read_before[0], read_before[1], read_before[2], read_before[3])
                self.scan.dep.update_N(rdata, 2)
            
            self.scan.dep.pushButton_DoIt_Deposition.setText('Do it')
            self.scan.enable_mode_serial(True)
            self.scan.stop = True
            self.scan.idling = True
            self.scan.dep.idling = True
            
    # Track slot
    def track_thread(self, track):
        threading.Thread(target = (lambda: self.track_excu(track))).start()
        
    # Track execution
    def track_excu(self, track):
        if self.scan.idling:
            self.scan.idling = False
            self.scan.track.idling = False
            self.scan.enable_mode_serial(False)
            self.scan.track.pushButton_Start_Track.setText('Stop')
            self.scan.track.pushButton_Start_Track.setEnable(False)
            self.dsp.track(track[0], track[1], track[2], track[3], track[4], track[5], track[6], track[7])
            self.scan.enable_mode_serial(True)
            self.scan.idling = True
            self.scan.track.idling = True
            
    # !!!
    # Spectroscopy slot
    def spectroscopy_thread(self):
        threading.Thread(target = (lambda: self.spectroscopy_excu())).start()
        
    # !!!
    # Spectroscopy execution
    def spectroscopy_excu(self):
        if self.scan.idling:
            self.scan.idling = False
            self.scan.spc.idling = False
            self.scan.enable_mode_serial(False)
            # self.scan.track.pushButton_Start_Track.setText('Stop')
            # self.scan.track.pushButton_Start_Track.setEnable(False)
            # self.dsp.track(track[0], track[1], track[2], track[3], track[4], track[5], track[6], track[7])
            self.scan.enable_mode_serial(True)
            self.scan.idling = True
            self.scan.spc.idling = True

    # Scan related stop slot
    def scan_stop(self):
        self.scan.stop = True
        self.dsp.stop = True
    

        
        
        
        
        
        
        
        
        
        
        
        
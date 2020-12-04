
# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:48:07
@Author   : milier00
@FileName : Setting.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication , QWidget, QMessageBox
from PyQt5.QtCore import pyqtSignal , QSettings
from PyQt5 import QtCore
from Setting_ui import Ui_Setting
import serial
import serial.tools.list_ports
from time import sleep
import conversion as cnv



class mySetting(QWidget, Ui_Setting):
    initDSP_signal = pyqtSignal(str, int)
    loadOffset_signal = pyqtSignal(int, int, int, int)
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()
    
    # Initial UI for setting
    def init_UI(self):
        self.initButton.clicked.connect(self.initButton_slot)
        self.loadButton.clicked.connect(self.loadButton_slot)
        self.comboBox_uart.activated.connect(self.find_port)
    
    # Initial setting
    def init_setting(self, succeed, port, baudrate, offset):
        if succeed:    # try "if False:" to test it
            self.comboBox_uart.setCurrentText(port)
            self.comboBox_baud.setCurrentText(str(baudrate))
            self.comboBox_uart.setEnabled(False)
            self.comboBox_baud.setEnabled(False)
            self.initButton.setEnabled(False)
            self.loadButton.setEnabled(True)
            self.spinBox_Biasoffset25_Setting.setEnabled(True)
            self.spinBox_Biasoffset5_Setting.setEnabled(True)
            self.spinBox_Biasoffset10_Setting.setEnabled(True)
            self.spinBox_Isetoff_Setting.setEnabled(True)
            
            # Need convert integers to number
            offset1 = cnv.bv(offset[14], 'd', 14)
            offset2 = cnv.bv(offset[9], 'd', 14)
            offset3 = cnv.bv(offset[10], 'd', 14)
            offset4 = cnv.bv(offset[15], 'd', 10)
            
            self.spinBox_Biasoffset25_Setting.setValue(offset1)
            self.spinBox_Biasoffset5_Setting.setValue(offset2)
            self.spinBox_Biasoffset10_Setting.setValue(offset3)
            self.spinBox_Isetoff_Setting.setValue(offset4)
        else:
            self.find_port()

    # Find available ports and add them into comboBox list
    def find_port(self):
        port_list = [port[0] for port in serial.tools.list_ports.comports()]
        self.comboBox_uart.clear()
        self.comboBox_uart.addItems(port_list)

    # Initial DSP button slot
    def initButton_slot(self):
        port = self.comboBox_uart.currentText()
        baudrate = int(self.comboBox_baud.currentText(), base=10)
        self.initDSP_signal.emit(port, baudrate)
        
    # Load offset button slot
    def loadButton_slot(self):
        offset1 = cnv.vb(self.spinBox_Biasoffset25_Setting.Value(), 'd', 14)
        offset2 = cnv.vb(self.spinBox_Biasoffset5_Setting.Value(), 'd', 14)
        offset3 = cnv.vb(self.spinBox_Biasoffset10_Setting.Value(), 'd', 14)
        offset4 = cnv.vb(self.spinBox_Biasoffset10_Setting.Value(), 'd', 10)
        self.loadOffset_signal.emit(offset1, offset2, offset3, offset4)
    
    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit()
        event.accept()
    
    # Enable serial
    def enable_serial(self, enable):
        self.loadButton.setEnabled(enable)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySetting()
    window.show()
    sys.exit(app.exec_())
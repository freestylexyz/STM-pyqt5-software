
# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:48:07
@Author   : milier00
@FileName : Setting.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QWidget, QApplication
from PyQt5.QtCore import pyqtSignal
from Setting_ui import Ui_Setting
import serial
import serial.tools.list_ports
import conversion as cnv


# mySetting class - It handles setting view operation, pack data and send data to main controller through signal
class mySetting(QWidget, Ui_Setting):
    initDSP_signal = pyqtSignal(str, int)               # Intial DSP signal, send out com port and baudrate
    loadOffset_signal = pyqtSignal(int, int, int, int)  # Load offset signal, send out offset data for +/-2.5V, +/-5V and I set offset
    close_signal = pyqtSignal()                         # Close setting signal

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()
    
    # Initial UI for setting
    def init_UI(self):
        self.initButton.clicked.connect(self.initButton_slot)   # Coneect initial button to slot
        self.loadButton.clicked.connect(self.loadButton_slot)   # Connect load offset button to slot 
        self.comboBox_uart.activated.connect(self.find_port)    # Active updating COM port combo box
    
    # Initial setting
    def init_setting(self, succeed, port, baudrate, offset):
        if succeed:     # Set up view in case of successfully finding DSP
            self.comboBox_uart.setCurrentText(port)             # Set COM port combo box to indicate current DSP COM port
            self.comboBox_baud.setCurrentText(str(baudrate))    # Set baud rate combo box to indicate current DSP baudrate
            self.comboBox_uart.setEnabled(False)                # Disable COM port combo box
            self.comboBox_baud.setEnabled(False)                # Disable baudrate combo box
            self.initButton.setEnabled(False)                   # Disable initial button
            self.loadButton.setEnabled(True)                    # Enable load offset button
            self.spinBox_Biasoffset25_Setting.setEnabled(True)  # Enable bias +/-2.5V offset spin box
            self.spinBox_Biasoffset5_Setting.setEnabled(True)   # Enable bias +/-5V offset spin box
            self.spinBox_Biasoffset10_Setting.setEnabled(True)  # Enable bias +/-10V offset spin box
            self.spinBox_Isetoff_Setting.setEnabled(True)       # Enable I set offset spin box
            
            # Need convert integers to number
            offset1 = cnv.bv(offset[14], 'd', 14)
            offset2 = cnv.bv(offset[9], 'd', 14)
            offset3 = cnv.bv(offset[10], 'd', 14)
            offset4 = cnv.bv(offset[15], 'd', 10)
            
            # Load current DSP stored offset for indication
            self.spinBox_Biasoffset25_Setting.setValue(offset1)
            self.spinBox_Biasoffset5_Setting.setValue(offset2)
            self.spinBox_Biasoffset10_Setting.setValue(offset3)
            self.spinBox_Isetoff_Setting.setValue(offset4)
        else:       # In case of not finding DSP with default setting
            self.find_port() # Find current connected serial device

    # Find available ports and add them into comboBox list
    def find_port(self):
        port_list = [port[0] for port in serial.tools.list_ports.comports()]    # Find avaible ports
        self.comboBox_uart.clear()                                              # Clear existing COM port combo box item
        self.comboBox_uart.addItems(port_list)                                  # Load available ports to COM port combo box

    # Initial DSP button slot
    def initButton_slot(self):
        port = self.comboBox_uart.currentText()                     # Obtain COM port
        baudrate = int(self.comboBox_baud.currentText(), base=10)   # Obtain baudrate
        self.initDSP_signal.emit(port, baudrate)                    # Send initial siganl to main controller
        
    # Load offset button slot
    def loadButton_slot(self):
        # Obtain offset bits
        offset1 = cnv.vb(self.spinBox_Biasoffset25_Setting.Value(), 'd', 14)
        offset2 = cnv.vb(self.spinBox_Biasoffset5_Setting.Value(), 'd', 14)
        offset3 = cnv.vb(self.spinBox_Biasoffset10_Setting.Value(), 'd', 14)
        offset4 = cnv.vb(self.spinBox_Isetoff_Setting.Value(), 'd', 10)
        
        # Send load offset signal to main controller
        self.loadOffset_signal.emit(offset1, offset2, offset3, offset4)
    
    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit()    # Emit close setting signal
        event.accept()              # Accept close event
    
    # Enable serial
    def enable_serial(self, enable):
        self.loadButton.setEnabled(enable)  # Enable load offset button


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySetting()
    window.show()
    sys.exit(app.exec_())
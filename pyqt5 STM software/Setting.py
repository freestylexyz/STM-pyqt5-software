
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



class mySetting(QWidget, Ui_Setting):

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        # self.init_UI()

    # auto find available ports and add them into comboBox list
    def find_port(self):
        port_list = [port[0] for port in serial.tools.list_ports.comports()]
        # print(port_list)
        self.comboBox_uart.clear()
        self.comboBox_uart.addItems(port_list)

    # not called for now
    def init_UI(self):
        self.settings = QSettings("config.ini", QSettings.IniFormat)
        self.baud = self.settings.value("SETUP/BAUD_VALUE")
        self.comboBox_baud.setCurrentText(self.baud)
        # self.comboBox_baud.currentIndexChanged.connect(self.combox_baud_cb)
        # self.loadButton.clicked.connect(self.btn_test_cb)

    # pop up a dialog to show dsp_succeed_signal
    def popdialog(self, ifsucceed):
        if ifsucceed:
            QMessageBox.information(self, "Reminder", "DSP initialized successfully!")
        else:
            QMessageBox.information(self, "Reminder", "DSP initialization failed!")

    # not called for now
    def combox_baud_cb(self):
        self.baud = self.comboBox_baud.currentText()

    # not called for now
    def btn_test_cb(self):
        self.settings.setValue("SETUP/BAUD_VALUE", self.baud)
        QMessageBox.information(self, "Reminder", "QSettings loaded successfully!")



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySetting()
    window.show()
    sys.exit(app.exec_())
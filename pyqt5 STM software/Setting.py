
# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:48:07
@Author   : milier00
@FileName : Setting.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication , QWidget, QMessageBox
from PyQt5.QtCore import pyqtSignal , Qt
from PyQt5.QtCore import Qt , QSettings
from Setting_ui import Ui_Setting
import serial
import serial.tools.list_ports
from time import sleep



class mySetting(QWidget, Ui_Setting):

    def __init__(self):
        super().__init__()
        self.setupUi(self)

    def find_port(self):
        port_values = []
        port_list = list(serial.tools.list_ports.comports())
        pass

    def init_UI(self):
        self.settings = QSettings("config.ini", QSettings.IniFormat)
        self.baud = self.settings.value("SETUP/BAUD_VALUE")
        self.comboBox_baud.setCurrentText(self.baud)
        self.comboBox_baud.currentIndexChanged.connect(self.combox_baud_cb)
        self.loadButton.clicked.connect(self.btn_test_cb)

    def combox_baud_cb(self):
            self.baud = self.comboBox_baud.currentText()

    def btn_test_cb(self):
            self.settings.setValue("SETUP/BAUD_VALUE", self.baud)
            QMessageBox.information(self, "Reminder", "QSettings loaded successfully!")



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySetting()
    window.show()
    sys.exit(app.exec_())
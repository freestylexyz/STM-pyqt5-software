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

        def check_valid_uart():
            # com select
            port_values = []

            port_list = list(serial.tools.list_ports.comports())
            length = len(port_list)
            if (0 == len(port_list)):
                print("can't find serial port")
                self.comboBox_uart.setCurrentIndex(-1)
                self.comboBox_uart.clear()
            else:
                for i in range(0, len(port_list)):
                    port_values.append(port_list[i][0])

                for i in range(len(port_list)):
                    index = self.comboBox_uart.findText(port_values[i], Qt.MatchFixedString)
                    if (index < 0):
                        current_text = self.comboBox_uart.currentText()
                        self.comboBox_uart.addItem(port_values[i])
                        if ("" == current_text):
                            # print("current text is NULL")
                            sleep(0.04)
                            self.comboBox_uart.setCurrentIndex(-1)
                        else:
                            print("current text is ", current_text)
                # remove eut
                count = self.comboBox_uart.count()
                for i in range(count):
                    data = self.comboBox_uart.itemText(i)
                    is_uart_exist = 0
                    for j in range(length):
                        if (data == port_values[j]):
                            is_uart_exist = 1
                    if (0 == is_uart_exist):
                        # remove1
                        self.comboBox_uart.removeItem(i)
                        current_data = self.comboBox_uart.currentText()
                        if (current_data == data or current_data == ""):
                            self.comboBox_uart.clear()
        # find COM
        check_valid_uart()

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
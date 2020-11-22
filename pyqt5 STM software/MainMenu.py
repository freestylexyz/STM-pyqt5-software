# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:47:27
@Author   : milier00
@FileName : MainMenu.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication , QMainWindow
from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSignal , Qt , QMetaObject
from Setting import mySetting
from TipApproach import myTipApproach
from Etest import myEtest
from MainMenu_ui import Ui_HoGroupSTM
from DigitalSignalProcessor import myDSP

class myMainMenu(QMainWindow, Ui_HoGroupSTM):

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.actionSetting.triggered['bool'].connect(self.open_setting)
        self.actionTipAppr.triggered['bool'].connect(self.open_tipappr)
        self.actionEtest.triggered['bool'].connect(self.open_etest)

        self.dsp = myDSP()
        self.settingwidget = mySetting()
        self.tipapprwidget = myTipApproach()
        self.etestwidget = myEtest()

        self.init_setting()

    def open_setting(self):
        self.settingwidget.show()

    def open_tipappr(self):
        self.tipapprwidget.show()

    def open_etest(self):
        self.etestwidget.show()

    # update setting view, or reinit dsp manually, based on dsp_succeed flag, and popup a messagebox
    def init_setting(self):
        if self.dsp.succeed:    # try "if False:" to test it
            self.settingwidget.comboBox_uart.setCurrentText(str(self.dsp.port))
            self.settingwidget.comboBox_baud.setCurrentText(str(self.dsp.baudrate))
            self.settingwidget.comboBox_uart.setEnabled(False)
            self.settingwidget.comboBox_baud.setEnabled(False)
            self.settingwidget.initButton.setEnabled(True)
            self.settingwidget.loadButton.setEnabled(True)
            self.settingwidget.spinBox_Biasoffset25_Setting.setEnabled(True)
            self.settingwidget.spinBox_Biasoffset5_Setting.setEnabled(True)
            self.settingwidget.spinBox_Biasoffset10_Setting.setEnabled(True)
            self.settingwidget.spinBox_Isetoff_Setting.setEnabled(True)
            self.settingwidget.spinBox_Biasoffset25_Setting.setValue(self.dsp.offset[3])
            self.settingwidget.spinBox_Biasoffset5_Setting.setValue(self.dsp.offset[8])
            self.settingwidget.spinBox_Biasoffset10_Setting.setValue(self.dsp.offset[13])
            self.settingwidget.spinBox_Isetoff_Setting.setValue(self.dsp.offset[-1])
        else:
            self.settingwidget.find_port()                                      # ask setting to auto find available ports
        self.settingwidget.initButton.clicked.connect(self.reinit_dsp)          # initButton clicked --> reinitialize dsp
        self.dsp.succeed_signal.connect(self.dsp_signal_slot)                   # dsp initialed --> popup messagebox displaying result
        self.settingwidget.loadButton.clicked.connect(self.setting_signal_slot) # loadButton clicked --> load offset to dsp

    # manually select port and baudrate and reinitialize dsp
    def reinit_dsp(self):
        port = self.settingwidget.comboBox_uart.currentText()
        baudrate = int(self.settingwidget.comboBox_baud.currentText())
        # print(port,baudrate)
        self.dsp.intial_dsp(port, baudrate)
        # print(self.dsp.baudrate,self.dsp.port)

    # ask setting to popup a messagebox to show dsp_succeed_signal
    def dsp_signal_slot(self, ifsucceed):
        self.settingwidget.popdialog(ifsucceed)

    # load offset to DSP status variable and perform related operation
    def setting_signal_slot(self):
        pass



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myMainMenu()
    window.show()
    sys.exit(app.exec_())
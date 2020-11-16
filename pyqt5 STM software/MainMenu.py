# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:47:27
@Author   : milier00
@FileName : MainMenu.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication , QMainWindow
from PyQt5.QtCore import pyqtSignal , Qt , QMetaObject
from Setting import mySetting
from TipApproach import myTipApproach
from Etest import myEtest
from MainMenu_ui import Ui_HoGroupSTM


class myMainMenu(QMainWindow, Ui_HoGroupSTM):

    def __init__(self):
        super().__init__()
        self.setupUi(self)

    def init_UI(self):
        self.actionSetting.triggered['bool'].connect(self.open_setting)
        self.actionTipAppr.triggered['bool'].connect(self.open_tipappr)
        self.actionEtest.triggered['bool'].connect(self.open_etest)
        QMetaObject.connectSlotsByName(self)

        self.settingwidget = mySetting()
        self.tipapprwidget = myTipApproach()
        self.etestwidget = myEtest()

    def open_setting(self):
        self.settingwidget.show()

    def open_tipappr(self):
        self.tipapprwidget.show()

    def open_etest(self):
        self.etestwidget.show()



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myMainMenu()
    window.show()
    sys.exit(app.exec_())
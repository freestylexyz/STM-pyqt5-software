# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:47:27
@Author   : milier00
@FileName : MainMenu.py
"""
import sys
sys.path.append("./ui/")
from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow, QDesktopWidget
from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSignal, Qt, QMetaObject, QSettings
from Setting import mySetting
from TipApproach import myTipApproach
from Etest import myEtest
from MainMenu_ui import Ui_HoGroupSTM
from DigitalSignalProcessor import myDSP
from Scan import myScan
import conversion as cnv

class myMainMenu(QMainWindow, Ui_HoGroupSTM):

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_mainMenu()
        
    def init_mainMenu(self):
        # Initial flags
        self.preamp_gain = 9        # Preamp gain flag gain 8(8), gain 9(9), gain(10)
        self.bias_dac = False       # Bias DAC selection 16bit DAC (False), 20bit DAC (True)
        self.mode = 0               # Software operation mode: None(0), Setting(-1), Electronics test(1)
                                    # Tip approach(2), Scan(3)
                                    
        # Instantiation                            
        self.setting = mySetting()                                  # Setting window
        self.tipappr = myTipApproach()                              # Tip approach window
        self.etest = myEtest()                                      # Electronic test window
        self.scan = myScan()                                        # Scan window
        self.dsp = myDSP()                                          # DSP module
        self.cnfg = QSettings("config.ini", QSettings.IniFormat)    # Configuration module

        self.init_mainui()            # Set up main menu UI

    # Set up UI for main menu
    def init_mainui(self):
        screen = QDesktopWidget().screenGeometry()
        size = self.frameGeometry()
        self.move(int((screen.width()-size.width())/2), int((screen.height()-size.height())/2))
        self.setFixedSize(self.width(), self.height())

        self.init_menu_control()      # Init control menu

    # Enable control  menu to call out docks
    def init_menu_control(self):
        actionCurrent = self.Current.toggleViewAction()
        self.menuControl.addAction(actionCurrent)
        self.Current.setVisible(False)
        actionBias = self.Bias.toggleViewAction()
        self.menuControl.addAction(actionBias)
        self.Bias.setVisible(False)
        actionZ = self.Zcontrol.toggleViewAction()
        self.menuControl.addAction(actionZ)
        self.Zcontrol.setVisible(False)
        self.actionShowAll = QtWidgets.QAction("Show All",self)
        self.menuControl.addAction(self.actionShowAll)
        self.actionShowAll.triggered.connect(self.show_all_dock)

    # Function for "Show all" under menu Control
    def show_all_dock(self):
        self.Current.setVisible(True)
        self.Bias.setVisible(True)
        self.Zcontrol.setVisible(True)

    # Enable serial window action
    def enable_serial_window(self, enable):
        self.menuTest.setEnabled(enable)
        self.menuTip_Approach.setEnabled(enable)
        self.menuScan.setEnabled(enable)
        self.menuSetting.setEnabled(enable)

    # !!! Enable dock widgets serial buttons
    def enable_dock_serial(self, enable):
        # Take care of all related dock buttons
        pass
        
    # Enable current mode related serial
    def enable_mode_serial(self, enable):
        if self.mode == 2:
            self.tipappr.enable_serial(enable)
        elif self.mode == 3:
            self.scan.enable_serial(enable)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myMainMenu()
    window.show()
    sys.exit(app.exec_())
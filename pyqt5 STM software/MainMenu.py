# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:47:27
@Author   : milier00
@FileName : MainMenu.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication, QMainWindow
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
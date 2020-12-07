# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:47:27
@Author   : milier00
@FileName : MainMenu.py
"""
import sys
sys.path.append("./ui/")
from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow, QDesktopWidget, QMessageBox
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
        self.init_UI()            # Set up main menu UI
        self.init_mainMenu()
        
    def init_mainMenu(self):
        # Initial flags
        self.initO = True           # Inital output
        self.idling = True          # Dock idling flag
        self.hard_retracted = False # Hard retract flag
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

        

    # Set up UI for main menu
    def init_UI(self):
        screen = QDesktopWidget().screenGeometry()
        sapcerVer = int(screen.width()*0.006)
        spacerHor = int(screen.height()*0.01)

        # Init Main menu size and position
        sizeMain = self.frameGeometry()
        self.move(sapcerVer, spacerHor)
        self.setFixedSize(self.width(), self.height())

        # Init control menubar
        self.Current.setVisible(False)
        self.Bias.setVisible(False)
        self.Zcontrol.setVisible(False)

        # Init Current dock size and position
        self.Current.resize(430, 360)
        sizeCurrent = self.Current.frameGeometry()
        self.Current.move( screen.width()-sizeCurrent.width()-sapcerVer, spacerHor)
        self.Current.setFixedSize(self.Current.width(), self.Current.height())

        # Init bias dock size and position
        self.Bias.resize(430, 460)
        sizeBias = self.Bias.geometry()
        self.Bias.move(screen.width()-sizeBias.width()-sapcerVer, sizeCurrent.height() + 2 * spacerHor)
        self.Bias.setFixedSize(self.Bias.width(), self.Bias.height())
        
        # Init Z control dock size and poisition
        self.Zcontrol.resize(430, 460)
        self.Zcontrol.move(sapcerVer, sizeMain.height()+2*spacerHor)
        self.Zcontrol.setFixedSize(self.Zcontrol.width(), self.Zcontrol.height())

    # Initial succeed message window
    def succeed_message(self, succeed):
        msgBox = QMessageBox()                          # Creat a message box
        msgBox.setIcon(QMessageBox.Information)         # Set icon
        if succeed:
            msgBox.setText("Successfully found DSP")    # Successful finding DSP infomation
        else:
            msgBox.setText("No DSP found")              # Fail to find DSP information
        msgBox.setWindowTitle("DSP initial message")    # Set title
        msgBox.setStandardButtons(QMessageBox.Ok)       # OK button
        msgBox.exec_()

    # Enable menu bar
    def enable_menubar(self, enable):
        self.menuTest.setEnabled(enable)
        self.menuTip_Approach.setEnabled(enable)
        self.menuScan.setEnabled(enable)
        self.menuSetting.setEnabled(enable)
        

    # Enable serial related features in all docks
    def enable_dock_serial(self, enable):
        self.enable_bias_serial(enable)             # Bias dock
        self.enable_current_serial(enable)          # Current dcok
        self.enable_Zcontrol_serial(enable)         # Z control dock
        self.menuControl.setEnabled(enable)         # Controll menu
    
    # Enable all serial related component in bias dock
    def enable_bias_serial(self, enable):
        self.groupBox_Bias_Bias.setEnabled(enable)
        self.groupBox_Dither_Bias.setEnabled(enable)
        self.groupBox_Range_Bias.setEnabled(enable)
        self.groupBox_Bias_Bias.setEnabled(enable)
        self.pushButton_Rampto1_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto2_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto3_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto4_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto5_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto6_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto7_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto8_BiasRamp.setEnabled(enable)
        self.pushButton_StopRamp_BiasRamp.setEnabled(enable)
        
    # Enable all serial related component in current dock
    def enable_current_serial(self, enable):
        self.groupBox_Gain_Current.setEnabled(enable)
        self.groupBox_Setpoint_Current.setEnabled(enable)
        self.pushButton_Rampto1_CurrRamp.setEnabled(enable)
        self.pushButton_Rampto2_CurrRamp.setEnabled(enable)
        self.pushButton_Rampto3_CurrRamp.setEnabled(enable)
        self.pushButton_Rampto4_CurrRamp.setEnabled(enable)
        self.pushButton_StopRamp_CurrRamp.setEnabled(enable)
    
    # Enable all serial related component in Z control dock
    def enable_Zcontrol_serial(self, enable):
        self.groupBox_Gain_Zcontrol.setEnabled(enable)
        self.groupBox_Feedack_Zcontrol.setEnabled(enable)
        self.groupBox_Retract_Zcontrol.setEnabled(enable)
        self.groupBox_Dither_Zcontrol.setEnabled(enable)
        self.pushButton_Send_Zoffset.setEnabled(enable)
        self.pushButton_Zauto_Zoffset.setEnabled(enable)
        self.pushButton_HardRetract_Zoffset.setEnabled(enable)
        self.pushButton_Advance_Zoffsetfine.setEnabled(enable)
        self.pushButton_Zero_Zoffsetfine.setEnabled(enable)
        self.pushButton_MatchCurrent_Zoffsetfine.setEnabled(enable)
    
    # Enable serial related features in current mode
    def enable_mode_serial(self, enable):
        if self.mode == 2:
            self.enable_dock_serial(enable)     # All docks
            self.tipappr.enable_serial(enable)  # Tip approach
        elif self.mode == 3:
            self.scan.enable_serial(enable)     # All docks
            self.enable_dock_serial(enable)     # Scan
            self.menuScan.setEnabled(enable)    # Scan menu
        elif self.mode == 0:
            self.enable_dock_serial(enable)     # All docks
            self.enable_menubar(enable)         # All windows


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myMainMenu()
    window.show()
    sys.exit(app.exec_())
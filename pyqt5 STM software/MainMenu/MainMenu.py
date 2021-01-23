# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:47:27
@Author   : milier00
@FileName : MainMenu.py
"""
import sys
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
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
import math

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
        self.groupBox_DAC_Bias.setEnabled(enable and (self.mode != 3))
        self.groupBox_Dither_Bias.setEnabled(enable)
        self.groupBox_Range_Bias.setEnabled(enable)
        self.pushButton_Rampto1_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto2_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto3_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto4_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto5_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto6_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto7_BiasRamp.setEnabled(enable)
        self.pushButton_Rampto8_BiasRamp.setEnabled(enable)
        self.pushButton_StopRamp_BiasRamp.setEnabled(False)
        
    # Enable all serial related component in current dock
    def enable_current_serial(self, enable):
        self.groupBox_Gain_Current.setEnabled(enable)
        self.groupBox_Setpoint_Current.setEnabled(enable)
        self.pushButton_Rampto1_CurrRamp.setEnabled(enable)
        self.pushButton_Rampto2_CurrRamp.setEnabled(enable)
        self.pushButton_Rampto3_CurrRamp.setEnabled(enable)
        self.pushButton_Rampto4_CurrRamp.setEnabled(enable)
        self.pushButton_StopRamp_CurrRamp.setEnabled(False)
    
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
            
    # stop button slot
    def stop_slot(self):
        self.dsp.stop = True
        
    # Set up basic view
    def init_dock(self):
        self.init_bias()            # Sync bias dock view with dsp
        self.init_current()         # Sync current dock view with dsp
        self.init_Zcontroller()     # Sync Z controller dock view with dsp
    
    #
    # Bias
    #
    # Init bias UI
    def init_bias(self):
        # Enable serial related modules
        self.enable_bias_serial(self.dsp.succeed)
        
        # Set up UI
        self.radioButton_ON_BiasDither.setChecked(self.dsp.lastdigital[0])      # Set up bias dither ON radio button
        self.radioButton_OFF_BiasDither.setChecked(not self.dsp.lastdigital[0]) # Set up bias dither OFF radio button
        self.radioButton_16bit_DAC.setChecked(not self.bias_dac)                # Set up 16bit DAC selection radio button
        self.radioButton_20bit_DAC.setChecked(self.bias_dac)                    # Set up 20bit DAC selection radio buttion
        self.bias_dac_slot(self.bias_dac)                                       # Set all other widgets based on bias DAC selection flag

    # Bias selection radio button slot
    def bias_dac_slot(self, checked):
        self.bias_dac = checked     # Change bias selection flag
        self.bias_spinbox_range()   # Set up all bias spin boxes range
        self.bias_range_radio()     # Set up bias range radio buttons
        
        # Set up scroll bar and main spin box value
        if self.bias_dac:   # 20bit DAC case
            self.scrollBar_Input_Bias.setMaximum(0xfffff)                       # Set scroll bar maximum first
            self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)              # Set scroll bar value
            self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.last20bit, '20'))  # Set spin box value
            self.spinBox_SpeedInput_BiasRamp.setValue(30)                       # Set default ramp speed
            self.scrollBar_Input_Bias.setPageStep(2500)                         # Set scroll bar page step
        else:
            self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])    # Set scroll bar value first
            self.scrollBar_Input_Bias.setMaximum(0xffff)                # Set scroll bar maximum
            self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.lastdac[13], 'd', self.dsp.dacrange[13]))  # Set spin box value
            self.spinBox_SpeedInput_BiasRamp.setValue(2)                # Set default ramp speed
            self.scrollBar_Input_Bias.setPageStep(150)                  # Set scroll bar page step
    
    #
    # Current
    #
    # Init current UI
    def init_current(self):
        self.enable_current_serial(self.dsp.succeed)    # Enable serial related modules
        self.current_set_radio()                        # Set up preamp gain radio button
        self.current_spinbox_range()                    # Set up all spin boxes range
        
        bits = self.dsp.lastdac[5]                      # Fetch current I ser bits
        self.scrollBar_Input_Setpoint.setValue(0xffff - bits)    # Set up scroll bar
        self.spinBox_Input_Setpoint.setValue(self.b2i(bits, self.preamp_gain))  # Set up main spin box 
        
    # Set radio button for preamp gain
    def current_set_radio(self):
        if self.preamp_gain == 8:
            self.radioButton_8_Gain.setChecked(True)    # Set gain 8 radio button if gain 8
        elif self.preamp_gain == 9:
            self.radioButton_9_Gain.setChecked(True)    # Set gain 9 radio button if gain 9
        elif self.preamp_gain == 10:
            self.radioButton_10_Gain.setChecked(True)   # Set gain 10 radio button if gain 10
    
    # Convert I set bits to current setpoint based on current status
    def b2i(self, bits, gain):
        value = cnv.bv(bits, 'd', self.dsp.dacrange[5])     # Conver bits to voltage
        
        # Determin mulitplier based on gain
        if gain == 8:
            multiplier = 10.0
        elif gain == 9:
            multiplier = 1.0
        elif gain == 10:
            multiplier = 0.1
        return 10.0 ** (-value / 10.0) * multiplier     # Return current setpoint
    
    # Convert current setpoint to I set bits based on current status
    def i2b(self, value, gain):
        # Determin mulitplier based on gain
        if gain == 8:
            multiplier = 10.0
        elif gain == 9:
            multiplier = 1.0
        elif gain == 10:
            multiplier = 0.1
        value = value / multiplier                      # Divide multiplier
        value = -10.0 * math.log(value, 10)             # Calculate I set voltage
        bits = cnv.vb(value, 'd', self.dsp.dacrange[5]) # Convert it to I set bits
        return bits
        
    # Set up all spin boxes input range
    def current_spinbox_range(self):
        # Determin set point limit based on current preamp gain
        minimum = self.b2i(0xffff, self.preamp_gain)
        maximum = self.b2i(0x0, self.preamp_gain)

        # Set minimum
        self.spinBox_Input_Setpoint.setMinimum(minimum)
        self.spinBox_Input1_CurrRamp.setMinimum(minimum)
        self.spinBox_Input2_CurrRamp.setMinimum(minimum)
        self.spinBox_Input3_CurrRamp.setMinimum(minimum)
        self.spinBox_Input4_CurrRamp.setMinimum(minimum)

        # Set maximum
        self.spinBox_Input_Setpoint.setMaximum(maximum)
        self.spinBox_Input1_CurrRamp.setMaximum(maximum)
        self.spinBox_Input2_CurrRamp.setMaximum(maximum)
        self.spinBox_Input3_CurrRamp.setMaximum(maximum)
        self.spinBox_Input4_CurrRamp.setMaximum(maximum)
    
    #
    # Z controller
    #
    # Init Z controller UI
    def init_Zcontroller(self):
        # Enable serial related modules
        self.enable_Zcontrol_serial(self.dsp.succeed)
        
        # Load status
        self.scrollBar_Input_Zoffset.setValue(self.dsp.lastdac[3] - 0x8000)
        self.spinBox_Input_Zoffset.setValue(self.dsp.lastdac[3] - 0x8000)
        self.spinBox_Indication_Zoffset.setValue(self.dsp.lastdac[3] - 0x8000)
        self.spinBox_Input_Zoffsetfine.setValue(self.dsp.lastdac[2] - 0x8000)
        self.slider_Input_Zoffsetfine.setValue(self.dsp.lastdac[2] - 0x8000)
        self.radioButton_ON_ZDither.setChecked(self.dsp.lastdigital[1])
        self.radioButton_OFF_ZDither.setChecked(not self.dsp.lastdigital[1])
        self.radioButton_ON_Feedback.setChecked(self.dsp.lastdigital[2])
        self.radioButton_OFF_Feedback.setChecked(not self.dsp.lastdigital[2])
        self.radioButton_ON_Retract.setChecked(self.dsp.lastdigital[3])
        self.radioButton_OFF_Retract.setChecked(not self.dsp.lastdigital[3])
        self.load_z1_gain()
        self.load_z2_gain()
        
        # Hard retract
        if self.hard_retracted:
            self.pushButton_HardRetract_Zoffset.setText("Hard unretract")
        else:
            self.pushButton_HardRetract_Zoffset.setText("Hard retract")

    # Load Z1 gain from DSP
    def load_z1_gain(self):
        if self.dsp.lastgain[2] == 0:
            self.radioButton_Z1gain10_Gain.setChecked(True)
        elif self.dsp.lastgain[2] == 1:
            self.radioButton_Z1gain1_Gain.setChecked(True)
        elif self.dsp.lastgain[2] == 3:
            self.radioButton_Z1gain01_Gain.setChecked(True)
    
    # Load Z2 gain from DSP
    def load_z2_gain(self):
        if self.dsp.lastgain[3] == 0:
            self.radioButton_Z2gain01_Gain.setChecked(True)
        elif self.dsp.lastgain[3] == 1:
            self.radioButton_Z2gain1_Gain.setChecked(True)
        elif self.dsp.lastgain[3] == 3:
            self.radioButton_Z2gain10_Gain.setChecked(True)
    
        

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myMainMenu()
    window.show()
    sys.exit(app.exec_())
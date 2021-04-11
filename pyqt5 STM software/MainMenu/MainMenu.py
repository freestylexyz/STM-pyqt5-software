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
from PyQt5.QtWidgets import QApplication, QMainWindow, QDesktopWidget, QMessageBox
from PyQt5.QtCore import QSettings
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
        self.init_UI()              # Set up main menu UI
        self.init_mainMenu()
        
    def init_mainMenu(self):
        # Initial flags
        self.initO = True           # Initial output
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
        self.cnfgBS = QSettings("configBS.ini", QSettings.IniFormat)  # Basic configuration module
        self.cnfgEX = QSettings("configEX.ini", QSettings.IniFormat)  # Extended configuration module

    # Set up UI for main menu
    def init_UI(self):
        screen = QDesktopWidget().screenGeometry()
        spacerVer = int(screen.width()*0.006)
        spacerHor = int(screen.height()*0.01)

        # Init Main menu size and position
        sizeMain = self.frameGeometry()
        self.move(spacerVer, spacerHor)
        self.setFixedSize(self.width(), self.height())

        # Init control menu bar
        self.Current.setVisible(False)
        self.Bias.setVisible(False)
        self.Zcontrol.setVisible(False)

        # Init Current dock size and position
        self.Current.resize(430, 360)
        sizeCurrent = self.Current.frameGeometry()
        self.Current.move( screen.width()-sizeCurrent.width()-spacerVer, spacerHor)
        self.Current.setFixedSize(self.Current.width(), self.Current.height())

        # Init bias dock size and position
        self.Bias.resize(430, 460)
        sizeBias = self.Bias.geometry()
        self.Bias.move(screen.width()-sizeBias.width()-spacerVer, sizeCurrent.height() + 2 * spacerHor)
        self.Bias.setFixedSize(self.Bias.width(), self.Bias.height())
        
        # Init Z control dock size and position
        self.Zcontrol.resize(430, 460)
        self.Zcontrol.move(spacerVer, sizeMain.height()+2*spacerHor)
        self.Zcontrol.setFixedSize(self.Zcontrol.width(), self.Zcontrol.height())

    # Pop out a warning message
    def msg(self, text):
        QMessageBox.warning(None, "STM", text, QMessageBox.Ok)

    # Enable menu bar
    def enable_menubar(self, index):
        self.menuTest.setEnabled(index == 0)
        self.menuTip_Approach.setEnabled((index == 0) or (index == 2))
        self.menuScan.setEnabled((index == 0) or (index == 3))
        self.menuSetting.setEnabled(index == 0)
        
    # Enable scan menu
    def enable_scan_menu(self, index):
        self.actionScan.setEnabled(index == 0)
        self.actionSpectra.setEnabled((index == 0) or (index == 1))
        self.actionDeposition.setEnabled((index == 0) or (index == 2))
        self.actionTrack.setEnabled(index == 0)
        # self.actionHop.setEnabled(index == 0)
        # self.actionManipulation.setEnabled(index == 0)

    # Enable serial related features in all docks
    def enable_dock_serial(self, enable):
        self.enable_bias_serial(enable)             # Bias dock
        self.enable_current_serial(enable)          # Current dock
        self.enable_Zcontrol_serial(enable)         # Z control dock
        self.menuControl.setEnabled(enable)         # Control menu
    
    # Enable all serial related component in bias dock
    def enable_bias_serial(self, enable):
        widgets = [self.groupBox_Bias_Bias, self.groupBox_Dither_Bias,\
                   self.pushButton_Rampto1_BiasRamp, self.pushButton_Rampto2_BiasRamp,\
                   self.pushButton_Rampto3_BiasRamp, self.pushButton_Rampto4_BiasRamp,\
                   self.pushButton_Rampto5_BiasRamp, self.pushButton_Rampto6_BiasRamp,\
                   self.pushButton_Rampto7_BiasRamp, self.pushButton_Rampto8_BiasRamp]
        for widget in widgets:
            widget.setEnabled(enable)
        
        self.groupBox_DAC_Bias.setEnabled(enable and (self.mode != 3))  # In scan mode will disable bias DAC selection
        self.groupBox_Range_Bias.setEnabled(enable and (not self.bias_dac)) # 20 bit DAC will disable range selection
        self.pushButton_StopRamp_BiasRamp.setEnabled(False)
        
    # Enable all serial related component in current dock
    def enable_current_serial(self, enable):
        widgets = [self.groupBox_Gain_Current, self.groupBox_Setpoint_Current,\
                   self.pushButton_Rampto1_CurrRamp, self.pushButton_Rampto2_CurrRamp,\
                   self.pushButton_Rampto3_CurrRamp, self.pushButton_Rampto4_CurrRamp]
        for widget in widgets:
            widget.setEnabled(enable)
        self.pushButton_StopRamp_CurrRamp.setEnabled(False)
    
    # Enable all serial related component in Z control dock
    def enable_Zcontrol_serial(self, enable):
        widgets = [self.groupBox_Gain_Zcontrol, self.groupBox_Feedack_Zcontrol,\
                   self.groupBox_Retract_Zcontrol, self.groupBox_Dither_Zcontrol,\
                   self.pushButton_Send_Zoffset, self.pushButton_Zauto_Zoffset,\
                   self.pushButton_HardRetract_Zoffset, self.pushButton_Advance_Zoffsetfine,\
                   self.pushButton_Zero_Zoffsetfine, self.pushButton_MatchCurrent_Zoffsetfine]
        for widget in widgets:
            widget.setEnabled(enable)
    
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
            self.enable_menubar(not enable)     # All windows
            
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
        self.groupBox_DAC_Bias.setEnabled(self.mode != 3)
        
        # Set up UI
        self.radioButton_ON_BiasDither.setChecked(self.dsp.lastdigital[0])      # Set up bias dither ON radio button
        self.radioButton_OFF_BiasDither.setChecked(not self.dsp.lastdigital[0]) # Set up bias dither OFF radio button
        self.radioButton_16bit_DAC.setChecked(not self.bias_dac)                # Set up 16bit DAC selection radio button
        self.radioButton_20bit_DAC.setChecked(self.bias_dac)                    # Set up 20bit DAC selection radio button
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
            self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])            # Set scroll bar value first
            self.scrollBar_Input_Bias.setMaximum(0xffff)                        # Set scroll bar maximum
            self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.lastdac[13], 'd', self.dsp.dacrange[13]))  # Set spin box value
            self.spinBox_SpeedInput_BiasRamp.setValue(2)                        # Set default ramp speed
            self.scrollBar_Input_Bias.setPageStep(150)                          # Set scroll bar page step
    
    # Set up all spin boxes input range
    def bias_spinbox_range(self):
        # Determine input limit based on selected DAC and DAC range
        minimum = cnv.bv(0, '20') if self.bias_dac else cnv.bv(0, 'd', self.dsp.dacrange[13])
        maximum = cnv.bv(0xfffff, '20') if self.bias_dac else cnv.bv(0xffff, 'd', self.dsp.dacrange[13])
            
        spinboxes = [self.spinBox_Input_Bias, self.spinBox_Input1_BiasRamp,\
                self.spinBox_Input2_BiasRamp, self.spinBox_Input3_BiasRamp,\
                self.spinBox_Input4_BiasRamp, self.spinBox_Input5_BiasRamp,\
                self.spinBox_Input6_BiasRamp, self.spinBox_Input7_BiasRamp,\
                self.spinBox_Input8_BiasRamp]
            
        for spin in spinboxes:
            spin.setMinimum(minimum)    # Set minimum for main spin box and ramp spin boxes
            spin.setMaximum(maximum)    # Set maximum for main spin box and ramp spin boxes
    
    # Set bias range radio button
    def bias_range_radio(self):
        if self.bias_dac:       # 20bit DAC case
            self.radioButton_5_Range.setChecked(True)   # Set range to +/-5V
            self.groupBox_Range_Bias.setEnabled(False)  # Disable range selection
        else:                   # 16bit DAC case
            radio_dict = {14: self.radioButton_25_Range, 9: self.radioButton_5_Range, 10: self.radioButton_10_Range}
            radio_dict[self.dsp.dacrange[13]].setChecked(True)      # Set corresponding radio button
            self.groupBox_Range_Bias.setEnabled(self.dsp.succeed)   # Enable range selection when found DSP

    #
    # Current
    #
    # Init current UI
    def init_current(self):
        self.enable_current_serial(self.dsp.succeed)    # Enable serial related modules
        self.groupBox_Gain_Current.setEnabled(True)     # Enable preamp gain selection no matter what
        self.current_set_radio()                        # Set up preamp gain radio button
        self.current_spinbox_range()                    # Set up all spin boxes range
        
        bits = self.dsp.lastdac[5]                      # Fetch current I ser bits
        self.scrollBar_Input_Setpoint.setValue(0xffff - bits)    # Set up scroll bar
        self.spinBox_Input_Setpoint.setValue(cnv.b2i(bits, self.preamp_gain, self.dsp.dacrange[5]))  # Set up main spin box 
        
    # Set radio button for preamp gain
    def current_set_radio(self):
        radio_list = [self.radioButton_8_Gain, self.radioButton_9_Gain, self.radioButton_10_Gain]
        radio_list[self.preamp_gain - 8].setChecked(True)      # Set corresponding radio button
        
    # Set up all spin boxes input range
    def current_spinbox_range(self):
        # Determine set point limit based on current preamp gain
        minimum = cnv.b2i(0xffff, self.preamp_gain, self.dsp.dacrange[5])
        maximum = cnv.b2i(0, self.preamp_gain, self.dsp.dacrange[5])
        spinboxes = [self.spinBox_Input_Setpoint, self.spinBox_Input1_CurrRamp, self.spinBox_Input2_CurrRamp,\
                     self.spinBox_Input3_CurrRamp, self.spinBox_Input4_CurrRamp]
            
        for spin in spinboxes:
            spin.setMinimum(minimum)    # Set minimum for main spin box and ramp spin boxes
            spin.setMaximum(maximum)    # Set maximum for main spin box and ramp spin boxes
    
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
        self.radioButton_ON_ZDither.setChecked(self.dsp.lastdigital[1])
        self.radioButton_OFF_ZDither.setChecked(not self.dsp.lastdigital[1])
        self.radioButton_ON_Feedback.setChecked(self.dsp.lastdigital[2])
        self.radioButton_OFF_Feedback.setChecked(not self.dsp.lastdigital[2])
        self.radioButton_ON_Retract.setChecked(self.dsp.lastdigital[3])
        self.radioButton_OFF_Retract.setChecked(not self.dsp.lastdigital[3])
        
        self.z_offset_update()
        self.z_fine_update()
        self.load_z1_gain()
        self.load_z2_gain()
        
        # Hard retract
        if self.hard_retracted:
            self.pushButton_HardRetract_Zoffset.setText("Hard unretract")
        else:
            self.pushButton_HardRetract_Zoffset.setText("Hard retract")

    # Load Z1 gain from DSP
    def load_z1_gain(self):
        radio_dict = {0: self.radioButton_Z1gain10_Gain, 1: self.radioButton_Z1gain1_Gain, 3: self.radioButton_Z1gain01_Gain}
        radio_dict[self.dsp.lastgain[2]].setChecked(True)      # Set corresponding radio button
    
    # Load Z2 gain from DSP
    def load_z2_gain(self):
        radio_dict = {0: self.radioButton_Z2gain01_Gain, 1: self.radioButton_Z2gain1_Gain, 3: self.radioButton_Z2gain10_Gain}
        radio_dict[self.dsp.lastgain[3]].setChecked(True)      # Set corresponding radio button
        
    # Z offset update
    def z_offset_update(self):
        self.label_Indication_Zoffset.setText(str(self.dsp.lastdac[3] - 0x8000))  # Update Z offset indication label
        
    # Z offset fine update
    def z_fine_update(self):
        self.label_Indication_Zoffsetfine.setText(str(self.dsp.lastdac[2] - 0x8000))    # Update Z offset fine indication label
        self.slider_Input_Zoffsetfine.setValue(self.dsp.lastdac[2] - 0x8000)            # Update slider value

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myMainMenu()
    window.show()
    sys.exit(app.exec_())
# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:46:49
@Author   : milier00
@FileName : main.py
"""
import sys
sys.path.append("./ui/")
import os
from PyQt5.QtWidgets import QApplication, QDesktopWidget, QMessageBox
from BiasControl import myBiasControl
from Zcontroller import myZcontroller
from CurrentControl import myCurrentControl
from SettingControl import mySettingControl
from EtestControl import myEtestControl
from TipApproachControl import myTipApproachControl
from ScanControl import myScanControl
import conversion as cnv
from logger import Logger

class mySTM(myBiasControl, myZcontroller, myCurrentControl, mySettingControl, myEtestControl, myTipApproachControl, myScanControl):
    def __init__(self, parent=None):
        super().__init__()
        self.init_menu()
        self.init_bias_dock()
        self.init_Zcontroller_dock()
        self.init_current_dock()
        self.init_STM()
        
    def init_menu(self):
        # Menu bar
        self.actionSetting.triggered['bool'].connect(self.open_setting)     # Connect open setting window
        self.actionTipAppr.triggered['bool'].connect(self.open_tipappr)     # Connect open tip approach window
        self.actionEtest.triggered['bool'].connect(self.open_etest)         # Connect open electronic test window
        
        self.actionScan.triggered['bool'].connect(self.open_scan)
        self.actionDeposition.triggered['bool'].connect(self.open_deposition)
        self.actionSpectra.triggered['bool'].connect(self.open_spc)
        self.actionTrack.triggered['bool'].connect(self.open_track)
        self.actionHop.triggered['bool'].connect(self.open_hop)
        self.actionManipulation.triggered['bool'].connect(self.open_manipulation)
        
        # Connect open control dock window
        self.actionShow_All_A.triggered.connect(self.show_all_dock)         # Connect open all docks
        self.actionBias.triggered['bool'].connect(self.bias_show)           # Connect open bias dock
        self.actionZ.triggered['bool'].connect(self.Zcontroller_show)       # Connect open Z control dock
        self.actionCurrent.triggered['bool'].connect(self.current_show)     # Connect open current dock

    def init_STM(self):
        # Connect DSP singal
        self.dsp.succeed_signal.connect(self.dsp_succeed_slot)
        # self.dsp.oscc_signal.connect()
        # self.dsp.rampMeasure_signal.connect()
        # self.dsp.giantStep_signal.connect()
        # self.dsp.rampTo_signal.connect()
        # self.dsp.rampDiag_signal.connect()
        
        # Connect setting signal
        self.setting.initDSP_signal.connect(self.setting_init_slot)
        self.setting.loadOffset_signal.connect(self.setting_load_slot)
        self.setting.close_signal.connect(self.closeWindow)
        
        # Connect tip approach signal
        self.tipappr.close_signal.connect(self.closeWindow)
        
        # Connect electronic test signal
        self.etest.close_signal.connect(self.closeWindow)
        
        # Connect scan signal
        self.scan.close_signal.connect(self.close_scan)

        # Do some real stuff
        self.load_config()              # Load DSP settings
        self.dsp.init_dsp(self.initO)   # Try to initial DSP
        # if self.mode_last != 0:
        #     pass
        # !!! self.write_cnfg()         # Update config in real time

    def write_cnfg(self):
        self.cnfg.clear()
        self.cnfg.setValue("CONFIG/BAUD_VALUE",self.dsp.baudrate)
        self.cnfg.setValue("CONFIG/COM_VALUE",self.dsp.port)
        self.cnfg.setValue("SETTING/PREAMP_GAIN",self.preamp_gain)
        self.cnfg.setValue("SETTING/MODE",self.mode)
        self.cnfg.setValue("SETTING/BIAS_DAC",self.bias_dac)
        self.cnfg.sync()

    # !!! Load all settings stored in configuration file to dsp module
    def load_config(self):
        self.dsp.baudrate = self.cnfg.value("CONFIG/BAUD_VALUE")
        self.dsp.port = self.cnfg.value("CONFIG/COM_VALUE")
        # !!! Need to determine if accidently exit and pop out window to let user decide if initialize output
        self.initO = bool(os.stat("initO.log").st_size != 0)  # Get initO from initO.log
        self.preamp_gain = self.cnfg.value("SETTING/PREAMP_GAIN")
        if self.initO:
            pass
        else:
            reply = QMessageBox.question(None,"Load output","Initialize output?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if reply == QMessageBox.Yes:
                self.initO = True
            else:
                self.mode_last = self.cnfg.value("SETTING/MODE")
        
    # DSP intial succeed slot
    def dsp_succeed_slot(self, succeed):
        self.versionLabel.setText(self.dsp.ver)     # Change version label
        if self.mode == -1:                         # Initial from setting
            self.setting.succeed_message(succeed)       # Pop out succeed message from setting
        elif self.mode == 0:                        # Initial form mian menu
            self.succeed_message(succeed)               # Pop out succeed message from main menu

        # Reinital setting view if succeed
        if succeed:
            self.setting.init_setting(self.dsp.succeed, self.dsp.port, self.dsp.baudrate, self.dsp.offset)

    # Close dsp serial port before exit application
    def closeEvent(self, event):
        if self.mode != 3:
            self.Bias.close()
            self.Zcontrol.close()
            self.Current.close()
            self.tipappr.close()
            self.dsp.close()
            self.write_cnfg()
            event.accept()
        else:
            QMessageBox.warning(None,"Reminder","Close Scan window first!", QMessageBox.Ok)
            event.ignore()

    
    # Open setting window
    def open_setting(self):
        if self.mode == 0:
            self.mode = -1
            self.hide_all_dock()
            self.setting.init_setting(self.dsp.succeed, self.dsp.port, self.dsp.baudrate, self.dsp.offset)
            self.setting.show()

    # Open electronic test window
    def open_etest(self):
        if self.mode == 0:
            self.mode = 1
            self.hide_all_dock()
            self.etest.init_etest()
            self.etest.show()

    # pop windown open scan first
    def msg_open_scan(self):
        QMessageBox.warning(None, "Reminder", "Open Scan window first!", QMessageBox.Ok)

    # Open tip approach window
    def open_tipappr(self):
        if self.mode == 0:
            self.mode = 2
            self.enable_menubar(False)
            self.tipappr.init_tipAppr()
            self.tipappr.show()

    # Open scan window
    def open_scan(self):
        if self.mode == 0:
            self.mode = 3
            self.enable_menubar(False)
            self.menuScan.setEnabled(True)
            self.scan.init_scan()
            self.scan.show()
    
    # Open spectroscopy window
    def open_spc(self):
        if self.mode == 3 and self.scan.mode == 0:
            self.scan.mode = 1
            self.menuScan.setEnabled(False)
            self.scan.spc.init_spc()
            self.scan.spc.show()
        else:
            self.msg_open_scan()
            
    # Open deposition window
    def open_deposition(self):
        if self.mode == 3 and self.scan.mode == 0:
            self.scan.mode = 2
            self.menuScan.setEnabled(False)
            self.scan.deposition.init_spc()
            self.scan.deposition.show()
        else:
            self.msg_open_scan()
    
    # Open track window
    def open_track(self):
        if self.mode == 3 and self.scan.mode == 0:
            self.scan.mode = 3
            self.menuScan.setEnabled(False)
            self.scan.track.init_spc()
            self.scan.track.show()
        else:
            self.msg_open_scan()
    
    # Open hop window
    def open_hop(self):
        if self.mode == 3 and self.scan.mode == 0:
            self.scan.mode = 4
            self.menuScan.setEnabled(False)
            self.scan.hop.init_spc()
            self.scan.hop.show()
        else:
            self.msg_open_scan()
    
    # Open manipulation window
    def open_manipulation(self):
        if self.mode == 3:
            self.scan.mode = 5
            self.menuScan.setEnabled(False)
            self.scan.manipulation.init_spc()
            self.scan.manipulation.show()
        else:
            self.msg_open_scan()
            
    # Close serial window  
    def closeWindow(self):
        self.mode = 0
        self.enable_menubar(True)
    
    # Close function for scan mode
    def close_scan(self):
        if self.scan.mode == 0:
            # !!! Do some close sequence
            self.closeWindow()
        else:
            self.scan.mode = 0
            self.menuScan.setEnabled(True)
    
    # Show all dock windows    
    def show_all_dock(self):
        self.bias_show()
        self.Zcontroller_show()
        self.current_show()
    
    # Hide all dock windows    
    def hide_all_dock(self):
        self.Bias.hide()
        self.Zcontrol.hide()
        self.Current.hide()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySTM()
    window.show()
    sys.stdout = Logger("initO.log")    # Temporary method used to record initO value
    print("running!!")
    sys.exit(app.exec_())

'''
About the temporary method for initO:
    initO.log records "running!!" for normal shutdown; nothing for dan's bug
    dan's bug is a trick to make it crash:
    click "Scan" --> click "Spectra/Deposition/Track/Hop/Manipulation"
'''

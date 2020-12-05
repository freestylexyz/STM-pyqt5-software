# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:46:49
@Author   : milier00
@FileName : main.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication
from BiasControl import myBiasControl
from Zcontroller import myZcontroller
from CurrentControl import myCurrentControl
from SettingControl import mySettingControl
from EtestControl import myEtestControl
from TipApproachControl import myTipApproachControl
from ScanControl import myScanControl

class mySTM(myBiasControl, myZcontroller, myCurrentControl, mySettingControl, myEtestControl, myTipApproachControl, myScanControl):
    def __init__(self, parent=None):
        super().__init__()
        self.init_menu()
        self.init_STM()
        self.init_bias_dock()
        self.init_Zcontroller_dock()
        self.init_current_dock()
        
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
        self.actionShowAll.triggered.connect(self.show_all_dock)            # Connect open all docks
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
        self.scan.close_signal.connect(self.closeWindow)

        # Do some real stuff
        self.load_config()              # Load DSP settings
        self.dsp.init_dsp(self.initO)   # Try to initial DSP
        
    # !!! Load all settings stored in configuration file to dsp module
    def load_config(self):
        self.dsp.baudrate = self.cnfg.value("SETUP/BAUD_VALUE")
        # self.dsp.port = 
        # !!! Need to determine if accidently exit and pop out window to let user decide if initialize output
        self.initO = True
        

    def dsp_succeed_slot(self, succeed):
        self.versionLabel.setText(self.dsp.ver)
        # !!! Need to enable and disable buttons and pop dialog
        if self.mode == -1:
            # !!! setting Q massage
            pass
        elif self.mode == 0:
            # !!! mainmenu Q massage
            pass
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
            # !!! Save configuration file
            event.accept()
        else:
            # !!! Pop out window to remind close scan window first
            event.reject()
    
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
    
    # Open tip approach window
    def open_tipappr(self):
        if self.mode == 0:
            self.mode = 2
            self.enable_serial_window(False)
            self.tipappr.init_tipAppr()
            self.tipappr.show()

    # Open scan window
    def open_scan(self):
        if self.mode == 0:
            self.mode = 3
            self.enable_serial_window(False)
            self.menuScan.setEnabled(True)
            self.scan.init_scan()
            self.scan.show()
    
    # Open spectroscopy window
    def open_spc(self):
        if self.mode == 3 and self.scan.mode == 0:
            self.scan.mode = 1
            self.enable_serial_window(False)
            self.scan.spc.init_spc()
            self.scan.spc.show()
        else:
            # !!! pop windown open scan first
            pass
            
    # Open deposition window
    def open_deposition(self):
        if self.mode == 3 and self.scan.mode == 0:
            self.scan.mode = 2
            self.enable_serial_window(False)
            self.scan.deposition.init_spc()
            self.scan.deposition.show()
        else:
            # !!! pop windown open scan first
            pass
    
    # Open track window
    def open_track(self):
        if self.mode == 3 and self.scan.mode == 0:
            self.scan.mode = 3
            self.enable_serial_window(False)
            self.scan.track.init_spc()
            self.scan.track.show()
        else:
            # !!! pop windown open scan first
            pass
    
    # Open hop window
    def open_hop(self):
        if self.mode == 3 and self.scan.mode == 0:
            self.scan.mode = 4
            self.enable_serial_window(False)
            self.scan.hop.init_spc()
            self.scan.hop.show()
        else:
            # !!! pop windown open scan first
            pass
    
    # Open manipulation window
    def open_manipulation(self):
        if self.mode == 3:
            self.scan.mode = 5
            self.enable_serial_window(False)
            self.scan.manipulation.init_spc()
            self.scan.manipulation.show()
        else:
            # !!! pop windown open scan first
            pass
            
    # Close serial window  
    def closeWindow(self):
        self.mode = 0
        self.enable_serial_window(True)
    
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
    sys.exit(app.exec_())


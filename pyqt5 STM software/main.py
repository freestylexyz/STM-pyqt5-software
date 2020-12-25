# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:46:49
@Author   : milier00
@FileName : main.py
"""
import sys
sys.path.append("./ui/")
sys.path.append("./MainMenu/")
sys.path.append("./Setting/")
sys.path.append("./Model/")
sys.path.append("./TipApproach/")
sys.path.append("./Scan/")
sys.path.append("./Etest/")
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
        self.dsp.rampMeasure_signal.connect(self.dsp_rampMeasure_slot)
        self.dsp.giantStep_signal.connect(self.giantStep_update)
        self.dsp.rampTo_signal.connect(self.dsp_rampTo_slot)
        # self.dsp.rampDiag_signal.connect()
        
        # Connect setting signal
        self.setting.initDSP_signal.connect(self.setting_init_slot)
        self.setting.loadOffset_signal.connect(self.setting_load_slot)
        self.setting.close_signal.connect(self.closeWindow)
        
        # Connect tip approach signal
        self.tipappr.close_signal.connect(self.closeWindow)
        self.tipappr.stop_signal.connect(self.stop_slot)
        self.tipappr.mode_signal.connect(self.dsp.digital_o)
        self.tipappr.giant_signal.connect(self.giant_slot)
        self.tipappr.approach_signal.connect(self.tip_appr_slot)
        
        # Connect electronic test signal
        self.etest.Etest.currentChanged.connect(self.init_tab_slot)
        self.etest.close_signal.connect(self.closeWindow)
        # I/O
        self.etest.range_changed_signal.connect(self.range_changed_slot)
        self.etest.ch_changed_signal.connect(self.ch_changed_slot)
        self.etest.digital_changed_signal.connect(self.dsp.digital_o)
        self.etest.gain_changed_signal.connect(self.dsp.gain)
        self.etest.adc_input_signal.connect(self.adc_input_slot)
        self.etest.dac_output_signal.connect(self.dsp.dac_W)
        self.etest.bit20_output_signal.connect(self.dsp.bit20_W)
        # Ramp Test
        self.etest.rtest_ramp_signal.connect(self.rtest_ramp_slot)
        self.etest.stop_signal.connect(self.stop_slot)
        # Square Wave
        self.etest.swave_start_signal.connect(self.swave_start_slot)
        # Oscilloscope
        self.etest.osci_start_signal.connect(self.osci_start_slot)
        self.dsp.oscc_signal.connect(self.osci_update)

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
        self.cnfg.setValue("CONFIG/BAUD_VALUE", self.dsp.baudrate)
        self.cnfg.setValue("CONFIG/COM_VALUE", self.dsp.port)
        self.cnfg.setValue("SETTING/PREAMP_GAIN", self.preamp_gain)
        self.cnfg.setValue("SETTING/MODE", self.mode)
        self.cnfg.setValue("SETTING/BIAS_DAC", self.bias_dac)
        self.cnfg.sync()

    # !!! Load all settings stored in configuration file to dsp module
    def load_config(self):
        self.dsp.baudrate = self.cnfg.value("CONFIG/BAUD_VALUE")
        self.dsp.port = self.cnfg.value("CONFIG/COM_VALUE")
        # !!! Need to determine if accidently exit and pop out window to let user decide if initialize output
        self.initO = bool(os.stat("initO.log").st_size != 0)  # Get initO from initO.log
        # self.initO = True
        # self.bias_dac = self.cnfg.value()
        if self.initO:
            pass
        else:
            reply = QMessageBox.question(None, "STM", "Initialize output?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if reply == QMessageBox.Yes:
                self.initO = True
            else:
                self.mode_last = int(self.cnfg.value("SETTING/MODE"))
                self.preamp_gain = int(self.cnfg.value("SETTING/PREAMP_GAIN"))
        
    # DSP intial succeed slot
    def dsp_succeed_slot(self, succeed):
        self.versionLabel.setText(self.dsp.ver)     # Change version label
        if self.mode == -1:                         # Initial from setting
            self.setting.succeed_message(succeed)       # Pop out succeed message from setting
        elif self.mode == 0:                        # Initial form mian menu
            self.succeed_message(succeed)               # Pop out succeed message from main menu

        # Reinital setting view if succeed
        self.setting.init_setting(self.dsp.succeed, self.dsp.port, self.dsp.baudrate, self.dsp.offset)
            
    # DSP ramp to update signal:
    def dsp_rampTo_slot(self, channel, current):
        # If not in the etest mode
        if self.mode != 1:
            if (channel == 0x1d) or (channel == 0x20):
                self.bias_update()                          # Update bias dock if ramping bias
            elif channel == 0x15:
                self.current_update()                       # Update current dock if ramping I set
            elif channel == 0x12:
                self.z_fine_update()                        # Update Z controller if ramping Z offset fine
            elif channel == 0x13:
                self.z_offset_update()                      # Update Z controller if ramping Z offset
        else:
            # !!! Etest update ramp
            self.ramp_update(channel, current)
            pass

    # DSP ramp measure update signal:
    def dsp_rampMeasure_slot(self, rdata):
        if self.mode == 1:
            self.ramp_read_update(rdata)           # Update etest if in etest mode

    # Close dsp serial port before exit application
    def closeEvent(self, event):
        # if True:                  # !!! For testing use
        if self.mode == 0:          # Can exit software, when in fundamental mode
            self.Bias.close()       # Close bias dock
            self.Zcontrol.close()   # Close Z controller dock
            self.Current.close()    # Close Current dock
            self.dsp.close()        # Terminate DSP serial communication
            self.write_cnfg()       # Write configuration file
            event.accept()          # Accept close event
        else:
            QMessageBox.warning(None,"STM","Close top window first!", QMessageBox.Ok)   # Pop out window to remind close the tip window
            event.ignore()          # Reject close event

    
    # Open setting window
    def open_setting(self):
        # Only can open setting window when software is in fundamental mode
        if self.mode == 0:
            self.mode = -1          # Change the mode variable
            self.hide_all_dock()    # Hide all dock windows
            self.setting.init_setting(self.dsp.succeed, self.dsp.port, self.dsp.baudrate, self.dsp.offset)  # Init setting view
            self.setting.show()     # Show setting window

    # Open electronic test window
    def open_etest(self):
        # only can open electronic test window when software is in fundamental mode
        if self.mode == 0:
            self.mode = 1           # Change the mode variable
            self.hide_all_dock()    # Hide all dock windows
            self.etest.init_etest(self.dsp.succeed, self.dsp.dacrange, self.dsp.adcrange, self.dsp.lastdigital, self.dsp.lastgain, self.dsp.lastdac, self.dsp.last20bit)  # Init etest view
            self.etest.show()       # Show electronics test window

    # pop windown open scan first
    def msg_open_scan(self):
        QMessageBox.warning(None, "STM", "Open Scan window first!", QMessageBox.Ok)

    # Open tip approach window
    def open_tipappr(self):
        # Only can open tip approach window when software is in fundamental mode
        if self.mode == 0:
            self.mode = 2                                   # Change the mode variable
            self.enable_menubar(False)                      # Disable menubar to prevent softeware enter other mode
            self.dsp.init_output()                          # Prepare all electronics to tip approach mode
            self.init_bias()                                # Update bias dock view
            self.init_current()                             # Update current dock view
            self.init_Zcontroller()                         # Update Z controller dock view
            self.tipappr.init_tipAppr(self.dsp.succeed, self.dsp.lastdigital)     # Init tip approach view
            self.tipappr.show()                             # Show tip approach window

    # Open scan window
    def open_scan(self):
        if self.mode == 0:
            self.mode = 3
            self.enable_menubar(False)
            self.menuScan.setEnabled(True)
            self.init_bias()
            self.init_current()
            self.init_Zcontroller()
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

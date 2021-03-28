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
from PyQt5.QtCore import QSettings
from BiasControl import myBiasControl
from Zcontroller import myZcontroller
from CurrentControl import myCurrentControl
from SettingControl import mySettingControl
from EtestControl import myEtestControl
from TipApproachControl import myTipApproachControl
from ScanControl import myScanControl
import conversion as cnv
from logger import Logger
import functools as ft
import numpy as np

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
        self.actionSetting.triggered['bool'].connect(lambda: self.open_window(-1))    # Connect open setting window
        self.actionEtest.triggered['bool'].connect(lambda: self.open_window(1))       # Connect open electronic test window
        self.actionTipAppr.triggered['bool'].connect(lambda: self.open_window(2))     # Connect open tip approach window
        
        self.actionScan.triggered['bool'].connect(lambda: self.open_scan(0))
        self.actionSpectra.triggered['bool'].connect(lambda: self.open_scan(1))
        self.actionDeposition.triggered['bool'].connect(lambda: self.open_scan(2))
        self.actionTrack.triggered['bool'].connect(lambda: self.open_scan(-1))
        # self.actionHop.triggered['bool'].connect(self.open_hop)
        # self.actionManipulation.triggered['bool'].connect(self.open_manipulation)
        
        # Connect open control dock window
        self.actionShow_All_A.triggered.connect(self.show_all_dock)         # Connect open all docks
        self.actionBias.triggered['bool'].connect(self.bias_show)           # Connect open bias dock
        self.actionZ.triggered['bool'].connect(self.Zcontroller_show)       # Connect open Z control dock
        self.actionCurrent.triggered['bool'].connect(self.current_show)     # Connect open current dock

    def init_STM(self):
        # Connect DSP singal
        self.dsp.succeed_signal.connect(self.dsp_succeed_slot)
        self.dsp.oscc_signal.connect(self.dsp_oscc_slot)
        self.dsp.rampMeasure_signal.connect(self.dsp_rampMeasure_slot)
        self.dsp.giantStep_signal.connect(self.giantStep_update)
        self.dsp.rampTo_signal.connect(self.dsp_rampTo_slot)
        self.dsp.track_signal.connect(self.scan.track_update)
        self.dsp.rampDiag_signal.connect(self.scan.send_update)
        self.dsp.scan_signal.connect(self.scan.scan_update)


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
        self.etest.Etest.tabBarClicked.connect(self.init_tab_slot)
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
        # Echo
        self.etest.echo_start_signal.connect(self.echo_start_slot)
        self.etest.echo_query_signal.connect(self.echo_query_slot)
        # Feedback Test
        self.etest.ftest_start_signal.connect(self.ftest_start_slot)


        # Connect scan signal
        self.bias_range_signal.connect(self.scan.bias_ran_change)
        # Scan
        self.scan.close_signal.connect(self.close_scan)
        self.scan.seq_list_signal.connect(ft.partial(self.open_seq_list, 0))
        self.scan.send_signal.connect(self.send_thread)
        self.scan.scan_signal.connect(self.scan_thread)
        self.scan.stop_signal.connect(self.scan_stop)
        self.scan.gain_changed_signal.connect(self.dsp.gain)

        # Sequence
        self.scan.seq_list.close_signal.connect(self.close_seq_list)

        # Deposition
        self.scan.dep.close_signal.connect(self.close_scan)
        self.scan.dep.seq_list_signal.connect(ft.partial(self.open_seq_list, 1))
        self.scan.dep.do_it_signal.connect(self.deposition_thread)
        self.scan.dep.stop_signal.connect(self.scan_stop)

        # Track
        self.scan.track.track_signal.connect(self.track_thread)
        self.scan.track.stop_signal.connect(self.scan_stop)

        # Spectroscopy
        self.scan.spc.close_signal.connect(self.close_scan)
        self.scan.spc.seq_list_signal.connect(ft.partial(self.open_seq_list, 2))
        self.scan.spc.spectroscopy_signal.connect(self.spectroscopy_thread)
        self.scan.spc.stop_signal.connect(self.scan_stop)

        # Do some real stuff
        self.load_config()              # Load DSP settings
        self.dsp.init_dsp(self.initO)   # Try to initial DSP
        # if self.mode_last != 0:
        #     pass
        # !!! self.write_cnfg()         # Update config in real time

    def write_cnfg(self):
        self.cnfgBS.clear()
        # Settings
        self.cnfgBS.setValue("CONFIG/BAUD_VALUE", self.dsp.baudrate)
        self.cnfgBS.setValue("CONFIG/COM_VALUE", self.dsp.port)
        self.cnfgBS.setValue("SETTING/PREAMP_GAIN", self.preamp_gain)
        self.cnfgBS.setValue("SETTING/MODE", self.mode)
        self.cnfgBS.setValue("SETTING/BIAS_DAC", self.bias_dac)

        # DSP
        self.cnfgBS.setValue("DSP/LAST_DAC", self.dsp.lastdac)
        self.cnfgBS.setValue("DSP/DAC_RANGE", self.dsp.dacrange)
        self.cnfgBS.setValue("DSP/ADC_RANGE", self.dsp.adcrange)
        self.cnfgBS.setValue("DSP/LAST_20BIT", self.dsp.last20bit)
        self.cnfgBS.setValue("DSP/LAST_DIGITAL", self.dsp.lastdigital)
        self.cnfgBS.setValue("DSP/LAST_GAIN", self.dsp.lastgain)
        self.cnfgBS.setValue("DSP/OFFSET", self.dsp.offset)
        # Main menu
        self.cnfgBS.setValue("MAIN/HARD_RETRACT", self.hard_retracted)
        self.cnfgBS.setValue("MAIN/PREAMP_GAIN", self.preamp_gain)
        self.cnfgBS.setValue("MAIN/BIAS_DAC", self.bias_dac)
        self.cnfgBS.setValue("MAIN/MODE", self.mode)
        self.cnfgBS.sync()
        
        self.cnfgEX.clear()
        # Bias control
        self.cnfgEX.setValue("BIAS/RAMP_SPEED", self.spinBox_SpeedInput_BiasRamp.value())
        # Current control
        self.cnfgEX.setValue("CURRENT/RAMP_SPEED", self.spinBox_SpeedInput_CurrRamp.value())
        # Tip approach
        self.cnfgEX.setValue("TIP_APPROACH/DELAY", self.tipappr.spinBox_Delay.value())
        self.cnfgEX.setValue("TIP_APPROACH/XIN_STEP_SIZE", self.tipappr.spinBox_Xstep.value())
        self.cnfgEX.setValue("TIP_APPROACH/ACCELERATION", self.tipappr.spinBox_Accel.value())
        self.cnfgEX.setValue("TIP_APPROACH/ZOUTER_STEP_SIZE", self.tipappr.spinBox_Zstep.value())
        self.cnfgEX.setValue("TIP_APPROACH/GIANT_STEP", self.tipappr.spinBox_Giant.value())
        self.cnfgEX.setValue("TIP_APPROACH/MIN_CURRENT", self.tipappr.spinBox_MInCurr.value())
        self.cnfgEX.setValue("TIP_APPROACH/BABY_STEP", self.tipappr.spinBox_Baby.value())
        self.cnfgEX.setValue("TIP_APPROACH/STEP_NUM", self.tipappr.spinBox_StepNum.value())
        # Scan | Send options
        self.cnfgEX.setValue("SEND_OPTIONS/MOVE_DELAY", self.scan.send_options.spinBox_MoveDelay_SendOptions.value())
        self.cnfgEX.setValue("SEND_OPTIONS/STEP_SIZE", self.scan.send_options.spinBox_StepSize_SendOptions.value())
        self.cnfgEX.setValue("SEND_OPTIONS/CRASH", self.scan.send_options.groupBox_Crash_SendOptions.isChecked())
        self.cnfgEX.setValue("SEND_OPTIONS/LIMIT", self.scan.send_options.spinBox_Limit_Crash.value())
        # Scan | Scan options
        self.cnfgEX.setValue("SCAN_OPTIONS/SCAN_ORDER",
                                self.scan.scan_options.radioButton_YFirst_OrderandDirection.isChecked())
        self.cnfgEX.setValue("SCAN_OPTIONS/READ_DIRECTION",
                                self.scan.scan_options.radioButton_ReadForward_OrderandDirection.isChecked())
        self.cnfgEX.setValue("SCAN_OPTIONS/TIP_PROTECTION",
                                self.scan.scan_options.groupBox_Tip_ScanOptions.isChecked())
        self.cnfgEX.setValue("SCAN_OPTIONS/RETRACT", self.scan.scan_options.spinBox_Retract_Tip.value())
        self.cnfgEX.setValue("SCAN_OPTIONS/SCAN_PROTECTION",
                                self.scan.scan_options.groupBox_Scan_ScanOptions.isChecked())
        self.cnfgEX.setValue("SCAN_OPTIONS/ZOUT_LIMIT", self.scan.scan_options.spinBox_ZoutLmit_Scan.value())
        option = 0 if self.scan.scan_options.radioButton_Stop_Scan.isChecked() else (
            1 if self.scan.scan_options.radioButton_Auto0_Scan.isChecked() else 2)
        self.cnfgEX.setValue("SCAN_OPTIONS/SCAN_PROTECTION_OPTION", option)
        self.cnfgEX.setValue("SCAN_OPTIONS/DELAY_OPTION", self.scan.scan_options.radioButton_Fixed_Delay.isChecked())
        self.cnfgEX.setValue("SCAN_OPTIONS/DELAY_MOVE_CONTROL",
                                self.scan.scan_options.spinBox_MoveControl_Delay.value())
        self.cnfgEX.setValue("SCAN_OPTIONS/DELAY_MOVE_OFF", self.scan.scan_options.spinBox_MoveOFF_Delay.value())
        self.cnfgEX.setValue("SCAN_OPTIONS/DELAY_MOVE_ON", self.scan.scan_options.spinBox_MoveON_Delay.value())
        self.cnfgEX.setValue("SCAN_OPTIONS/DELAY_READ_CONTROL",
                                self.scan.scan_options.spinBox_ReadControl_Delay.value())
        self.cnfgEX.setValue("SCAN_OPTIONS/DELAY_READ_OFF", self.scan.scan_options.spinBox_ReadOFF_Delay.value())
        self.cnfgEX.setValue("SCAN_OPTIONS/DELAY_READ_ON", self.scan.scan_options.spinBox_ReadON_Delay.value())
        self.cnfgEX.setValue("SCAN_OPTIONS/DELAY_LINE_CONTROL",
                                self.scan.scan_options.spinBox_LineControl_Delay.value())
        self.cnfgEX.setValue("SCAN_OPTIONS/DELAY_LINE_OFF", self.scan.scan_options.spinBox_LineOFF_Delay.value())
        self.cnfgEX.setValue("SCAN_OPTIONS/DELAY_LINE_ON", self.scan.scan_options.spinBox_LineON_Delay.value())
        self.cnfgEX.setValue("SCAN_OPTIONS/AVERAGE_NUM", self.scan.scan_options.spinBox_Avg.value())
        # !!! Pre-scan
        # Scan | Scan control
        self.cnfgEX.setValue("SCAN_CONTROL/SCAN_SIZE", self.scan.spinBox_ScanSize_ScanControl.value())
        self.cnfgEX.setValue("SCAN_CONTROL/STEP_SIZE", self.scan.spinBox_StepSize_ScanControl.value())
        self.cnfgEX.setValue("SCAN_CONTROL/SEQUENCE", self.scan.scan_seq_list)
        # Scan | Track
        self.cnfgEX.setValue("TRACK/TRACK_SIZE", self.scan.track.spinBox_TrackSize_Track.value())
        self.cnfgEX.setValue("TRACK/STEP_SIZE", self.scan.track.spinBox_StepSize_Track.value())
        self.cnfgEX.setValue("TRACK/READ_CHANNEL", self.scan.track.comboBox_ReadCh_Track.currentIndex())
        self.cnfgEX.setValue("TRACK/AVERAGE", self.scan.track.spinBox_Avg_Track.value())
        self.cnfgEX.setValue("TRACK/SCAN_DELAY", self.scan.track.spinBox_ScanDelay_Track.value())
        self.cnfgEX.setValue("TRACK/STAY_DELAY", self.scan.track.spinBox_StayDelay_Track.value())
        self.cnfgEX.setValue("TRACK/PLANE_FIT", self.scan.track.groupBox_PlaneFit_Track.isChecked())
        self.cnfgEX.setValue("TRACK/PLANE_FIT_X", self.scan.track.spinBox_X_PlaneFit.value())
        self.cnfgEX.setValue("TRACK/PLANE_FIT_Y", self.scan.track.spinBox_Y_PlaneFit.value())
        self.cnfgEX.setValue("TRACK/MODE", self.scan.track.radioButton_Max_PlaneFit.isChecked())
        # Scan | Deposition
        self.cnfgEX.setValue("DEPOSITION/POKE_TIP", self.scan.dep.spinBox_Bias_PokeTip.value())
        self.cnfgEX.setValue("DEPOSITION/DELTA_Z", self.scan.dep.spinBox_DeltaZ_PokeTip.value())
        self.cnfgEX.setValue("DEPOSITION/SEQUENCE", self.scan.dep.groupBox_Seq_Deposition.isChecked())
        self.cnfgEX.setValue("DEPOSITION/SEQUENCE_NAME", self.scan.dep_seq_list)
        self.cnfgEX.setValue("DEPOSITION/READ", self.scan.dep.groupBox_Read_Deposition.isChecked())
        self.cnfgEX.setValue("TRACK/READ_CHANNEL", self.scan.dep.comboBox_Ch_Read.currentIndex())
        self.cnfgEX.setValue("DEPOSITION/AVERAGE_NUM", self.scan.dep.spinBox_AvgNum_Read.value())
        self.cnfgEX.setValue("DEPOSITION/READ_NUM", self.scan.dep.spinBox_Num_Read.value())
        self.cnfgEX.setValue("DEPOSITION/PULSE_READ_CHANNEL", self.scan.dep.comboBox_Ch_Pulse.currentIndex())
        self.cnfgEX.setValue("DEPOSITION/PULSE_WAIT", self.scan.dep.spinBox_Wait_Pulse.value())
        self.cnfgEX.setValue("DEPOSITION/PULSE_AVERAGE_NUM", self.scan.dep.spinBox_AvgNum_Pulse.value())
        self.cnfgEX.setValue("DEPOSITION/PULSE_DELAY", self.scan.dep.spinBox_Delay_Pulse.value())
        self.cnfgEX.setValue("DEPOSITION/N_SAMPLE", self.scan.dep.groupBox_ReadNSample_Deposition.isChecked())
        self.cnfgEX.setValue("DEPOSITION/N_SAMPLE_NUM", self.scan.dep.spinBox_Num_ReadNSample.value())
        self.cnfgEX.setValue("DEPOSITION/READ_UNTIL", self.scan.dep.groupBox_ReadUntil_Deposition.isChecked())
        self.cnfgEX.setValue("DEPOSITION/READ_UNTIL_NUM", self.scan.dep.spinBox_StopNum_ReadUntil.value())
        self.cnfgEX.setValue("DEPOSITION/READ_UNTIL_CHANGE", self.scan.dep.spinBox_Change_ReadUntil.value())
        self.cnfgEX.setValue("DEPOSITION/READ_UNTIL_MODE",
                                self.scan.dep.radioButton_Continuous_ReadUntil.isChecked())
        # Scan | Spectroscopy
        self.cnfgEX.setValue("SPECTROSCOPY/MIN", self.scan.spc.spinBox_Min_General.value())
        self.cnfgEX.setValue("SPECTROSCOPY/MAX", self.scan.spc.spinBox_Max_General.value())
        self.cnfgEX.setValue("SPECTROSCOPY/STEP_SIZE", self.scan.spc.spiBox_StepSize_General.value())
        self.cnfgEX.setValue("SPECTROSCOPY/NUM", self.scan.spc.label_DataNum_General.text())
        self.cnfgEX.setValue("SPECTROSCOPY/PASS", self.scan.spc.spinBox_Pass_General.value())
        self.cnfgEX.setValue("SPECTROSCOPY/RAMP_CHANNEL", self.scan.spc.comboBox_RampCh_General.currentIndex())
        self.cnfgEX.setValue("SPECTROSCOPY/DELTA", self.scan.spc.groupBox_Delta_General.isChecked())
        self.cnfgEX.setValue("SPECTROSCOPY/DELTA_Z", self.scan.spc.spinBox_Delta_Z.value())
        self.cnfgEX.setValue("SPECTROSCOPY/DELTA_BIAS", self.scan.spc.spinBox_Bias_Delta.value())
        self.cnfgEX.setValue("SPECTROSCOPY/MAPPING", self.scan.spc.groupBox_Mapping.isChecked())
        # Scan | Advance option
        self.cnfgEX.setValue("ADVANCE_OPTION/DO", self.scan.spc.adv.spinBox_DoZCorrection_ZDrift.value())
        self.cnfgEX.setValue("ADVANCE_OPTION/Z_CORRECTION", self.scan.spc.adv.groupBox_ZDrift_Correction.isChecked())
        self.cnfgEX.setValue("ADVANCE_OPTION/DELAY", self.scan.spc.adv.spinBox_Delay_ZDrift.value())
        self.cnfgEX.setValue("ADVANCE_OPTION/MATCH", self.scan.spc.adv.checkBox_MatchCurr_ZDrift.isChecked())
        self.cnfgEX.setValue("ADVANCE_OPTION/TRACK", self.scan.spc.adv.checkBox_Tracking_Correction.isChecked())
        self.cnfgEX.setValue("ADVANCE_OPTION/MEASURE_MODE", self.scan.spc.adv.radioButton_I_Measure.isChecked())
        self.cnfgEX.setValue("ADVANCE_OPTION/CH2", self.scan.spc.adv.checkBox_Ch2_Measure.isChecked())
        self.cnfgEX.setValue("ADVANCE_OPTION/CH3", self.scan.spc.adv.checkBox_Ch3_Measure.isChecked())
        self.cnfgEX.setValue("ADVANCE_OPTION/AVERAGE_I", self.scan.spc.adv.spinBox_Avg_I_2.value())
        self.cnfgEX.setValue("ADVANCE_OPTION/AVERAGE_Z", self.scan.spc.adv.spinBox_Avg_Z.value())
        self.cnfgEX.setValue("ADVANCE_OPTION/AVERAGE_CH2", self.scan.spc.adv.spinBox_Avg_CH2.value())
        self.cnfgEX.setValue("ADVANCE_OPTION/AVERAGE_CH3", self.scan.spc.adv.spinBox_Avg_CH3.value())
        self.cnfgEX.setValue("ADVANCE_OPTION/AUTO_SAVE", self.scan.spc.adv.groupBox_AutoSave_AdvOption.isChecked())
        self.cnfgEX.setValue("ADVANCE_OPTION/AUTO_SAVE_EVERY",
                                self.scan.spc.adv.checkBox_SaveEveryPasses_Autosave.isChecked())
        self.cnfgEX.setValue("ADVANCE_OPTION/SEQUENCE", self.scan.spc.adv.groupBox_Seq_AdvOption.isChecked())
        self.cnfgEX.setValue("ADVANCE_OPTION/SEQUENCE_NAME", self.scan.spc_seq_list)
        self.cnfgEX.setValue("ADVANCE_OPTION/MOVE_DELAY", self.scan.spc.adv.spinBox_MoveDelay_Dealy.value())
        self.cnfgEX.setValue("ADVANCE_OPTION/MEASURE_DELAY", self.scan.spc.adv.spinBox_MeasureDelay_Dealy.value())
        self.cnfgEX.setValue("ADVANCE_OPTION/WAIT_DELAY", self.scan.spc.adv.spinBox_Wait_Delay.value())
        scan_direction = 0 if self.scan.spc.adv.radioButton_Forward_Dir.isChecked() else (
            1 if self.scan.spc.adv.radioButton_Bacward_Dir.isChecked() else (
                2 if self.scan.spc.adv.radioButton_AvgBoth_Dir.isChecked() else 3))
        self.cnfgEX.setValue("ADVANCE_OPTION/SCAN_DIRECTION", scan_direction)
        # !!! pre-scan
        # Scan | Point editor
        self.cnfgEX.setValue("SCAN/POINT_EDITOR", self.scan.point_list)
        self.cnfgEX.sync()



    # !!! Load all settings stored in configuration file to dsp module
    def load_config(self):
        self.dsp.baudrate = self.cnfgBS.value("CONFIG/BAUD_VALUE")
        self.dsp.port = self.cnfgBS.value("CONFIG/COM_VALUE")
        # !!! Need to determine if accidently exit and pop out window to let user decide if initialize output
        self.initO = bool(os.stat("initO.log").st_size != 0)  # Get initO from initO.log
        # self.initO = True
        # self.bias_dac = self.cnfg.value()
# =============================================================================
#         # if self.initO:
#         #     pass
#         # else:
#         #     reply = QMessageBox.question(None, "STM", "Initialize output?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
#         #     if reply == QMessageBox.Yes:
#         #         self.initO = True
#         #     else:
#         #         self.mode_last = int(self.cnfg.value("SETTING/MODE"))
#         #         self.preamp_gain = int(self.cnfg.value("SETTING/PREAMP_GAIN"))
# =============================================================================
        self.initO = True       # !!! For temporary use
        # Load params from config
        # DSP
        self.dsp.lastdac = self.cnfgBS.value("DSP/LAST_DAC", type=int)
        self.dsp.dacrange = self.cnfgBS.value("DSP/DAC_RANGE", type=int)
        self.dsp.adcrange = self.cnfgBS.value("DSP/ADC_RANGE", type=int)
        self.dsp.last20bit = self.cnfgBS.value("DSP/LAST_20BIT", type=int)
        self.dsp.lastdigital = self.cnfgBS.value("DSP/LAST_DIGITAL", type=bool)
        self.dsp.lastgain = self.cnfgBS.value("DSP/LAST_GAIN", type=int)
        self.dsp.offset = self.cnfgBS.value("DSP/OFFSET", type=int)
        # Main menu
        self.hard_retracted = self.cnfgBS.value("MAIN/HARD_RETRACT", type=bool)
        self.preamp_gain = self.cnfgBS.value("MAIN/PREAMP_GAIN", type=int)
        self.bias_dac = self.cnfgBS.value("MAIN/BIAS_DAC", type=bool)
        self.mode = self.cnfgBS.value("MAIN/MODE", type=int)
        # Bias control
        self.spinBox_SpeedInput_BiasRamp.setValue(self.cnfgEX.value("BIAS/RAMP_SPEED", type=int))
        # Current control
        self.spinBox_SpeedInput_CurrRamp.setValue(self.cnfgEX.value("CURRENT/RAMP_SPEED", type=int))
        # Tip approach
        self.tipappr.spinBox_Delay.setValue(self.cnfgEX.value("TIP_APPROACH/DELAY", type=int))
        self.tipappr.spinBox_Xstep.setValue(self.cnfgEX.value("TIP_APPROACH/XIN_STEP_SIZE", type=float))
        self.tipappr.spinBox_Accel.setValue(self.cnfgEX.value("TIP_APPROACH/ACCELERATION", type=float))
        self.tipappr.spinBox_Zstep.setValue(self.cnfgEX.value("TIP_APPROACH/ZOUTER_STEP_SIZE", type=float))
        self.tipappr.spinBox_Giant.setValue(self.cnfgEX.value("TIP_APPROACH/GIANT_STEP", type=int))
        self.tipappr.spinBox_MInCurr.setValue(self.cnfgEX.value("TIP_APPROACH/MIN_CURRENT", type=float))
        self.tipappr.spinBox_Baby.setValue(self.cnfgEX.value("TIP_APPROACH/BABY_STEP", type=int))
        self.tipappr.spinBox_StepNum.setValue(self.cnfgEX.value("TIP_APPROACH/STEP_NUM", type=int))
        # Scan | Send options
        self.scan.send_options.spinBox_MoveDelay_SendOptions.setValue(self.cnfgEX.value("SEND_OPTIONS/MOVE_DELAY", type=int))
        self.scan.send_options.spinBox_StepSize_SendOptions.setValue(self.cnfgEX.value("SEND_OPTIONS/STEP_SIZE", type=int))
        self.scan.send_options.groupBox_Crash_SendOptions.setChecked(self.cnfgEX.value("SEND_OPTIONS/CRASH", type=bool))
        self.scan.send_options.spinBox_Limit_Crash.setValue(self.cnfgEX.value("SEND_OPTIONS/LIMIT", type=float))
        # Scan | Scan options
        self.scan.scan_options.radioButton_YFirst_OrderandDirection.setChecked(
            self.cnfgEX.value("SCAN_OPTIONS/SCAN_ORDER", type=bool))
        self.scan.scan_options.radioButton_ReadForward_OrderandDirection.setChecked(
            self.cnfgEX.value("SCAN_OPTIONS/READ_DIRECTION", type=bool))
        self.scan.scan_options.groupBox_Tip_ScanOptions.setChecked(self.cnfgEX.value("SCAN_OPTIONS/TIP_PROTECTION", type=bool))
        self.scan.scan_options.spinBox_Retract_Tip.setValue(self.cnfgEX.value("SCAN_OPTIONS/RETRACT", type=int))
        self.scan.scan_options.groupBox_Scan_ScanOptions.setChecked(
            self.cnfgEX.value("SCAN_OPTIONS/SCAN_PROTECTION", type=bool))
        self.scan.scan_options.spinBox_ZoutLmit_Scan.setValue(self.cnfgEX.value("SCAN_OPTIONS/ZOUT_LIMIT", type=float))
        if self.cnfgEX.value("SCAN_OPTIONS/SCAN_PROTECTION_OPTION") == 0:
            self.scan.scan_options.radioButton_Stop_Scan.setChecked(True)
        elif self.cnfgEX.value("SCAN_OPTIONS/SCAN_PROTECTION_OPTION") == 1:
            self.scan.scan_options.radioButton_Auto0_Scan.setChecked(True)
        elif self.cnfgEX.value("SCAN_OPTIONS/SCAN_PROTECTION_OPTION") == 2:
            self.scan.scan_options.radioButton_AutoPre_Scan.setChecked(True)
        self.scan.scan_options.radioButton_Fixed_Delay.setChecked(self.cnfgEX.value("SCAN_OPTIONS/DELAY_OPTION", type=bool))
        self.scan.scan_options.spinBox_MoveControl_Delay.setValue(
            self.cnfgEX.value("SCAN_OPTIONS/DELAY_MOVE_CONTROL", type=int))
        self.scan.scan_options.spinBox_MoveOFF_Delay.setValue(self.cnfgEX.value("SCAN_OPTIONS/DELAY_MOVE_OFF", type=int))
        self.scan.scan_options.spinBox_MoveON_Delay.setValue(self.cnfgEX.value("SCAN_OPTIONS/DELAY_MOVE_ON", type=int))
        self.scan.scan_options.spinBox_ReadControl_Delay.setValue(
            self.cnfgEX.value("SCAN_OPTIONS/DELAY_READ_CONTROL", type=int))
        self.scan.scan_options.spinBox_ReadOFF_Delay.setValue(self.cnfgEX.value("SCAN_OPTIONS/DELAY_READ_OFF", type=int))
        self.scan.scan_options.spinBox_ReadON_Delay.setValue(self.cnfgEX.value("SCAN_OPTIONS/DELAY_READ_ON", type=int))
        self.scan.scan_options.spinBox_LineControl_Delay.setValue(
            self.cnfgEX.value("SCAN_OPTIONS/DELAY_LINE_CONTROL", type=int))
        self.scan.scan_options.spinBox_LineOFF_Delay.setValue(self.cnfgEX.value("SCAN_OPTIONS/DELAY_LINE_OFF", type=int))
        self.scan.scan_options.spinBox_LineON_Delay.setValue(self.cnfgEX.value("SCAN_OPTIONS/DELAY_LINE_ON", type=int))
        self.scan.scan_options.spinBox_Avg.setValue(self.cnfgEX.value("SCAN_OPTIONS/AVERAGE_NUM", type=int))
        # !!! Pre-scan
        # Scan | Scan control
        self.scan.spinBox_ScanSize_ScanControl.setValue(self.cnfgEX.value("SCAN_CONTROL/SCAN_SIZE", type=int))
        self.scan.spinBox_StepSize_ScanControl.setValue(self.cnfgEX.value("SCAN_CONTROL/STEP_SIZE", type=int))
        self.scan.scan_seq_list = self.cnfgEX.value("SCAN_CONTROL/SEQUENCE")
        # Scan | Track
        self.scan.track.spinBox_TrackSize_Track.setValue(self.cnfgEX.value("TRACK/TRACK_SIZE", type=int))
        self.scan.track.spinBox_StepSize_Track.setValue(self.cnfgEX.value("TRACK/STEP_SIZE", type=int))
        self.scan.track.comboBox_ReadCh_Track.setCurrentIndex(self.cnfgEX.value("TRACK/READ_CHANNEL", type=int))
        self.scan.track.spinBox_Avg_Track.setValue(self.cnfgEX.value("TRACK/AVERAGE", type=int))
        self.scan.track.spinBox_ScanDelay_Track.setValue(self.cnfgEX.value("TRACK/SCAN_DELAY", type=int))
        self.scan.track.spinBox_StayDelay_Track.setValue(self.cnfgEX.value("TRACK/STAY_DELAY", type=int))
        self.scan.track.groupBox_PlaneFit_Track.setChecked(self.cnfgEX.value("TRACK/PLANE_FIT", type=bool))
        self.scan.track.spinBox_X_PlaneFit.setValue(self.cnfgEX.value("TRACK/PLANE_FIT_X", type=float))
        self.scan.track.spinBox_Y_PlaneFit.setValue(self.cnfgEX.value("TRACK/PLANE_FIT_Y", type=float))
        self.scan.track.radioButton_Max_PlaneFit.setChecked(self.cnfgEX.value("TRACK/MODE", type=bool))
        # Scan | Deposition
        self.scan.dep.spinBox_Bias_PokeTip.setValue(self.cnfgEX.value("DEPOSITION/POKE_TIP", type=float))
        self.scan.dep.spinBox_DeltaZ_PokeTip.setValue(self.cnfgEX.value("DEPOSITION/DELTA_Z", type=int))
        self.scan.dep.groupBox_Seq_Deposition.setChecked(self.cnfgEX.value("DEPOSITION/SEQUENCE", type=bool))
        self.scan.dep_seq_list = self.cnfgEX.value("DEPOSITION/SEQUENCE_NAME")
        self.scan.dep.groupBox_Read_Deposition.setChecked(self.cnfgEX.value("DEPOSITION/READ", type=bool))
        self.scan.dep.comboBox_Ch_Read.setCurrentIndex(self.cnfgEX.value("TRACK/READ_CHANNEL", type=int))
        self.scan.dep.spinBox_AvgNum_Read.setValue(self.cnfgEX.value("DEPOSITION/AVERAGE_NUM", type=int))
        self.scan.dep.spinBox_Num_Read.setValue(self.cnfgEX.value("DEPOSITION/READ_NUM", type=int))
        self.scan.dep.comboBox_Ch_Pulse.setCurrentIndex(self.cnfgEX.value("DEPOSITION/PULSE_READ_CHANNEL", type=int))
        self.scan.dep.spinBox_Wait_Pulse.setValue(self.cnfgEX.value("DEPOSITION/PULSE_WAIT", type=int))
        self.scan.dep.spinBox_AvgNum_Pulse.setValue(self.cnfgEX.value("DEPOSITION/PULSE_AVERAGE_NUM", type=int))
        self.scan.dep.spinBox_Delay_Pulse.setValue(self.cnfgEX.value("DEPOSITION/PULSE_DELAY", type=int))
        self.scan.dep.groupBox_ReadNSample_Deposition.setChecked(self.cnfgEX.value("DEPOSITION/N_SAMPLE", type=bool))
        self.scan.dep.spinBox_Num_ReadNSample.setValue(self.cnfgEX.value("DEPOSITION/N_SAMPLE_NUM", type=int))
        self.scan.dep.groupBox_ReadUntil_Deposition.setChecked(self.cnfgEX.value("DEPOSITION/READ_UNTIL", type=bool))
        self.scan.dep.spinBox_StopNum_ReadUntil.setValue(self.cnfgEX.value("DEPOSITION/READ_UNTIL_NUM", type=int))
        self.scan.dep.spinBox_Change_ReadUntil.setValue(self.cnfgEX.value("DEPOSITION/READ_UNTIL_CHANGE", type=float))
        self.scan.dep.radioButton_Continuous_ReadUntil.setChecked(self.cnfgEX.value("DEPOSITION/READ_UNTIL_MODE", type=bool))
        # Scan | Spectroscopy
        self.scan.spc.spinBox_Min_General.setValue(self.cnfgEX.value("SPECTROSCOPY/MIN", type=float))
        self.scan.spc.spinBox_Max_General.setValue(self.cnfgEX.value("SPECTROSCOPY/MAX", type=float))
        self.scan.spc.spiBox_StepSize_General.setValue(self.cnfgEX.value("SPECTROSCOPY/STEP_SIZE", type=float))
        self.scan.spc.label_DataNum_General.setText(self.cnfgEX.value("SPECTROSCOPY/NUM"))
        self.scan.spc.spinBox_Pass_General.setValue(self.cnfgEX.value("SPECTROSCOPY/PASS", type=int))
        self.scan.spc.comboBox_RampCh_General.setCurrentIndex(self.cnfgEX.value("SPECTROSCOPY/RAMP_CHANNEL", type=int))
        self.scan.spc.groupBox_Delta_General.setChecked(self.cnfgEX.value("SPECTROSCOPY/DELTA", type=bool))
        self.scan.spc.spinBox_Delta_Z.setValue(self.cnfgEX.value("SPECTROSCOPY/DELTA_Z", type=int))
        self.scan.spc.spinBox_Bias_Delta.setValue(self.cnfgEX.value("SPECTROSCOPY/DELTA_BIAS", type=float))
        self.scan.spc.groupBox_Mapping.setChecked(self.cnfgEX.value("SPECTROSCOPY/MAPPING", type=bool))
        # Scan | Advance option
        self.scan.spc.adv.spinBox_DoZCorrection_ZDrift.setValue(self.cnfgEX.value("ADVANCE_OPTION/DO", type=int))
        self.scan.spc.adv.groupBox_ZDrift_Correction.setChecked(self.cnfgEX.value("ADVANCE_OPTION/Z_CORRECTION", type=bool))
        self.scan.spc.adv.spinBox_Delay_ZDrift.setValue(self.cnfgEX.value("ADVANCE_OPTION/DELAY", type=int))
        self.scan.spc.adv.checkBox_MatchCurr_ZDrift.setChecked(self.cnfgEX.value("ADVANCE_OPTION/MATCH", type=bool))
        self.scan.spc.adv.checkBox_Tracking_Correction.setChecked(self.cnfgEX.value("ADVANCE_OPTION/TRACK", type=bool))
        self.scan.spc.adv.radioButton_I_Measure.setChecked(self.cnfgEX.value("ADVANCE_OPTION/MEASURE_MODE", type=bool))
        self.scan.spc.adv.checkBox_Ch2_Measure.setChecked(self.cnfgEX.value("ADVANCE_OPTION/CH2", type=bool))
        self.scan.spc.adv.checkBox_Ch3_Measure.setChecked(self.cnfgEX.value("ADVANCE_OPTION/CH3", type=bool))
        self.scan.spc.adv.spinBox_Avg_I_2.setValue(self.cnfgEX.value("ADVANCE_OPTION/AVERAGE_I", type=int))
        self.scan.spc.adv.spinBox_Avg_Z.setValue(self.cnfgEX.value("ADVANCE_OPTION/AVERAGE_Z", type=int))
        self.scan.spc.adv.spinBox_Avg_CH2.setValue(self.cnfgEX.value("ADVANCE_OPTION/AVERAGE_CH2", type=int))
        self.scan.spc.adv.spinBox_Avg_CH3.setValue(self.cnfgEX.value("ADVANCE_OPTION/AVERAGE_CH3", type=int))
        self.scan.spc.adv.groupBox_AutoSave_AdvOption.setChecked(self.cnfgEX.value("ADVANCE_OPTION/AUTO_SAVE", type=bool))
        self.scan.spc.adv.checkBox_SaveEveryPasses_Autosave.setChecked(
            self.cnfgEX.value("ADVANCE_OPTION/AUTO_SAVE_EVERY", type=bool))
        self.scan.spc.adv.groupBox_Seq_AdvOption.setChecked(self.cnfgEX.value("ADVANCE_OPTION/SEQUENCE", type=bool))
        self.scan.spc_seq_list = self.cnfgEX.value("ADVANCE_OPTION/SEQUENCE_NAME")
        self.scan.spc.adv.spinBox_MoveDelay_Dealy.setValue(self.cnfgEX.value("ADVANCE_OPTION/MOVE_DELAY", type=int))
        self.scan.spc.adv.spinBox_MeasureDelay_Dealy.setValue(self.cnfgEX.value("ADVANCE_OPTION/MEASURE_DELAY", type=int))
        self.scan.spc.adv.spinBox_Wait_Delay.setValue(self.cnfgEX.value("ADVANCE_OPTION/WAIT_DELAY", type=int))
        if self.cnfgEX.value("ADVANCE_OPTION/SCAN_DIRECTION") == 0:
            self.scan.spc.adv.radioButton_Forward_Dir.setChecked(True)
        elif self.cnfgEX.value("ADVANCE_OPTION/SCAN_DIRECTION") == 1:
            self.scan.spc.adv.radioButton_Bacward_Dir.setChecked(True)
        elif self.cnfgEX.value("ADVANCE_OPTION/SCAN_DIRECTION") == 2:
            self.scan.spc.adv.radioButton_AvgBoth_Dir.setChecked(True)
        elif self.cnfgEX.value("ADVANCE_OPTION/SCAN_DIRECTION") == 3:
            self.scan.spc.adv.radioButton_Both_Dir.setChecked(True)
        # !!! pre-scan
        # Scan | Point editor
        self.scan.point_list = self.cnfgEX.value("SCAN/POINT_EDITOR")


    # DSP intial succeed slot
    def dsp_succeed_slot(self, succeed):
        # Pop out succees message
        text = "Successfully found DSP" if succeed else "No DSP found"
        QMessageBox.information(None, "STM", text, QMessageBox.Ok)
        
        # Set up view
        self.versionLabel.setText(self.dsp.ver)                        # Change version label
        self.setting.init_setting(self.dsp.succeed, self.dsp.port, \
                                  self.dsp.baudrate, self.dsp.offset)  # Reinital setting view if succeed
            
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
            self.ramp_update(channel, current)              # Update etest ramp if in etest mode

    # DSP ramp measure update signal:
    def dsp_rampMeasure_slot(self, rdata):
        if self.mode == 1:
            self.ramp_read_update(rdata)            # Update etest ramp read if in etest mode
        elif self.mode == 3:
            self.scan.spc.update_spc(rdata)         # Update spectroscopy is in scan mode
            
    # DSP OSC_C update signal
    def dsp_oscc_slot(self, rdata):
        if self.mode == 1:
            self.osci_update(rdata)                 # Update etest continuous oscilloscope if in etest mode
        elif self.mode == 3:
            self.scan.dep.update_C(rdata)           # Update deposition if in scan mode

    # Close dsp serial port before exit application
    def closeEvent(self, event):
        if self.mode == 0:          # Can exit software, when in fundamental mode
            self.Bias.close()       # Close bias dock
            self.Zcontrol.close()   # Close Z controller dock
            self.Current.close()    # Close Current dock
            self.dsp.close()        # Terminate DSP serial communication
            self.write_cnfg()       # Write configuration file
            event.accept()          # Accept close event
        else:
            self.msg("Close top window first!")     # Pop out window to remind close the tip window
            event.ignore()          # Reject close event

    # open window: tip approach, setting, etest
    def open_window(self, index):
        if (index == -1) and (self.mode == 0):          # Open setting
            self.mode = -1                                      # Change the mode variable
            self.hide_all_dock()                                # Hide all dock windows
            self.setting.init_setting(self.dsp.succeed, self.dsp.port, self.dsp.baudrate, self.dsp.offset)  # Init setting view
            self.setting.show()                                 # Show setting window
            
        elif (index == 1) and (self.mode == 0):         # Open etest
            self.mode = 1                                       # Change the mode variable
            self.hide_all_dock()                                # Hide all dock windows
            self.etest.init_etest(self.dsp.succeed, self.dsp.dacrange, self.dsp.adcrange, self.dsp.lastdigital, \
                                  self.dsp.lastgain, self.dsp.lastdac, self.dsp.last20bit)  # Init etest view
            self.etest.show()                                   # Show electronics test window
            
        elif (index == 2) and (self.mode == 0):         # Open tip appraoch
            self.mode = 2                                       # Change the mode variable
            self.enable_menubar(2)                              # Disable menubar to prevent softeware enter other mode
            self.setup_tipappr(True)                            # Prepare all electronics to tip approach mode
            self.tipappr.init_tipAppr(self.dsp.succeed, self.dsp.lastdigital)     # Init tip approach view
            self.tipappr.show()                                 # Show tip approach window

    # Close serial window
    # Used for closing setting, etest and tip approach
    def closeWindow(self):
        self.mode = 0
        self.enable_menubar(0)

    # Open scan window
    # Open scan sub window
    def open_scan(self, index):
        if (index == 0) and (self.mode == 0):       # Open scan
            self.mode = 3
            self.enter_scan()                                               # Enter scan mode
            self.scan.init_scan(self.dsp, self.bias_dac, self.preamp_gain)  # Init scan view
            self.scan.show()
            self.enable_menubar(3)                                          # Disable menubar to prevent softeware enter other mode
            self.enable_scan_menu(0)
        elif (index == 1) and (self.mode == 3):     # Open spectroscopy
            self.scan.mode = 1
            self.scan.point_list = [[self.dsp.lastdac[0], self.dsp.lastdac[15]]]    # Init point list
            self.scan.spc.groupBox_Mapping.setChecked(False)                        # Uncheck mapping groupbox
            # Close and open track based on spectroscopy advance option
            if self.scan.spc.adv.checkBox_Tracking_Correction.isChecked():
                self.scan.open_track(2)
            else:
                self.scan.open_track(0)
            self.scan.spc.show()
            self.enable_scan_menu(1)
        elif (index == 2) and (self.mode == 3):     # Open deposition
            self.scan.mode = 2
            self.scan.dep.show()
            self.enable_scan_menu(2)
        elif (index == -1) and (self.mode == 3):    # Open track
            self.scan.track.show()
        else:
            self.msg("Open Scan window first!")
    
    # Close scan window
    # Close scan sub window
    def close_scan(self):
        if self.scan.mode == 0:
            self.exit_scan()
            self.closeWindow()
            self.scan.track.hide()          # Also close track window
        else:
            self.scan.mode = 0
            self.scan.open_track(0)          # In case of close spectroscopy with open track
            self.enable_scan_menu(0)        # Enable all scan action
    
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

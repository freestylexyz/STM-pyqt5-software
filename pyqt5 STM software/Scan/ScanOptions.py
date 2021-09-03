# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/24 17:40:23
@Author   : milier00
@FileName : ScanOptions.py
"""
import sys
sys.path.append("../ui/")
sys.path.append("../Model/")
from PyQt5.QtWidgets import QApplication, QWidget, QButtonGroup
from ScanOptions_ui import Ui_ScanOptions
import conversion as cnv


class myScanOptions(QWidget, Ui_ScanOptions):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        # Conversions
        self.spinBox_Retract_Tip.editingFinished.connect(lambda: self.scrollBar_Retract_tip.setValue(self.spinBox_Retract_Tip.value()))
        self.scrollBar_Retract_tip.valueChanged.connect(self.spinBox_Retract_Tip.setValue)
        self.spinBox_Advance_Feedabck.editingFinished.connect(lambda: self.scrollBar_Advance_Feedabck.setValue(self.spinBox_Advance_Feedabck.value()))
        self.scrollBar_Advance_Feedabck.valueChanged.connect(self.spinBox_Advance_Feedabck.setValue)

        self.delay_group = QButtonGroup()
        self.delay_group.addButton(self.radioButton_Fixed_Delay, 0)
        self.delay_group.addButton(self.radioButton_Variable_Delay, 1)
        self.delay_group.buttonToggled[int, bool].connect(self.delay_changed)

    # delay radio button slot ( 0: fixed; 1: variable )
    def delay_changed(self, index, status):
        if status:
            if index == 0:
                self.spinBox_MoveControl_Delay.setEnabled(True)
                self.spinBox_ReadControl_Delay.setEnabled(True)
                self.spinBox_LineControl_Delay.setEnabled(True)
                self.spinBox_MoveOFF_Delay.setEnabled(False)
                self.spinBox_ReadOFF_Delay.setEnabled(False)
                self.spinBox_LineOFF_Delay.setEnabled(False)
                self.spinBox_MoveON_Delay.setEnabled(False)
                self.spinBox_ReadON_Delay.setEnabled(False)
                self.spinBox_LineON_Delay.setEnabled(False)
            else:
                self.spinBox_MoveControl_Delay.setEnabled(False)
                self.spinBox_ReadControl_Delay.setEnabled(False)
                self.spinBox_LineControl_Delay.setEnabled(False)
                self.spinBox_MoveOFF_Delay.setEnabled(True)
                self.spinBox_ReadOFF_Delay.setEnabled(True)
                self.spinBox_LineOFF_Delay.setEnabled(True)
                self.spinBox_MoveON_Delay.setEnabled(True)
                self.spinBox_ReadON_Delay.setEnabled(True)
                self.spinBox_LineON_Delay.setEnabled(True)

    # Configure scan direction
    def configure_scan(self):
        # channel_x is point channel, channel_y is line channel
        if self.radioButton_YFirst_OrderandDirection.isChecked():
            channel_x, channel_y = 0x1f, 0x10
        else:
            channel_x, channel_y = 0x10, 0x1f
        # Configure point channel ramping direction
        dir_x = self.radioButton_ReadForward_OrderandDirection.isChecked()
        return channel_x, channel_y, dir_x
        
    # Configure delay based on feedback
    def configure_delay(self, feedback):
        if self.radioButton_Fixed_Delay.isChecked():    # Fixed delay
            move_delay = self.spinBox_MoveControl_Delay.value()
            measure_delay = self.spinBox_ReadControl_Delay.value()
            line_delay = self.spinBox_LineControl_Delay.value()
        else:
            if feedback:                                # Feedback on default
                move_delay = self.spinBox_MoveON_Delay.value()
                measure_delay = self.spinBox_ReadON_Delay.value()
                line_delay = self.spinBox_LineON_Delay.value()
            else:                                       # Feedback off default
                move_delay = self.spinBox_MoveOFF_Delay.value()
                measure_delay = self.spinBox_ReadOFF_Delay.value()
                line_delay = self.spinBox_LineOFF_Delay.value()
        return move_delay, measure_delay, line_delay
    
    # Configure scan protection
    def configure_scan_protect(self):
        if self.groupBox_Scan_ScanOptions.isChecked():
            if self.radioButton_Stop_Scan.isChecked():
                scan_protect_flag = 1       # Stop scan
            elif self.radioButton_Auto0_Scan.isChecked():
                scan_protect_flag = 2       # Adjust Z feedback to zero
            elif self.radioButton_AutoPre_Scan.isChecked():
                scan_protect_flag = 3       # Adjust Z feedback to the previous middle point
        else:
            scan_protect_flag = 0           # Do nothing
        limit = cnv.vb(self.spinBox_ZoutLmit_Scan.value(), 'a') - cnv.vb(0.0, 'a')  # Limit for scan protection
        return scan_protect_flag, limit
        
    # Configure tip protection
    def configure_tip_protect(self, seq):
        if seq.feedback and (seq.ditherB or seq.ditherZ):   # Feedback is on and either dither is on
            tip_protection = False                                          # Can not match current, disable tip protection
        else:                                               # Other circumstance
            tip_protection = self.groupBox_Tip_ScanOptions.isChecked()      # Tip protection flag
        tip_protect_data = self.scrollBar_Retract_tip.value()               # Retract Z offset fine bits
        return tip_protection, tip_protect_data
            
    # Configure before scan procedures
    def configure_prescan(self, seq_feedback, dsp):
        # Determine enable flag
        prescan = dsp.lastdigital[2] and (not seq_feedback)                             # Enable only when current feedback is on but sequence feedback is off
        prescan = self.groupBox_Feedback_ScanOptions.isChecked() and (not prescan)      # Enable by use 
        
        match_curr = prescan and (not(dsp.lastdigital[0] or dsp.lastdigital[1]))        # Enable match current when both dither are off and prescan is activated
        match_curr = self.checkBox_MatchCurr_Feedback.isChecked() and match_curr        # Enable match current by user
        advance_bit = self.scrollBar_Advance_Feedabck.value() if prescan else 0         # Enable advance bit only when prescan is activated
        return match_curr, advance_bit



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myScanOptions()
    window.show()
    sys.exit(app.exec_())
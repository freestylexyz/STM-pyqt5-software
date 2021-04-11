# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/24 17:30:19
@Author   : milier00
@FileName : AdvanceOption.py
"""
import sys
sys.path.append("../ui/")
from PyQt5.QtWidgets import QApplication, QWidget
from AdvanceOption_ui import Ui_AdvanceOption


class myAdvanceOption(QWidget, Ui_AdvanceOption):
    def __init__(self):
        super().__init__()
        self.setupUi(self)

    # Configure pre-scan for sequence
    def configure_prescan(self):
        feedback = self.radioButton_FeedbackON.isChecked()          # Default sequence, feedback
        ditherB = self.radioButton_BiasON_AdvOption.isChecked()     # Default sequence, bias dither
        ditherZ = self.radioButton_ZON_AdvOption.isChecked()        # Default sequence, Z dither
        return feedback, ditherB, ditherZ
    
    # Configure delays
    def configure_delay(self):
        move_delay = self.spinBox_MoveDelay_Dealy.value()           # Delay waited before moving 1 step
        measure_delay = self.spinBox_MeasureDelay_Dealy.value()     # Delay waited before read sequence
        return move_delay, measure_delay
        
    # Configure command list and data list for sequence
    def configure_measure(self):
        command_list = [0]                                      # First step is wait
        data_list = [self.spinBox_Wait_Delay.value() * 1000]    # Wait delay
        if self.radioButton_I_Measure.isChecked:
            command_list += [0xdc]                              # Preamp: 0xc0 + 4 * 7
            data_list += [self.spinBox_Avg_I.value()]           # Preamp average number
        if self.radioButton_Z_Measure.isChecked:
            command_list += [0xd8]                              # Zout: 0xc0 + 4 * 6
            data_list += [self.spinBox_Avg_Z.value()]           # Zout average number
        if self.checkBox_Ch2_Measure.isChecked:
            command_list += [0xc8]                              # ADC2: 0xc0 + 4 * 2
            data_list += [self.spinBox_Avg_CH2.value()]         # ADC2 average number
        if self.checkBox_Ch3_Measure.isChecked:
            command_list += [0xcc]                              # ADC3: 0xc0 + 4 * 3
            data_list += [self.spinBox_Avg_CH3.value()]         # ADC3 average number
            
        if self.groupBox_Seq_AdvOption.isChecked():
            return command_list, data_list                      # Return command list and data list
        else:
            return [], []                                       # Return empty list to indicate not using sequence
    
    # Configure correction options
    def configure_correction(self, feedback):
        corr_pass_num = self.spinBox_DoZCorrection_ZDrift.value()
        z_flag = self.groupBox_ZDrift_Correction.isChecked() and (not feedback) # No need to do z drift correction if feedback in on
        match_flag = self.checkBox_MatchCurr_ZDrift.isChecked() and z_flag      # No to do current matching if not doing Z drift correction
        feedback_delay = self.spinBox_Delay_ZDrift.value() if z_flag else 0     # Feedback delay 0ms if not doing Z drift correction
        track_flag = self.checkBox_Tracking_Correction.isChecked()              # XY drift correction (tracking)
        
        return corr_pass_num, z_flag, match_flag, feedback_delay, track_flag
        
    # Configure scan options
    def configure_scan(self):
        forward = True      # If measuring forward pass
        backward = True     # If measuring backward pass
        average = False     # If averaging forward and backward pass
        if self.radioButton_Forward_Dir.isChecked():
            backward = False
        elif self.radioButton_Bacward_Dir.isChecked():
            forward = False
        elif self.radioButton_AvgBoth_Dir.isChecked():
            average = True
        return forward, backward, average
    
    # Configure rescan options
    def configure_rescan(self):
        # Make rescan passes super large if not doing rescan, so that it never triggers rescan
        return self.spinBox_Pass_Rescan.value() if self.groupBox_Rescan.isChecked() else 0xffffffff
    


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myAdvanceOption()
    window.show()
    sys.exit(app.exec_())
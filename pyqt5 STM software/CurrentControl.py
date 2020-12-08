# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 16:04:07 2020

@author: yaoji
"""

import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QMessageBox
from MainMenu import myMainMenu
import conversion as cnv
import threading
import math

# myCurrentControl - This class handles current dock control
class myCurrentControl(myMainMenu):
    # Init current dock
    def init_current_dock(self):
        self.init_current()     # Init current dock view
        
        # Connect signals and slots
        self.radioButton_8_Gain.toggled.connect(self.current_gain)  # Preamp gain 8 radio button
        self.radioButton_9_Gain.toggled.connect(self.current_gain)  # Preamp gain 9 radio button
        self.radioButton_10_Gain.toggled.connect(self.current_gain) # Preamp gain 10 radion button
        
        self.spinBox_Input_Setpoint.editingFinished.connect(self.current_value) # Setpoint main spin box
        self.scrollBar_Input_Setpoint.valueChanged.connect(self.current_out)    # Setpoint scroll bar
        
        # Ramp button 1 to 4
        self.pushButton_Rampto1_CurrRamp.clicked.connect(self.current_ramp_1)
        self.pushButton_Rampto2_CurrRamp.clicked.connect(self.current_ramp_2)
        self.pushButton_Rampto3_CurrRamp.clicked.connect(self.current_ramp_3)
        self.pushButton_Rampto4_CurrRamp.clicked.connect(self.current_ramp_4)
        self.pushButton_StopRamp_CurrRamp.clicked.connect(self.current_stop)    # Stop ramp button
    
    # Show current dock
    def current_show(self):
        self.init_current() # Reinit current dock view
        self.Current.show() # Show current dock
    
    # Update current indication
    def current_update(self, bits):
        value = self.b2v(bits, self.preamp_gain)
        self.spinBox_Input_Current.setValue(value)  # Update main spinbox only, scrollbar will not follow
    
    def init_current(self):
        self.enable_current_serial(self.dsp.succeed)    # Enable serial related modules
        self.current_set_radio()                        # Set up preamp gain radio button
        self.current_spinbox_range()                    # Set up all spin boxes range
        
        bits = self.dsp.lastdac[5]                      # Fetch current I ser bits
        self.scrollBar_Input_Setpoint.setValue(bits)    # Set up scroll bar
        self.spinBox_Input_Setpoint.setValue(self.b2i(bits, self.preamp_gain))  # Set up main spin box 

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
        else:
            multiplier = 1000.0
            
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
        else:
            multiplier = 1000.0
            
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
    
    # Set radio button for preamp gain
    def current_set_radio(self):
        if self.preamp_gain == 8:
            self.radioButton_8_Gain.setChecked(True)    # Set gain 8 radio button if gain 8
        elif self.preamp_gain == 9:
            self.radioButton_9_Gain.setChecked(True)    # Set gain 9 radio button if gain 9
        elif self.preamp_gain == 10:
            self.radioButton_10_Gain.setChecked(True)   # Set gain 10 radio button if gain 10
    
    # Pream gain execution
    def current_gain_excu(self, gain, value):
        self.enable_mode_serial(False)                  # Disable all serial component in current window
        self.idling = False                             # Toggle dock idling flag
        self.preamp_gain = gain                         # Change preamp gain flag
        self.current_spinbox_range()                    # Set all spin boxes range
        bits = self.i2b(value, self.preamp_gain)        # Obtain target bits after changing gain
        self.dsp.rampTo(0x15, bits, 2, 1000, 0, False)  # Ramp to target
        self.scrollBar_Input_Setpoint.setValue(bits)    # Set scroll bar to current status
        self.idling = True                              # Toggle dock idling flag
        self.enable_mode_serial(True)                   # Enable all serial component in current window
    
    # Preamp gain radio button slot
    def current_gain(self):
        # Determin gain based on radio button check status
        if self.radioButton_8_Gain.isChecked():
            gain = 8
        elif self.radioButton_9_Gain.isChecked():
            gain = 9
        elif self.radioButton_10_Gain.isChecked():
            gain = 10
        
        # If preamp gain is changed
        if gain != self.preamp_gain:
            minimum = self.b2i(0xffff, gain)                # Target gain setpoint minimum
            maximum = self.b2i(0x0, gain)                   # Target gain setpoint maximum
            value = self.spinBox_Input_Setpoint.value()     # Current setpoint
            if (value > maximum) and (value < minimum):     # If current setpoint out of target range
                self.current_out_of_range_message()             # Out of range message
                self.current_set_radio()                        # Set the radio button back
            elif self.dsp.lastdigital[3]:                   # If feedack is on
                self.current_feedback_off_message()             # Remind turn the feedback off
                self.current_set_radio()                        # Set the radio button back
            else:                                           # Everythin is good
                threading.Thread(target = (lambda: self.current_gain_excu(gain, value))).start()    # Execute with thread

                
    # Setpoint spinBox slot
    def current_value(self):
        value = self.spinBox_Input_Setpoint.value()     # Current setpoint
        self.scrollBar_Input_Setpoint.setValue(self.i2b(value, self.preamp_gain))   # Change I set by changing scrollbar value
    
    # Setpoint scroll bar slot
    def current_out(self, bits):
        if bits != self.dsp.lastdac[5]:     # It it is a real change
            self.dsp.bit20_W(0x15, bits)    # Direct output I set
            self.spinBox_Input_Setpoint.setValue(self.b2i(bits, self.preamp_gain))  # Set main spin box for indication
    
    # Current stop ramp button slot
    def current_stop(self):
        self.dsp.stop = True    # Toggle DSP stop flag
        self.pushButton_StopRamp_CurrRamp.setEnabled(False) # Prevent multiple toggle
    
        
    # Setpoint ramp function
    def current_ramp(self, value):
        self.enable_mode_serial(False)                                  # Disable all serial component in current window
        self.idling = False                                              # Toggle dock idling flag
        self.pushButton_StopRamp_CurrRamp.setEnabled(True)              # Enable stop button
        step = self.spinBox_SpeedInput_CurrRamp.value()                 # Obtain ramp speed
        target = self.i2b(value, self.preamp_gain)                      # Obtain target
        self.rampTo(0x15, target, step, 1000, 0, True)                  # Ramp
        self.scrollBar_Input_Setpoint.setValue(self.dsp.lastdac[5])     # Set scroll bar
        self.idling = True                                              # Toggle dock idling flag
        self.enable_mode_serial(True)                                   # Enable all serial component in current window
        
    # Setpoint ramp button 1 slot
    def current_ramp_1(self):
        value = self.spinBox_Input1_CurrRamp.value()
        threading.Thread(target = (lambda: self.current_ramp(value))).start()
    
    # Setpoint ramp button 1 slot
    def current_ramp_2(self):
        value = self.spinBox_Input2_CurrRamp.value()
        threading.Thread(target = (lambda: self.current_ramp(value))).start()
        
    # Setpoint ramp button 1 slot
    def current_ramp_3(self):
        value = self.spinBox_Input3_CurrRamp.value()
        threading.Thread(target = (lambda: self.current_ramp(value))).start()
    
    # Setpoint ramp button 1 slot
    def current_ramp_4(self):
        value = self.spinBox_Input4_CurrRamp.value()
        threading.Thread(target = (lambda: self.current_ramp(value))).start()
        
    # Initial setpoint out of range message window
    def current_out_of_range_message(self):
        msgBox = QMessageBox()                          # Creat a message box
        msgBox.setIcon(QMessageBox.Information)         # Set icon
        msgBox.setText("Set point is out of target range")    # Out of range message
        msgBox.setWindowTitle("Current")                # Set title
        msgBox.setStandardButtons(QMessageBox.Ok)       # OK button
        msgBox.exec_()
        
    # Initial feedback off message window
    def current_feedback_off_message(self):
        msgBox = QMessageBox()                          # Creat a message box
        msgBox.setIcon(QMessageBox.Warning)             # Set icon
        msgBox.setText("Need to turn the feedback off before changing preamp gain")    # Feedback off
        msgBox.setWindowTitle("Current")                # Set title
        msgBox.setStandardButtons(QMessageBox.Ok)       # OK button
        msgBox.exec_()

# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 16:04:07 2020

@author: yaoji
"""

import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QMessageBox, QButtonGroup
from MainMenu import myMainMenu
import conversion as cnv
import threading

# myCurrentControl - This class handles current dock control
class myCurrentControl(myMainMenu):
    # Init current dock
    def init_current_dock(self):
        self.init_current()                                                             # Init current dock view
        self.spinBox_SpeedInput_CurrRamp.setValue(10)                                   # Some initial speed value
        
        # Connect signals and slots
        self.preamp_gain_group = QButtonGroup()
        self.preamp_gain_group.addButton(self.radioButton_8_Gain, 8)
        self.preamp_gain_group.addButton(self.radioButton_9_Gain, 9)
        self.preamp_gain_group.addButton(self.radioButton_10_Gain, 10)
        self.preamp_gain_group.buttonToggled[int, bool].connect(self.current_gain)
        
        self.spinBox_Input_Setpoint.editingFinished.connect(self.current_value)         # Setpoint main spin box
        self.scrollBar_Input_Setpoint.valueChanged.connect(self.current_out)            # Setpoint scroll bar
        
        # Ramp button 1 to 4
        self.pushButton_Rampto1_CurrRamp.clicked.connect(lambda: self.current_ramp(0))
        self.pushButton_Rampto2_CurrRamp.clicked.connect(lambda: self.current_ramp(1))
        self.pushButton_Rampto3_CurrRamp.clicked.connect(lambda: self.current_ramp(2))
        self.pushButton_Rampto4_CurrRamp.clicked.connect(lambda: self.current_ramp(3))
        self.pushButton_StopRamp_CurrRamp.clicked.connect(self.current_stop)            # Stop ramp button
    
    # Show current dock
    def current_show(self):
        self.init_current() # Reinit current dock view
        self.Current.show() # Show current dock
    
    # Update current indication
    def current_update(self):
        value = cnv.b2i(self.dsp.lastdac[5], self.preamp_gain, self.dsp.dacrange[5])
        self.spinBox_Input_Setpoint.setValue(value)  # Update main spinbox only, scrollbar will not follow

    # Pream gain execution
    def current_gain_excu(self, gain, value):
        if self.idling:
            self.enable_mode_serial(False)                  # Disable all serial component in current window
            self.idling = False                             # Toggle dock idling flag
            self.preamp_gain = gain                         # Change preamp gain flag
            self.current_spinbox_range()                    # Set all spin boxes range
            bits = cnv.i2b(value, self.preamp_gain, self.dsp.dacrange[5])        # Obtain target bits after changing gain
            self.dsp.rampTo(0x15, bits, 200, 10000, 0, True)  # Ramp to target
            self.scrollBar_Input_Setpoint.setValue(0xffff - bits)    # Set scroll bar to current status
            self.idling = True                              # Toggle dock idling flag
            self.enable_mode_serial(True)                   # Enable all serial component in current window
    
    # Preamp gain radio button slot
    def current_gain(self, gain, status):        
        # If preamp gain is changed
        if (gain != self.preamp_gain) and status:
            minimum = cnv.b2i(0xffff, gain, self.dsp.dacrange[5])                           # Target gain setpoint minimum
            maximum = cnv.b2i(0, gain, self.dsp.dacrange[5])                                # Target gain setpoint maximum
            value = cnv.b2i(self.dsp.lastdac[5], self.preamp_gain, self.dsp.dacrange[5])    # Current setpoint
            if (value > maximum) or (value < minimum):      # If current setpoint out of target range
                QMessageBox.warning(None, "Current", "Set point is out of target range", QMessageBox.Ok) # Out of range
                self.current_set_radio()                        # Set the radio button back
            elif self.dsp.lastdigital[2] and (not self.dsp.lastdigital[3]):    # If feedack is on and retract is off
                QMessageBox.warning(None, "Current", "Turn the FEEDBACK OFF or RETRACT ON!", QMessageBox.Ok)  # Pop out window to remind
                self.current_set_radio()                        # Set the radio button back
            else:                                           # Everythin is good
                threading.Thread(target = (lambda: self.current_gain_excu(gain, value))).start()    # Execute with thread

    # Setpoint spinBox slot
    def current_value(self):
        value = self.spinBox_Input_Setpoint.value()     # Current setpoint
        self.scrollBar_Input_Setpoint.setValue(0xffff - cnv.i2b(value, self.preamp_gain, self.dsp.dacrange[5]))   # Change I set by changing scrollbar value
    
    # Setpoint scroll bar slot
    def current_out(self, bits):
        bits = 0xffff - bits
        if bits != self.dsp.lastdac[5]:     # It it is a real change
            self.dsp.dac_W(0x15, bits)      # Direct output I set
         # Set main spin box for indication
        self.spinBox_Input_Setpoint.setValue(cnv.b2i(bits, self.preamp_gain, self.dsp.dacrange[5]))
    
    # Current stop ramp button slot
    def current_stop(self):
        self.dsp.stop = True    # Toggle DSP stop flag
        self.pushButton_StopRamp_CurrRamp.setEnabled(False) # Prevent multiple toggle

    # Setpoint ramp function
    def current_ramp_excu(self, value):
        if self.idling:
            self.enable_mode_serial(False)                                          # Disable all serial component in current window
            self.idling = False                                                     # Toggle dock idling flag
            self.pushButton_StopRamp_CurrRamp.setEnabled(True)                      # Enable stop button
            step = self.spinBox_SpeedInput_CurrRamp.value()                         # Obtain ramp speed
            target = cnv.i2b(value, self.preamp_gain, self.dsp.dacrange[5])         # Obtain target
            self.dsp.rampTo(0x15, target, step * 10, 10000, 0, True)                # Ramp
            self.scrollBar_Input_Setpoint.setValue(0xffff - self.dsp.lastdac[5])    # Set scroll bar
            self.idling = True                                                      # Toggle dock idling flag
            self.enable_mode_serial(True)                                           # Enable all serial component in current window
    
    # Ramp set current slot
    def current_ramp(self, index):
        spin = [self.spinBox_Input1_CurrRamp, self.spinBox_Input2_CurrRamp, \
                self.spinBox_Input3_CurrRamp, self.spinBox_Input4_CurrRamp][index]  # Get spin box
        value = spin.value()                                                        # Get ramp target value
        threading.Thread(target = (lambda: self.current_ramp_excu(value))).start()  # Execute with thread

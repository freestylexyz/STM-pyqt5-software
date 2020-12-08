# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 13:52:10 2020

@author: yaoji
"""

import sys
sys.path.append("./ui/")
from MainMenu import myMainMenu
from PyQt5.QtWidgets import QMessageBox
import conversion as cnv
import threading

# myBiasControl - This class handles bias dock control
class myBiasControl(myMainMenu):
    # Init bias dock
    def init_bias_dock(self):
        self.init_bias()        # Initial bias dock view
        
        # Connect signals and slots
        self.scrollBar_Input_Bias.valueChanged.connect(self.bias_out)       # Scroll bar
        self.spinBox_Input_Bias.editingFinished.connect(self.bias_value)    # Main spinbox
        self.pushButton_Filp_Bias.clicked.connect(self.bias_flip)           # Flip button
        
        self.radioButton_ON_BiasDither.toggled.connect(self.bias_dither)    # Bias dither radio button
        
        self.radioButton_20bit_DAC.toggled.connect(self.bias_dac_slot)      # Bias DAC selection
        
        # Range selection
        self.radioButton_25_Range.toggled.connect(self.bias_range)
        self.radioButton_5_Range.toggled.connect(self.bias_range)
        self.radioButton_10_Range.toggled.connect(self.bias_range)
        
        # Ramp button 1 to 8
        self.pushButton_Rampto1_BiasRamp.clicked.connect(self.bias_ramp_1)
        self.pushButton_Rampto2_BiasRamp.clicked.connect(self.bias_ramp_2)
        self.pushButton_Rampto3_BiasRamp.clicked.connect(self.bias_ramp_3)
        self.pushButton_Rampto4_BiasRamp.clicked.connect(self.bias_ramp_4)
        self.pushButton_Rampto5_BiasRamp.clicked.connect(self.bias_ramp_5)
        self.pushButton_Rampto6_BiasRamp.clicked.connect(self.bias_ramp_6)
        self.pushButton_Rampto7_BiasRamp.clicked.connect(self.bias_ramp_7)
        self.pushButton_Rampto8_BiasRamp.clicked.connect(self.bias_ramp_8)
        self.pushButton_StopRamp_BiasRamp.clicked.connect(self.bias_stop)   # Stop ramp button
    
    # Update bias indication
    def bias_update(self, bits):
        if self.bias_dac:
            value = cnv.bv(bits, '20')
        else:
            value = cnv.bv(bits, 'd', self.dsp.dacrange[13])
        self.spinBox_Input_Bias.setValue(value)                 # Update main spin box only, scroll bar will not follow
    
    # Show bias dock
    def bias_show(self):
        self.init_bias()    # Reinital bias dock view, every time call bias dock
        self.Bias.show()    # Show bias dock
    
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
    
    # Check feedback and cross zero to determin if OK to change current value
    # True stands for can not change    
    def check_feedback(self, bits, equal0):
        fb = self.dsp.lastdigital[2]    # Obtain current feedback status
        
        if self.bias_dac: # Case with 20 bit DAC 
            if equal0:
                cross_zero = ((bits - 0x80000) * (self.dsp.last20bit - 0x80000)) == 0   # Only check equal to 0, use for direct output
            else:
                cross_zero = ((bits - 0x80000) * (self.dsp.last20bit - 0x80000)) <= 0   # Check if target and current cross zero, use for ramp
        else: # Case with 16 bit DAC
            if equal0:
                cross_zero = ((bits - 0x8000) * (self.dsp.lastdac[13] - 0x8000)) == 0  # Only check equal to 0, use for direct output
            else:
                cross_zero = ((bits - 0x8000) * (self.dsp.lastdac[13] - 0x8000)) <= 0  # Check if target and current cross zero, use for ramp
        
        return fb and cross_zero    # Return if output rule violated
    
    # Set up all spin boxes input range
    def bias_spinbox_range(self):
        # Determin input limit based on selected DAC and DAC range
        if self.bias_dac:
            minimum = cnv.bv(0, '20')
            maximum = cnv.bv(0xfffff, '20')
        else:
            minimum = cnv.bv(0, 'd', self.dsp.dacrange[13])
            maximum = cnv.bv(0xffff, 'd', self.dsp.dacrange[13])
            
        # Set minimum for main spin box and ramp spin boxes
        self.spinBox_Input_Bias.setMinimum(minimum)
        self.spinBox_Input1_BiasRamp.setMinimum(minimum)
        self.spinBox_Input2_BiasRamp.setMinimum(minimum)
        self.spinBox_Input3_BiasRamp.setMinimum(minimum)
        self.spinBox_Input4_BiasRamp.setMinimum(minimum)
        self.spinBox_Input5_BiasRamp.setMinimum(minimum)
        self.spinBox_Input6_BiasRamp.setMinimum(minimum)
        self.spinBox_Input7_BiasRamp.setMinimum(minimum)
        self.spinBox_Input8_BiasRamp.setMinimum(minimum)
        
        # Set maximum for main spin box and ramp spin boxes
        self.spinBox_Input_Bias.setMaximum(maximum)
        self.spinBox_Input1_BiasRamp.setMaximum(maximum)
        self.spinBox_Input2_BiasRamp.setMaximum(maximum)
        self.spinBox_Input3_BiasRamp.setMaximum(maximum)
        self.spinBox_Input4_BiasRamp.setMaximum(maximum)
        self.spinBox_Input5_BiasRamp.setMaximum(maximum)
        self.spinBox_Input6_BiasRamp.setMaximum(maximum)
        self.spinBox_Input7_BiasRamp.setMaximum(maximum)
        self.spinBox_Input8_BiasRamp.setMaximum(maximum)
    
    # Set bias range radio button
    def bias_range_radio(self):
        if self.bias_dac:       # 20bit DAC case
            self.radioButton_5_Range.setChecked(True)   # Set range to +/-5V
            self.groupBox_Range_Bias.setEnabled(False)  # Disable range selection
        else:                   # 16bit DAC case
            if self.dsp.dacrange[13] == 14:
                self.radioButton_25_Range.setChecked(True)  # Set +/-2.5V radio button
            elif self.dsp.dacrange[13] == 9:
                self.radioButton_5_Range.setChecked(True)   # Set +/-5V radio button
            elif self.dsp.dacrange[13] == 10:
                self.radioButton_10_Range.setChecked(True)  # Set +/-10V radio button
            
            if self.dsp.succeed:
                self.groupBox_Range_Bias.setEnabled(True)   # Enable range selection when found DSP
            else:
                self.groupBox_Range_Bias.setEnabled(False)  # Disable range selection when no DSP found
                
    # Bias spinBox slot
    def bias_value(self):
        value = self.spinBox_Input_Bias.value()     # Obtain current value
        
        # Output bias by changing scroll bar
        if self.bias_dac:
            self.scrollBar_Input_Bias.setValue(cnv.vb(value, '20'))
        else:
            self.scrollBar_Input_Bias.setValue(cnv.vb(value, 'd', self.dsp.dacrange[13]))
    
    # Bias scrollBar slot
    def bias_out(self, bits):
        if self.bias_dac:                    # 20bit DAC case
            if bits != self.dsp.last20bit:      # Proceed next if it is a real change
                if self.check_feedback(bits, True):     # Can not change 
                    self.cross_zero_message()           # Cross zero message
                    self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)      # Change scroll bar back to current status
                else:                                   # Can change
                    self.dsp.bit20_W(0x10, bits)                                # Direct output
            self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.last20bit, '20'))  # Set main spin box based on DSP status
        else:                                # 16 bit case
            if bits != self.dsp.lastdac[13]:    # Proceed next if it is a real change
                if self.check_feedback(bits, True):     # Can not change
                    self.cross_zero_message()           # Cross zero message
                    self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])    # Change scroll bar back to current status
                else:                                   # Can change
                    self.dsp.dac_W(0x1d, bits)                                  # Direct output
            self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.lastdac[13], 'd', self.dsp.dacrange[13])) # Set main spin box based on DSP status
    
    # Bias flip button slot
    def bias_flip(self):
        bits = self.scrollBar_Input_Bias.value()    # Obtain current scroll bar bits
        
        # Reverse bits polarity
        if self.bias_dac:
            bits = 0xfffff - bits
        else:
            bits = 0xffff - bits
        
        self.scrollBar_Input_Bias.setValue(bits)    # Output bias by changing scroll bar
        
    # Bias dither radio button slot
    def bias_dither(self, checked):
        if self.dsp.lastdigital[0] != checked:
            self.dsp.digital_o(0, checked)          # Toggle bias dither
    
    # Bias range radio button slot
    def bias_range(self):
        # Determin bias range variable
        if self.radioButton_25_Range.isChecked():
            ran = 14
        elif self.radioButton_5_Range.isChecked():
            ran = 9
        elif self.radioButton_10_Range.isChecked():
            ran = 10
        
        # If really need to change bias range
        if self.dsp.dacrange[13] != ran:
            minimum = cnv.bv(0, 'd', ran)                   # Target range minimum
            maximum = cnv.bv(0xffff, 'd', ran)              # Target range maximum
            value = self.spinBox_Input_Bias.value()         # Current value
            if (value > maximum) or (value < minimum):      # If current bias out of target range, abort range changing
                self.bias_out_of_range_message()                        # Out of range message
                self.bias_range_radio()                                 # Restore to original radio button setup
            else:                                           # Continue if current bias in the target range
                if not self.bias_dac:
                    threading.Thread(target = (lambda: self.bias_range_excu(ran, value))).start()  # Execute with thread

                    
    # Bias range execution
    def bias_range_excu(self, ran, value):
        self.enable_mode_serial(False)                      # Disable all serial related component in current window
        self.idling = False                                 # Toggle dock idling flag
        feedback_store = self.dsp.lastdigital[2]            # Store current feedback status
        self.dsp.digital_o(2, False)                        # Feedback off
        self.dsp.rampTo(0x1d, 0x8000, 10, 200, 0, False)    # Ramp bias to 0
        self.dsp.dac_range(13, ran)                         # Change range
        self.dsp.rampTo(0x1d, cnv.vb(value, 'd', self.dsp.dacrange[13]), 10, 200, 0, False)  # Restore to original bias voltage
        self.dsp.digital_o(2, feedback_store)               # Restore orignial feedback status
                
        self.bias_spinbox_range()                           # Set spin boxes range
        self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])                                    # Set scroll bar value
        self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.lastdac[13], 'd', self.dsp.dacrange[13]))  # Set spin box value
        self.idling = True                                  # Toggle dock idling flag
        self.enable_mode_serial(True)                       # Enable all serial related component in current window
        

    # Bias stop ramp button slot
    def bias_stop(self):
        self.dsp.stop = True                                    # Toggle DSP stop flag
        self.pushButton_StopRamp_BiasRamp.setEnabled(False)     # Disable stop push button (no need to push again)
    
    # Bias check ramp function
    def bias_check_ramp(self, value):
        # Figure out target based on bias DAC
        if self.bias_dac:
            channel = 0x20
            target = cnv.vb(value, '20')
        else:
            channel = 0x1d
            target = cnv.vb(value, 'd', self.dsp.dacrange[13])
            
        # Check if able to ramp
        if self.check_feedback(target, False):  # Can not ramp
            self.cross_zero_message()           # Cross zero message box
            ok = False
        else:                                   # Can ramp
            ok = True
        return ok, channel, target
    
    # Bias ramp function
    def bias_ramp(self, channel, target):
        self.enable_mode_serial(False)                      # Disable all serial related component in current window
        self.idling = False                                 # Toggle dock idling flag
        self.pushButton_StopRamp_BiasRamp.setEnabled(True)  # Enable stop push button
        step = self.spinBox_SpeedInput_BiasRamp.value()     # Obtain step value
        self.dsp.rampTo(channel, target, step * 10, 10000, 0, True)   # Ramp

        # Update scroll bar with DSP status variable
        if self.bias_dac:
            self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
        else:
            self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])           
        self.idling = True                  # Toggle dock idling flag
        self.enable_mode_serial(True)       # Enable all serial related component in current window
        
    # Bias ramp button 1 slot
    def bias_ramp_1(self):
        value = self.spinBox_Input1_BiasRamp.value()                        # Obtain target
        ok, channel, bits = self.bias_check_ramp(value)                     # Check ramp
        if ok:
            threading.Thread(target = (lambda: self.bias_ramp(channel, bits))).start()  # Ramp with thread
        else:
            # Update scroll bar with DSP status variable
            if self.bias_dac:
                self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
            else:
                self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])
        
    # Bias ramp button 2 slot
    def bias_ramp_2(self):
        value = self.spinBox_Input2_BiasRamp.value()                        # Obtain target
        ok, channel, bits = self.bias_check_ramp(value)                     # Check ramp
        if ok:
            threading.Thread(target = (lambda: self.bias_ramp(channel, bits))).start()  # Ramp with thread
        else:
            # Update scroll bar with DSP status variable
            if self.bias_dac:
                self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
            else:
                self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])
    
    # Bias ramp button 3 slot
    def bias_ramp_3(self):
        value = self.spinBox_Input3_BiasRamp.value()                        # Obtain target
        ok, channel, bits = self.bias_check_ramp(value)                     # Check ramp
        if ok:
            threading.Thread(target = (lambda: self.bias_ramp(channel, bits))).start()  # Ramp with thread
        else:
            # Update scroll bar with DSP status variable
            if self.bias_dac:
                self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
            else:
                self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])
    
    # Bias ramp button 4 slot
    def bias_ramp_4(self):
        value = self.spinBox_Input4_BiasRamp.value()                        # Obtain target
        ok, channel, bits = self.bias_check_ramp(value)                     # Check ramp
        if ok:
            threading.Thread(target = (lambda: self.bias_ramp(channel, bits))).start()  # Ramp with thread
        else:
            # Update scroll bar with DSP status variable
            if self.bias_dac:
                self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
            else:
                self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])

    # Bias ramp button 5 slot
    def bias_ramp_5(self):
        value = self.spinBox_Input5_BiasRamp.value()                        # Obtain target
        ok, channel, bits = self.bias_check_ramp(value)                     # Check ramp
        if ok:
            threading.Thread(target = (lambda: self.bias_ramp(channel, bits))).start()  # Ramp with thread
        else:
            # Update scroll bar with DSP status variable
            if self.bias_dac:
                self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
            else:
                self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])
        
    # Bias ramp button 6 slot
    def bias_ramp_6(self):
        value = self.spinBox_Input6_BiasRamp.value()                        # Obtain target
        ok, channel, bits = self.bias_check_ramp(value)                     # Check ramp
        if ok:
            threading.Thread(target = (lambda: self.bias_ramp(channel, bits))).start()  # Ramp with thread
        else:
            # Update scroll bar with DSP status variable
            if self.bias_dac:
                self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
            else:
                self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])
    
    # Bias ramp button 7 slot
    def bias_ramp_7(self):
        value = self.spinBox_Input7_BiasRamp.value()                        # Obtain target
        ok, channel, bits = self.bias_check_ramp(value)                     # Check ramp
        if ok:
            threading.Thread(target = (lambda: self.bias_ramp(channel, bits))).start()  # Ramp with thread
        else:
            # Update scroll bar with DSP status variable
            if self.bias_dac:
                self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
            else:
                self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])
    
    # Bias ramp button 8 slot
    def bias_ramp_8(self):
        value = self.spinBox_Input8_BiasRamp.value()                        # Obtain target
        ok, channel, bits = self.bias_check_ramp(value)                     # Check ramp
        if ok:
            threading.Thread(target = (lambda: self.bias_ramp(channel, bits))).start()  # Ramp with thread
        else:
            # Update scroll bar with DSP status variable
            if self.bias_dac:
                self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)
            else:
                self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])

    # Initial bias out of range message window
    def bias_out_of_range_message(self):
        msgBox = QMessageBox()                          # Creat a message box
        msgBox.setIcon(QMessageBox.Warning)             # Set icon
        msgBox.setText("Current bias is out of target range")        # Out of range message
        msgBox.setWindowTitle("Bias")                   # Set title
        msgBox.setStandardButtons(QMessageBox.Ok)       # OK button
        msgBox.exec_()
        
    # Initial cross zero message window
    def cross_zero_message(self):
        msgBox = QMessageBox()                          # Creat a message box
        msgBox.setIcon(QMessageBox.Warning)             # Set icon
        msgBox.setText("Feedback in on can not cross zero")        # Cross zero message
        msgBox.setWindowTitle("Bias")                   # Set title
        msgBox.setStandardButtons(QMessageBox.Ok)       # OK button
        msgBox.exec_()
# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 13:52:10 2020

@author: yaoji
"""

import sys
sys.path.append("./ui/")
from MainMenu import myMainMenu
from PyQt5.QtWidgets import QMessageBox, QButtonGroup
from PyQt5.QtCore import pyqtSignal
import conversion as cnv
import threading
import time

# myBiasControl - This class handles bias dock control
class myBiasControl(myMainMenu):
    bias_range_signal = pyqtSignal(int)
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
        self.bias_range_group = QButtonGroup()
        self.bias_range_group.addButton(self.radioButton_25_Range, 14)
        self.bias_range_group.addButton(self.radioButton_5_Range, 9)
        self.bias_range_group.addButton(self.radioButton_10_Range, 10)
        self.bias_range_group.buttonToggled[int, bool].connect(self.bias_range)
        
        # Ramp button 1 to 8
        self.pushButton_Rampto1_BiasRamp.clicked.connect(lambda: self.bias_ramp(0))
        self.pushButton_Rampto2_BiasRamp.clicked.connect(lambda: self.bias_ramp(1))
        self.pushButton_Rampto3_BiasRamp.clicked.connect(lambda: self.bias_ramp(2))
        self.pushButton_Rampto4_BiasRamp.clicked.connect(lambda: self.bias_ramp(3))
        self.pushButton_Rampto5_BiasRamp.clicked.connect(lambda: self.bias_ramp(4))
        self.pushButton_Rampto6_BiasRamp.clicked.connect(lambda: self.bias_ramp(5))
        self.pushButton_Rampto7_BiasRamp.clicked.connect(lambda: self.bias_ramp(6))
        self.pushButton_Rampto8_BiasRamp.clicked.connect(lambda: self.bias_ramp(7))
        self.pushButton_StopRamp_BiasRamp.clicked.connect(self.bias_stop)   # Stop ramp button
    
    # Update bias indication
    def bias_update(self):
        value = cnv.bv(self.dsp.last20bit, '20') if self.bias_dac else cnv.bv(self.dsp.lastdac[13], 'd', self.dsp.dacrange[13])
        # Update main spin box only, scroll bar will not follow (on poupose)
        self.spinBox_Input_Bias.setValue(value) 
    
    # Show bias dock
    def bias_show(self):
        self.init_bias()    # Reinital bias dock view, every time call bias dock
        self.Bias.show()    # Show bias dock

    # Check feedback and cross zero to determin if OK to change current value
    # True stands for can not change    
    def check_feedback(self, bits, equal0):
        fb = self.dsp.lastdigital[2]    # Obtain current feedback status
        
        if self.bias_dac: # Case with 20 bit DAC 
            equal_zero = ((bits - 0x80000) == 0)   # Only check equal to 0, use for direct output
            cross_zero = (((bits - 0x80000) * (self.dsp.last20bit - 0x80000)) < 0)   # Check if target and current cross zero, use for ramp
        else: # Case with 16 bit DAC
            equal_zero = ((bits - 0x8000) == 0)  # Only check equal to 0, use for direct output
            cross_zero = (((bits - 0x8000) * (self.dsp.lastdac[13] - 0x8000)) < 0)  # Check if target and current cross zero, use for ramp
        
        cross_zero = equal_zero if equal0 else (cross_zero or equal_zero)
        return fb and cross_zero    # Return if output rule violated

    # Bias spinBox slot
    def bias_value(self):
        value = self.spinBox_Input_Bias.value()                                                     # Obtain current voltage
        bits = cnv.vb(value, '20') if self.bias_dac else cnv.vb(value, 'd', self.dsp.dacrange[13])  # Calcualte bits
        self.scrollBar_Input_Bias.setValue(bits)                                                    # Update scrollbar
    
    # Bias scrollBar slot
    def bias_out(self, bits):
        if self.bias_dac:                    # 20bit DAC case
            if bits != self.dsp.last20bit:      # Proceed next if it is a real change
                if self.check_feedback(bits, True):     # Can not change 
                    QMessageBox.warning(None, "Bias", "Feedback in on can not cross zero", QMessageBox.Ok) # Cross zero message box
                    self.scrollBar_Input_Bias.setValue(self.dsp.last20bit)      # Change scroll bar back to current status
                else:                                   # Can change
                    self.dsp.bit20_W(0x10, bits)                                # Direct output
            self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.last20bit, '20'))  # Set main spin box based on DSP status
        else:                                # 16 bit case
            if bits != self.dsp.lastdac[13]:    # Proceed next if it is a real change
                if self.check_feedback(bits, True):     # Can not change
                    QMessageBox.warning(None, "Bias", "Feedback in on can not cross zero", QMessageBox.Ok)      # Cross zero message
                    self.scrollBar_Input_Bias.setValue(self.dsp.lastdac[13])    # Change scroll bar back to current status
                else:                                   # Can change
                    self.dsp.dac_W(0x1d, bits)                                  # Direct output
            self.spinBox_Input_Bias.setValue(cnv.bv(self.dsp.lastdac[13], 'd', self.dsp.dacrange[13])) # Set main spin box based on DSP status
    
    # Bias flip button slot
    def bias_flip(self):
        bits = (0xfffff - self.scrollBar_Input_Bias.value()) if self.bias_dac else (0xffff - self.scrollBar_Input_Bias.value())
        self.scrollBar_Input_Bias.setValue(bits)    # Output bias by changing scroll bar
        
    # Bias dither radio button slot
    def bias_dither(self, checked):
        if self.dsp.lastdigital[0] != checked:
            self.dsp.digital_o(0, checked)          # Toggle bias dither
    
    # Bias range radio button slot
    def bias_range(self, ran, status):
        # If really need to change bias range
        if (self.dsp.dacrange[13] != ran) and status:
            minimum = cnv.bv(0, 'd', ran)                   # Target range minimum
            maximum = cnv.bv(0xffff, 'd', ran)              # Target range maximum
            value = self.spinBox_Input_Bias.value()         # Current value
            if (value > maximum) or (value < minimum):      # If current bias out of target range, abort range changing
                QMessageBox.warning(None, "Bias", "Current bias is out of target range!", QMessageBox.Ok)    # Pop out remider
                self.bias_range_radio()                                                                      # Restore to original radio button setup
            else:                                           # Continue if current bias in the target range
                if (not self.bias_dac) and self.idling:
                    threading.Thread(target = (lambda: self.bias_range_excu(ran, value))).start()            # Execute with thread
                    
    # Bias range execution
    def bias_range_excu(self, ran, value):
        if self.idling:
            self.enable_mode_serial(False)                      # Disable all serial related component in current window
            self.idling = False                                 # Toggle dock idling flag
            feedback_store = self.dsp.lastdigital[2]            # Store current feedback status
            self.dsp.digital_o(2, False)                        # Feedback off
            time.sleep(0.5)                                     # The reed relay will take some time
            self.dsp.rampTo(0x1d, 0x8000, 10, 200, 0, True)     # Ramp bias to 0
            self.dsp.dac_range(13, ran)                         # Change range
            self.bias_range_signal.emit(ran)                    # Emit bias range signal
            self.dsp.rampTo(0x1d, cnv.vb(value, 'd', self.dsp.dacrange[13]), 10, 200, 0, True)  # Restore to original bias voltage
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
        channel = 0x20 if self.bias_dac else 0x1d
        target = cnv.vb(value, '20') if self.bias_dac else cnv.vb(value, 'd', self.dsp.dacrange[13])
            
        # Check if able to ramp
        ok = True                               # Default can ramp
        if self.check_feedback(target, False):  # Can not ramp
            QMessageBox.warning(None, "Bias", "Feedback in on can not cross zero", QMessageBox.Ok) # Cross zero message box
            ok = False
        return ok, channel, target
    
    # Bias ramp function
    def bias_ramp_excu(self, channel, target):
        if self.idling:
            self.enable_mode_serial(False)                      # Disable all serial related component in current window
            self.idling = False                                 # Toggle dock idling flag
            self.pushButton_StopRamp_BiasRamp.setEnabled(True)  # Enable stop push button
            step = self.spinBox_SpeedInput_BiasRamp.value()     # Obtain step value
            self.dsp.rampTo(channel, target, step * 10, 10000, 0, True)   # Ramp
            
            # Update scroll bar with DSP status variable
            bits = self.dsp.last20bit if self.bias_dac else self.dsp.lastdac[13]
            self.scrollBar_Input_Bias.setValue(bits)         
            self.idling = True                  # Toggle dock idling flag
            self.enable_mode_serial(True)       # Enable all serial related component in current window
            
    # Bias ramp slot
    def bias_ramp(self, index):
        # Obtain spinbox
        spin = [self.spinBox_Input1_BiasRamp, self.spinBox_Input2_BiasRamp, self.spinBox_Input3_BiasRamp, self.spinBox_Input4_BiasRamp,\
                self.spinBox_Input5_BiasRamp, self.spinBox_Input6_BiasRamp, self.spinBox_Input7_BiasRamp, self.spinBox_Input8_BiasRamp][index]
            
        value = spin.value()                                                # Obtain target
        ok, channel, bits = self.bias_check_ramp(value)                     # Check ramp
        if ok:
            threading.Thread(target = (lambda: self.bias_ramp_excu(channel, bits))).start()  # Ramp with thread
        else:
            # Update scroll bar with DSP status variable
            bits = self.dsp.last20bit if self.bias_dac else self.dsp.lastdac[13]
            self.scrollBar_Input_Bias.setValue(bits)
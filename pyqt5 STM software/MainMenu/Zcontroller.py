# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 16:04:05 2020

@author: yaoji
"""

import sys

sys.path.append("./ui/")
from PyQt5.QtWidgets import QMessageBox, QButtonGroup
from MainMenu import myMainMenu
import time
import threading
from functools import partial


class myZcontroller(myMainMenu):
    # Init Zcontroller dock
    def init_Zcontroller_dock(self):
        self.init_Zcontroller()

        # Z offset coarse
        self.scrollBar_Input_Zoffset.valueChanged.connect(self.spinBox_Input_Zoffset.setValue)
        self.spinBox_Input_Zoffset.valueChanged.connect(self.scrollBar_Input_Zoffset.setValue)
        self.pushButton_Send_Zoffset.clicked.connect(self.z_send)
        self.pushButton_Zauto_Zoffset.clicked.connect(self.z_auto)
        self.pushButton_HardRetract_Zoffset.clicked.connect(lambda: self.z_retract(True, True))

        # Z offset fine
        self.pushButton_Advance_Zoffsetfine.clicked.connect(self.z_advance)
        self.pushButton_Zero_Zoffsetfine.clicked.connect(self.z_zero)
        self.pushButton_MatchCurrent_Zoffsetfine.clicked.connect(self.z_match_i)

        # Digital toggle
        self.radioButton_ON_Feedback.toggled.connect(self.z_feedback)
        self.radioButton_ON_Retract.toggled.connect(partial(self.z_retract, False))
        self.radioButton_ON_ZDither.toggled.connect(self.z_dither)

        # Gain Change
        self.Z1_gain_group = QButtonGroup()
        self.Z1_gain_group.addButton(self.radioButton_Z1gain01_Gain, 3)
        self.Z1_gain_group.addButton(self.radioButton_Z1gain1_Gain, 1)
        self.Z1_gain_group.addButton(self.radioButton_Z1gain10_Gain, 0)
        self.Z1_gain_group.buttonToggled[int, bool].connect(self.z_gain_1)

        self.Z2_gain_group = QButtonGroup()
        self.Z2_gain_group.addButton(self.radioButton_Z2gain01_Gain, 0)
        self.Z2_gain_group.addButton(self.radioButton_Z2gain1_Gain, 1)
        self.Z2_gain_group.addButton(self.radioButton_Z2gain10_Gain, 3)
        self.Z2_gain_group.buttonToggled[int, bool].connect(self.z_gain_2)

    # Show Zcontroller dock
    def Zcontroller_show(self):
        self.groupBox_Gain_Zcontrol.setEnabled(self.mode == 1)
        self.init_Zcontroller()  # Re-init Z control view
        self.Zcontrol.show()  # Show Z control

    # Z offset coarse send
    def z_send(self):
        bits = self.scrollBar_Input_Zoffset.value()  # Obtain target bits
        bits = bits + 0x8000  # Add view offset
        if bits != self.dsp.lastdac[3]:  # If it is a real output
            threading.Thread(target=(lambda: self.z_send_excu(bits))).start()  # Execute with thread
        else:
            self.scrollBar_Input_Zoffset.setValue(self.dsp.lastdac[3] - 0x8000)  # Update indication
            self.z_offset_update()  # Update input spin box

    # Z offset coarse ramp execution
    def z_send_excu(self, bits):
        if self.idling:
            self.enable_mode_serial(False)  # Disable all serial related component in current mode
            self.idling = False  # Toggle idling flag
            self.dsp.rampTo(0x13, bits, 1, 100, 0, False)  # Ramp Z offset to target value
            self.scrollBar_Input_Zoffset.setValue(self.dsp.lastdac[3] - 0x8000)  # Update scrollbar value
            self.z_offset_update()  # Update indication
            self.idling = True  # Toggle back idling flag
            self.enable_mode_serial(True)  # Enable all serial related component in current mode

    # Z auto 0
    def z_auto(self):
        if self.dsp.lastdigital[2]:  # If feedback is on
            threading.Thread(target=self.z_auto_excu).start()  # Execute Z auto with thread
        else:  # If feedback is off
            QMessageBox.warning(None, "Z control", "Feedback is OFF!",
                                QMessageBox.Ok)  # Pop out reminder and abort action

    # Z auto 0 execution
    def z_auto_excu(self):
        if self.idling:
            self.enable_mode_serial(False)  # Disable all serial related component in current mode
            self.idling = False  # Toggle idling flag
            self.dsp.rampTo(0x12, 0x8000, 100, 1000, 0, False)  # Return Z offset fine to zero
            self.z_fine_update()  # Update Z offset fine indication
            time.sleep(2)  # Wait 2 seconds to let it stabilize
            self.dsp.zAuto0()  # Command DSP to do Z auto 0
            self.scrollBar_Input_Zoffset.setValue(self.dsp.lastdac[3] - 0x8000)  # Update Z offset scroll bar
            self.z_offset_update()  # Update Z offset indication
            self.idling = True  # Toggle back idling flag
            self.enable_mode_serial(True)  # Enable all serial related component in current mode

    # Z offset fine advance execution
    def z_advance_excu(self, bits):
        if self.idling:
            self.enable_mode_serial(False)  # Disable all serial related component in current mode
            self.idling = False  # Toggle idling flange
            self.dsp.rampTo(0x12, bits, 100, 1000, 0, False)  # Return Z offset fine to target bits
            self.z_fine_update()  # Update Z offset fine spin box
            self.idling = True  # Toggle back idling flag
            self.enable_mode_serial(True)  # Enable all serial related component in current mode

    # Z offset fine advance
    def z_advance(self):
        bits = self.spinBox_AdvInput_Zoffsetfine.value()  # Obtain the number of bits need to advanced
        bits = max(0, min(0xffff,
                          bits + self.dsp.lastdac[2]))  # Add advancing bits to current bits and limited by boundary
        if bits != self.dsp.lastdac[2]:  # If it is a real output
            threading.Thread(target=(lambda: self.z_advance_excu(bits))).start()  # Send to target with thread

    # Z offset fine zero
    def z_zero(self):
        if self.dsp.lastdac[2] != 0x8000:  # If current z offset fine is not 0
            threading.Thread(target=(lambda: self.z_advance_excu(0x8000))).start()  # Send to zero with thread

    # Z match current execution
    def z_match_excu(self):
        self.enable_mode_serial(False)  # Disable all serial related component in current mode
        self.idling = False  # Toggle idling flag
        self.dsp.iAuto()  # Command DSP to do current auto
        self.z_fine_update()  # Update Z offset fine spin box
        self.idling = True  # Toggle back idling flag
        self.enable_mode_serial(True)  # Enable all serial related component in current mode

    # Z match current
    def z_match_i(self):
        if not (self.dsp.lastdigital[2] or self.dsp.lastdigital[3]):  # If feedback off and retract off
            threading.Thread(target=self.z_match_excu).start()  # Execute i auto with thread
        elif self.dsp.lastdigital[2]:  # If feedback on
            QMessageBox.warning(None, "Z control", "Feedback is ON!",
                                QMessageBox.Ok)  # Pop out reminding message and abort
        elif self.dsp.lastdigital[3]:  # If retract on
            QMessageBox.warning(None, "Z control", "Retract is ON!",
                                QMessageBox.Ok)  # Pop out reminding message and abort

    # Feedback toggle
    def z_feedback(self, feedback):
        if feedback != self.dsp.lastdigital[2]:  # If it is a real toggle
            if (not feedback) and self.dsp.lastdigital[3]:
                QMessageBox.warning(None, "Z control", "Retract is ON, CANNOT turn off feedback!", QMessageBox.Ok)
            else:
                self.dsp.digital_o(2, feedback)  # Command DSP to toggle feedback
            self.radioButton_ON_Feedback.setChecked(self.dsp.lastdigital[2])  # Set feedback radio button to sync status
            self.radioButton_OFF_Feedback.setChecked(
                not self.dsp.lastdigital[2])  # Set feedback radio button to sync status

    # Hard retract execution
    def z_hard_excu(self):
        self.enable_mode_serial(False)  # Disable all serial related component in current mode
        self.idling = False  # Toggle idling flag
        time.sleep(5)
        self.hard_retracted = not self.hard_retracted  # Toggle hard retracted flag
        # Change push button text based on hard retracted flag
        if self.hard_retracted:
            self.pushButton_HardRetract_Zoffset.setText("Hard unretract")
            self.dsp.rampTo(0x13, 0, 1, 100, 0, False)  # Send Z offset to 0
        else:
            self.pushButton_HardRetract_Zoffset.setText("Hard retract")
            self.dsp.zAuto0()  # Command DSP to do Z auto 0
        self.scrollBar_Input_Zoffset.setValue(self.dsp.lastdac[3] - 0x8000)  # Update Z offset scroll bar
        self.z_offset_update()  # Update Z offset indication
        self.idling = True  # Toggle back idling flag
        self.enable_mode_serial(True)  # Enable all serial related component in current mode

    # Retract toggle
    def z_retract(self, hard, retract):
        popped = False  # Initial popped flag
        if hard:  # If hard retract
            retract = not self.hard_retracted  # Obtain retract data from hard retracted flag

        if retract != self.dsp.lastdigital[3]:  # If it is a real toggle
            if self.dsp.lastdigital[2]:  # If feedback is ON
                self.dsp.digital_o(3, retract)  # Command DSP to toggle retract
            else:  # If feedback is OFF
                QMessageBox.warning(None, "Z control", "Feedback is OFF!",
                                    QMessageBox.Ok)  # Pop out reminder message and abort
                popped = True  # Toggle popped flag to avoid to avoid pop same message twice
        self.radioButton_ON_Retract.setChecked(self.dsp.lastdigital[3])  # Set retract radio button to sync status
        self.radioButton_OFF_Retract.setChecked(not self.dsp.lastdigital[3])  # Set retract radio button to sync status

        if hard and self.dsp.lastdigital[2]:  # If hard retract and feedback is on
            threading.Thread(target=self.z_hard_excu).start()  # Execute Z auto after with 5 seconds delay
        elif hard and (not popped):  # If need to hard retract and haven't remind feedback is off
            QMessageBox.warning(None, "Z control", "Feedback is OFF!", QMessageBox.Ok)  # pop out window to remind
        # All other situation, directly abort

    # Z dither toggle
    def z_dither(self, dither_on):
        if dither_on != self.dsp.lastdigital[1]:  # If it is a real toggle
            self.dsp.digital_o(1, dither_on)  # Command DSP to toggle Z dither

    # Z1 gain toggle
    def z_gain_1(self, gain, status):
        if (gain != self.dsp.lastgain[2]) and status:  # If it is a real toggle
            if self.dsp.lastdigital[2]:  # If feedback is ON
                self.dsp.gain(2, gain)  # Command DSP to toggle Z1 gain
            else:  # If feedback is OFF
                QMessageBox.warning(None, "Z control", "Feedback is OFF!", QMessageBox.Ok)  # Pop out window to remind
        if status:
            self.load_z1_gain()  # Load the real Z1 gain

    # Z2 gain execution
    def z_gain2_excu(self, gain):
        if self.idling:
            self.enable_mode_serial(False)  # Disable all serial related component in current mode
            self.idling = False  # Toggle idling flag
            self.dsp.rampTo(0x12, 0x8000, 100, 1000, 0, False)  # Return Z offset fine to zero
            self.z_fine_update()  # Update Z offset fine spin box
            self.dsp.zAuto0()  # Command DSP to do Z auto
            self.scrollBar_Input_Zoffset.setValue(self.dsp.lastdac[3] - 0x8000)  # Update Z offset scroll bar
            self.z_offset_update()  # Update Z offset spin box
            bits = self.dsp.lastdac[3]  # Obtain current Z offset bits
            if gain < self.dsp.lastgain[2]:  # If changing to a smaller gain
                bits -= 300  # Adjust Z feedback to be little bit extended
                # So that it get less extended after changing gain (Piezo gets shorter)
            elif gain > self.dsp.lastgain[2]:  # If changing to a bigger gain
                bits += 300  # Adjust Z feedback to be little bit contracted
                # So that it get more contracted after changing gain (Piezo gets shorter)
            self.dsp.rampTo(0x13, bits, 1, 500, 0, False)  # Execute the adjustment
            time.sleep(1)  # Wait 1 seconds to wait for feedback Z to respond
            self.dsp.gain(3, gain)  # Command DSP to toggle Z2 gain
            self.dsp.zAuto0()  # Command DSP to execute Z auto one more time
            self.scrollBar_Input_Zoffset.setValue(self.dsp.lastdac[3] - 0x8000)  # Update Z offset scroll bar
            self.z_offset_update()  # Update Z offset spin box
            self.load_z2_gain()  # Load the real Z2 gain
            self.idling = True  # Toggle back idling flag
            self.enable_mode_serial(True)  # Enable all serial related component in current mode

    # Z2 gain toggle
    def z_gain_2(self, gain, status):
        if (gain != self.dsp.lastgain[3]) and status:  # If it is a real toggle
            if self.dsp.lastdigital[2]:  # If feedback is ON
                threading.Thread(
                    target=(lambda: self.z_gain2_excu(gain))).start()  # Execute Z2 gain changing sequence with thread
            else:  # If feedback is OFF
                QMessageBox.warning(None, "Z control", "Feedback is OFF!", QMessageBox.Ok)  # Pop out window to remind
        if status:
            self.load_z2_gain()  # Load the real Z2 gain

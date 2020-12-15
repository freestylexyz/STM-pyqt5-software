# -*- coding: utf-8 -*-
"""
Created on Thu Dec  3 11:11:17 2020

@author: yaoji
"""

import sys
sys.path.append("./ui/")
from MainMenu import myMainMenu
import threading

class myTipApproachControl(myMainMenu):
    # Giant step execution
    def giant_excu(self, channel, x, z, delay, g, stepnum):
        self.enable_mode_serial(False)                                      # Disable all serial realted features
        self.etest.idling = False                                           # Toggle idling flag
        self.etest.pushButton_Stop.setEnabled(True)                         # Enable stop push button
        gx = g                                                              # Caluculate nonlinear coefficient for x channel
        gz = g                                                              # Caluculate nonlinear coefficient for z channel
        self.dsp.giantStep(channel, x, z, 100, delay, gx, gz, stepnum)      # Execute giant stpes
        self.etest.idling = True                                            # Toggle back idling flag
        self.etest.label_Status.setText("Idling")                           # Set the status label back to idling
        self.enable_mode_serial(True)                                       # Enable all serial related features

    # Giant step slot
    def giant_slot(self, channel, x, z, delay, g):
        threading.Thread(target = (lambda: self.giant_excu(channel, x, z, delay, g))).start()
        
    # Tip approach execution
    def tip_appr_excu(self, x, z, delay, g, giant, baby, minCurr):
        self.enable_mode_serial(False)
        self.enable_mode_serial(False)                                      # Disable all serial realted features
        self.etest.idling = False                                           # Toggle idling flag
        self.etest.pushButton_Stop.setEnabled(True)                         # Enable stop push button
        gx = g                                                              # Caluculate nonlinear coefficient for x channel
        gz = g                                                              # Caluculate nonlinear coefficient for z channel
        self. dsp.tipApproach(x, z, 100, delay, gx, gz, giant, baby, gz, minCurr)      # Execute Tip approach
        self.etest.idling = True                                            # Toggle back idling flag
        self.etest.label_Status.setText("Idling")                           # Set the status label back to idling
        self.enable_mode_serial(True)                                       # Enable all serial related features
    
    # Tip approach slot
    def tip_appr_slot(self, x, z, delay, g, giant, baby, minCurr):
        threading.Thread(target = (lambda: self.tip_appr_excu(x, z, delay, g, giant, baby, minCurr))).start()
        
    # Update stepnumber
    def giantStep_update(self, i):
        self.etest.label_Pass.setText(str(i))       # Update pass number label
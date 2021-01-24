# -*- coding: utf-8 -*-
"""
Created on Thu Dec  3 11:11:17 2020

@author: yaoji
"""

import sys
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from MainMenu import myMainMenu
import threading
import time

class myTipApproachControl(myMainMenu):
    # Set up tip approach
    # Flag ture used for set up tip approach window, false for set up tip approach operation
    def setup_tipappr(self, flag):
        if self.idling or (not flag):
            if flag:
                self.enable_mode_serial(False)                                  # Disable all serial realted features
                self.idling = False
            # Set up inner piezo
            self.dsp.rampTo(0x13, 0x8000, 2, 1000, 0, False)                    # Ramp Z offset to 0V
            self.dsp.rampTo(0x12, 0x8000, 20, 1000, 0, False)                   # Ramp Z offset fine to 0V
            self.dsp.gain(2, 3)                                                 # Make sure Z1 gain is 0.1
            self.dsp.gain(3, 3)                                                 # Make sure Z2 gain to 10.0
            
            # Set up outer piezo
            self.dsp.rampTo(0x1a, 0x8000, 2, 1000, 0, False)                    # Ramp Zouter to 0V
            self.dsp.rampDiag(0x10, 0x1f, 0x8000, 0x8000, 2, 1000, 0, False)    # Ramp XY in to 0V
            self.dsp.rampDiag(0x11, 0x1e, 0x8000, 0x8000, 2, 1000, 0, False)    # Ramp XY offset to 0V
            self.dsp.gain(0, 3)                                                 # Make sure X gain is 10.0
            self.dsp.gain(1, 3)                                                 # Make sure Y gain is 10.0
            self.dsp.digital_o(4, 1)                                            # Make sure Zouter is in coarse mode
                
            # Set up other digital
            self.dsp.digital_o(0, 0)                                            # Make sure bias dither is off
            self.dsp.digital_o(1, 0)                                            # Make sure Z dither is off
            self.dsp.digital_o(2, 1)                                            # Make sure feedback is on
            
            # Special configuration
            if flag:
                self.dsp.digital_o(3, 0)                                        # Make sure retract is off
                self.dsp.digital_o(5, 0)                                        # Set up in rotation mode
                self.init_dock()                                                # Reload all 3 dock view
                self.enable_mode_serial(True)                                   # Enable serial based on current mode
                self.idling = True                                              # Toggle idling flag back
                        
    # Giant step execution
    def giant_excu(self, channel, x, z, delay, g, stepnum):
        if self.tipappr.idling:
            self.enable_mode_serial(False)                                      # Disable all serial realted features
            self.tipappr.idling = False                                         # Toggle idling flag
            self.tipappr.pushButton_Stop.setEnabled(True)                       # Enable stop push button
            gx = g                                                              # Caluculate nonlinear coefficient for x channel
            gz = g                                                              # Caluculate nonlinear coefficient for z channel
            
            self.setup_tipappr(False)                                           # Set up tip approach
            self.dsp.digital_o(3, 1)                                            # Retract tip
            self.init_dock()                                                    # Reload all 3 dock view
            time.sleep(1)                                                       # Wait 1s to let feedback loop respond
            self.dsp.giantStep(channel, x, z, 100, delay, gx, gz, stepnum)      # Execute giant stpes
            self.dsp.digital_o(3, 0)                                            # Unretract tip
            time.sleep(5)                                                       # Wait 5s to let feedback loop respond
            self.init_dock()                                                    # Reload all 3 dock view
            
            self.tipappr.idling = True                                          # Toggle back idling flag
            self.tipappr.label_Status_Status.setText("Idling")                  # Set the status label back to idling
            self.enable_mode_serial(True)                                       # Enable all serial related features
            self.tipappr.set_translation(self.dsp.lastdigital[5])               # Set translation

    # Giant step slot
    def giant_slot(self, channel, x, z, delay, g, stepnum):
        threading.Thread(target = (lambda: self.giant_excu(channel, x, z, delay, g, stepnum))).start()
        
    # Tip approach execution
    def tip_appr_excu(self, x, z, delay, g, giant, baby, minCurr):
        if self.tipappr.idling:
            self.enable_mode_serial(False)
            self.enable_mode_serial(False)                                      # Disable all serial realted features
            self.tipappr.idling = False                                         # Toggle idling flag
            self.tipappr.pushButton_Stop.setEnabled(True)                       # Enable stop push button
            gx = g                                                              # Caluculate nonlinear coefficient for x channel
            gz = g                                                              # Caluculate nonlinear coefficient for z channel
            
            self.setup_tipappr(False)                                           # Set up tip approach
            self.digital_o(5, 0)                                                # Set up in rotation mode
            self.dsp.digital_o(3, 1)                                            # Retract tip
            self.init_dock()                                                    # Reload all 3 dock view
            time.sleep(1)                                                       # Wait 1s to let feedback loop respond
            self. dsp.tipApproach(x, z, 100, delay, gx, gz, giant, baby, gz, minCurr)      # Execute Tip approach
            self.dsp.digital_o(3, 0)                                            # Unretract tip
            time.sleep(5)                                                       # Wait 5s to let feedback loop respond
            self.init_dock()                                                    # Reload all 3 dock view
            
            self.tipappr.idling = True                                          # Toggle back idling flag
            self.tipappr.label_Status_Status.setText("Idling")                  # Set the status label back to idling
            self.enable_mode_serial(True)                                       # Enable all serial related features
    
    # Tip approach slot
    def tip_appr_slot(self, x, z, delay, g, giant, baby, minCurr):
        threading.Thread(target = (lambda: self.tip_appr_excu(x, z, delay, g, giant, baby, minCurr))).start()
        
    # Update stepnumber
    def giantStep_update(self, i):
        self.tipappr.label_Pass.setText(str(i))       # Update pass number label
        

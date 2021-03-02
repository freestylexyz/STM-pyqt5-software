# -*- coding: utf-8 -*-
"""
Created on Sun Jan  3 15:55:44 2021

@author: yaoji
"""
from datetime import datetime
from DigitalSignalProcessor import myDSP
from sequence import mySequence
import numpy as np

# General data class
class STMData():
    def __init__(self):
        self.time = datetime.now()      # Current time
        self.seq = mySequence()         # Sequence
        self.path = ''                  # Saved path
        self.data = np.array([])        # Data
        self.bias_dac = False           # Bias DAC selection, False -> 16bit DAC, True -> 20bitDAC
        self.preamp_gain = 9            # Pre-amp gain
        self.lockin_flag = False        # Lock

        self.lastdac = [0x8000] * 16    # Last ouput of all DAC channels
        self.dacrange = [10] * 16       # All DAC channels' current range
        self.adcrange = [0] * 8         # All ADC channels' current range
        self.last20bit = 0x80000        # Last ouput of 20bit DAC
        self.lastdigital = [False] * 6  # 0 - 5 : bias dither, z dither, feedback, retract, coarse, translation
        self.lastgain = [1] * 4         # 0 -> gain 10.0, 1 -> gain 1.0, 3 -> gain 0.1
                                        # Z1 gain is different from others, 3 -> gain 10.0, 1 -> gain 1.0, 0 -> gain 0.1
        self.offset = [0] * 16          # 0 - 14 are bias offset for different range, 15 is Iset offset
        
    # Load system status and sequence
    def load_status(self, dsp, preamp_gain, bias_dac, seq):
        self.seq = seq
        self.bias_dac = bias_dac
        self.preamp_gain = preamp_gain
        
        self.lastdac = dsp.lastdac
        self.dacrange = dsp.dacrange
        self.adcrange = dsp.adcrange
        self.last20bit = dsp.last20bit
        self.lastdigital = dsp.lastdigital
        self.lastgain = dsp.lastgain
        self.offset = dsp.offset
    
    # Load lock in parameters
    # params is a list of parameters, osc_type is string
    def load_lockin(self, params, osc_type):
        self.ocs_type = osc_type
        self.osc_rms = params[0]
        self.osc_freq = params[1]
        self.lockin_freq = params[2]
        self.phase1 = params[3]
        self.phase2 = params[4]
        self.sen1 = params[5]
        self.sen2 = params[6]
        self.offset1 = params[7]
        self.offset2 = params[8]
        self.tc1 = params[9]
        self.tc2 = params[10]

# Depostion data class
class DepData(STMData):
    def __init__(self):
        super().__init__()
        self.read_ch = 0xdc     # Read channel
        self.read_mode = 0      # Reading mode: 0 - > Do nothing; 1 -> Continuous read; 
                                # 2 -> N samples read until total amount of change detected
                                # 3 -> N smples reading
        self.read_delay = 0     # Delay waited before executing reading procedure
        self.read_delay2 = 0    # Delay between 2 neighbouring sampling
        self.read_num = 0       # Total number of samples for N sampes reading
        self.read_avg = 0       # Average number for each sampling
        self.read_change = 0    # Total amount of change to stop reading for reading mode 2
                                # This is also used for reading mode 1, but stop signal is sent by PC
                                # In reading mode 2 DSP decide stop by itself
        self.read_stop_num = 0  # Number of reading to be performed after total amount of change is detected
                                # This is used in both reading mode 2 and 3
        
    # Load options for deposition
    # read is a list of depostion option
    def load(self, read):
        self.read_ch = read[0]
        self.read_mode = read[1]
        self.read_delay = read[2]
        self.read_delay2 = read[3]
        self.read_num = read[4]
        self.read_avg = read[5]
        self.read_change = read[6]
        self.read_stop_num = read[7]

# Scan data class
class ScanData(STMData):
    def __init__(self):
        super().__init__()
        self.point = 0      # Point index variable for updateing data 
        self.line = 0       # Line index variable for updating data
        self.minimum = 0    # Minimum variable for updating data
        
        self.step_num, self.step_size = 127, 1                              # Pixel number per line, pixel size
        self.channel_x, self.channel_y, self.dir_x = 0x1f, 0x10, True       # Point ramp channel, line ramp channel, point ramp direction
        self.move_delay, self.measure_delay, self.line_delay = 0, 0, 0      # Delay waited before moving 1 step, delay watied before read sequence
                                                                            # Delay waited before starting each line
        self.scan_protect_flag, self.limit = False, 0                       # Scan protect flag: 0 -> No protection
                                                                            # 1 -> Stop scan
                                                                            # 2 -> Adjust Z feedback to zero
                                                                            # 3 -> Adjust Z feedback to the previous middle point
                                                                            # Z feedback limit to trigger scan protection
        self.tip_protection, self.tip_protect_data = False, 0               # Tip protection flag (True / False), retracted Z offset fine data
        self.match_curr, self.advance_bit = False, 0                        # Match current option before scan (feedback off only)
                                                                            # Advanced Z offset fine after feedback off (before scan)
        
    # Load options for scan
    def load(self, step_num, step_size, channel_x, channel_y, dir_x, move_delay, measure_delay, line_delay, \
             scan_protect_flag, limit, tip_protection, tip_protect_data, match_curr, advance_bit):
        self.step_num, self.step_size = step_num, step_size
        self.channel_x, self.channel_y, self.dir_x = channel_x, channel_y, dir_x
        self.move_delay, self.measure_delay, self.line_delay = move_delay, measure_delay, line_delay
        self.scan_protect_flag, self.limit = scan_protect_flag, limit
        self.tip_protection, self.tip_protect_data = tip_protection, tip_protect_data
        self.match_curr, self.advance_bit = match_curr, advance_bit
        
        # Need to load_status before load options to configure sequence read number
        self.data = -np.ones((self.seq.read_num, step_num, step_num))   # Init a space for image data (-1 stdands for not read yet)
        self.point = 0 if self.dir_x else (self.step_num - 1)           # Init point index based on scan direction 
        self.line = 0                                                   # Init line index
        
    # Update scan data and return data for display
    def update_data(self, rdata):
        # axis 0 is image index
        # axis 1 is x pixel number
        # axis 2 is y pixel number
        
        # Start a new line if point index out of range
        if (self.point >= self.step_num) or (self.point <= -1):
            self.point = 0 if self.dir_x else (self.step_num - 1)
            self.line += 1
        
        # Write pixel data into data space, based on point channel
        if self.channel_x == 0x1f:
            self.data[:, self.line, self.point] = np.array(rdata)
        else:
            self.data[:, self.point, self.line] = np.array(rdata)
            
        self.point = self.point + 1 if self.dir_x else self.point - 1   # Increment or decrement point index based on scan direction
        self.minimum = min(self.minimum, rdata[0])                      # Determine minimum data for display
        return np.maximum(self.data[0, :, :], self.minimum)             # Replace all -1 with minimum for display
        
# Spectroscopy data class
class SpcData(STMData):
    def __init__(self):
        super().__init__()
        self.data_ = np.array([])       # Current pass spectroscopy storage space
        self.num_pt = 0                 # Physical point index varaiable for updating data
        self.data_pt = 0                # Data point index variable for updating data
        
        self.start, self.step, self.data_num = 0, 1, 1  # Start, step, number of data point for each spectroscopy
        self.pass_num = 1                               # Total number of passes
        self.ramp_ch, self.delta_data = 0, 0            # Ramp channel address, delta data
        self.move_delay, self.read_delay = 0, 0         # Delay waited before moving 1 step, delay watied before read sequence
        self.scan_dir = 2                               # Scan direction flag: 0 -> Read forward only
                                                        # 1 -> Read backward only
                                                        # 2 -> Read both forward and backward but don't average
                                                        # 3 -> Average both forward and backward
        self.corr_pass_num = 1                          # Number of passes before doing one correction
        self.feedback_delay = 0                         # Feedback on delay for Z correction
        self.z_flag = False                             # If do Z correction (feedback on)
        self.match_flag = False                         # If do current matching for Z correction
        self.track_flag = False                         # If do XY correction (tracking) 
        
        self.rescan = 0xffffffff                        # Number of passes before doing one re-scan
        self.pattern = [0, 0, 1]                        # Match pattern [pattern center x, pattern center y, pattern size]
        self.point_list = [[0, 0]]                      # Physical point list
        
        self.pre_scan = ScanData()                       # Prescan data
        
    # Load options for spectroscopy    
    def load(self, start, step, data_num, ramp_ch, delta_data, move_delay, measure_delay,\
             forward, backward, average, corr_pass_num, z_flag, match_flag, feedback_delay, track_flag,\
                 rescan, pre_scan, point_list, pattern):
        self.start, self.step, self.data_num = start, step, data_num
        self.ramp_ch, self.delta_data = ramp_ch, delta_data
        self.move_delay, self.read_delay = move_delay, measure_delay
        
        self.corr_pass_num = corr_pass_num
        self.feedback_delay = feedback_delay
        self.z_flag, self.match_flag, self.track_flag = z_flag, match_flag, track_flag
        
        self.rescan, self.pre_scan, self.point_list, self.pattern = rescan, pre_scan, point_list, pattern
        
        # Configure scan_dir flag
        if forward and (not backward):
            self.scan_dir = 0
        elif backward and (not forward):
            self.scan_dir = 1
        elif backward and forward and (not average):
            self.scan_dir = 2
        else:
            self.scan_dir = 3
        
        # Init a space for current pass data
        self.data_ = -np.ones((len(self.point_list), self.seq.read_num + 1, self.data_num * (1 + (self.scan_dir > 1))))
        self.num_pt = 0     # Init physical point index
        self.data_pt = 0    # Init data point index
        self.pass_num = 0   # Pass number need to initialized to 0 before start

    # Update current pass data
    # Return forward data and backward data of current pass and current physical point for display
    def update_data(self, rdata):
        # axis 0 is point index
        # axis 1 is read channel
        # axis 2 is data num
        
        # Start a physical point if point index out of range
        if self.data_pt >= (self.data_num * (1 + (self.scan_dir > 1))):
            self.data_pt = 0
            self.num_pt += 1
            
        self.data_[self.num_pt, :, self.data_pt] = np.array(rdata)      # Write point data to current pass spectroscopy space
        self.data_pt += 1                                               # Increment of data point index
        
        # Determin forward data and backward data of current pass and current physical point based on scan direction
        if self.scan_dir == 0:      # Forward only: all data is forward data, backward data is empty
            f = self.data_[self.num_pt, :, :self.data_pt].reshape(self.seq.read_num + 1, self.data_pt)
            b = np.array([]).reshape(self.seq.read_num + 1, self.data_pt)       # Empty also need to match dimensons
        elif self.scan_dir == 1:    # Backward only: all data is backward data, forward data is empty
            f = np.array([]).reshape(self.seq.read_num + 1, self.data_pt)       # Empty also need to match dimensons
            b = self.data_[self.num_pt, :, :self.data_pt].reshape(self.seq.read_num + 1, self.data_pt)
            b = np.flip(b, 2)                                                   # Flip backward data order
        else:                       # Both of the left mode will keep both forward and backward data for current pass
            f_num = min(self.data_pt, self.data_num)                            # Determine forward data already measured
            f = self.data_[self.num_pt, :, :f_num].reshape(self.seq.read_num + 1, f_num)
            b_num = max(self.data_pt, self.data_num)                            # Determine backward data already measured
            b = self.data_[self.num_pt, :, self.data_num:b_num].reshape(self.seq.read_num + 1, b_num - self.data_num)
            b = np.flip(b, axis = 2)
            
        return f, b

    # Combine current data and average with existing data
    def combine_data(self):
        pt_num = len(self.point_list)   # Total number of physical points
        
        # Wait until all data has been updated
        while(True):
            a = self.num_pt == (pt_num - 1)                                     # All physical points have been measured
            b = self.data_pt == (self.data_num * (1 + (self.scan_dir > 1)))     # All data point have been measured
            if a and b:
                break
            
        if self.scan_dir == 0:      # Read forward only
            pass                                        # No action needed
        elif self.scan_dir == 1:    # Read backward only
            self.data_ = np.flip(self.data_, 2)         # Reverse data order
        else:
            x = self.data_[:, 0, :self.data_num].reshape(pt_num, 1, self.data_num)                      # Ramp value
            f = self.data_[:, 1: , :self.data_num].reshape(pt_num, self.seq.read_num, self.data_num)    # Forward pass
            b = self.data_[:, 1: , self.data_num:].reshape(pt_num, self.seq.read_num, self.data_num)    # Backward pass
            b = np.flip(b, axis = 2)                                                                    # Flip backward pass order
            if self.scan_dir == 2:  # Keep both forward and backward
                self.data_ = np.append(np.append(x, f, axis = 1), b, axis = 1)        
            else:                   # Average forward and backward
                self.data_ = np.append(x, (f + b) / 2, axis = 1)                      
    
    # Average with existing data
    def avg_data(self):
        # Average current passes with previous passes
        if self.pass_num == 0:
            self.data = self.data_ 
        else:
            self.data = ((self.data * self.pass_num) + self.data_) / (self.pass_num + 1)
        # Pass number increment
        self.pass_num += 1
        # New pass
        self.data_ = -np.ones((len(self.point_list), self.seq.read_num + 1, self.data_num * (1 + (self.scan_dir > 1))))
        self.num_pt = 0
        self.data_pt = 0 


            


        
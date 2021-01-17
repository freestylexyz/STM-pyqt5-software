# -*- coding: utf-8 -*-
"""
Created on Sun Jan  3 15:55:44 2021

@author: yaoji
"""
from datetime import datetime
from DigitalSignalProcessor import myDSP
from sequence import mySequence
import numpy as np

class STMData():
    def __init__(self):
        self.time = datetime.now()
        self.seq = mySequence()
        self.path = ''
        self.data = np.array([])
        self.bias_dac = False
        self.preamp_gain = 9
        
        self.lastdac = [0x8000] * 16    # Last ouput of all DAC channels
        self.dacrange = [10] * 16       # All DAC channels' current range
        self.adcrange = [0] * 8         # All ADC channels' current range
        self.last20bit = 0x80000        # Last ouput of 20bit DAC
        self.lastdigital = [False] * 6  # 0 - 5 : bias dither, z dither, feedback, retract, coarse, translation
        self.lastgain = [1] * 4         # 0 -> gain 10.0, 1 -> gain 1.0, 3 -> gain 0.1
                                        # Z1 gain is different from others, 3 -> gain 10.0, 1 -> gain 1.0, 0 -> gain 0.1
        self.offset = [0] * 16          # 0 - 14 are bias offset for different range, 15 is Iset offset
        
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
    
    def load_lockin(self, params, osc_type):
        self.ocs_type = osc_type
        self.osc_rms = self.params[0]
        self.osc_freq = self.params[1]
        self.lockin_freq = self.params[2]
        self.phase1 = self.params[3]
        self.phase2 = self.params[4]
        self.sen1 = self.params[5]
        self.sen2 = self.params[6]
        self.offset1 = self.params[7]
        self.offset2 = self.params[8]
        self.tc1 = self.params[9]
        self.tc2 = self.params[10]

class DepData(STMData):
    def __init__(self):
        super().__init__()
        self.read_ch = 0xdc
        self.read_mode = 0
        self.read_delay = 0
        self.read_delay2 = 0
        self.read_num = 0
        self.read_avg = 0
        self.read_change = 0
        self.read_stop_num = 0
        
    def load(self, read):
        self.read_ch = read[0]
        self.read_mode = read[1]
        self.read_delay = read[2]
        self.read_delay2 = read[3]
        self.read_num = read[4]
        self.read_avg = read[5]
        self.read_change = read[6]
        self.read_stop_num = read[7]
        
class ScanData(STMData):
    def __init__(self):
        super().__init__()
        self.point = 0
        self.line = 0
        self.step_num, self.step_size = 128, 1
        self.channel_x, self.channel_y, self.dir_x = 0x1f, 0x10, True
        self.move_delay, self.measure_delay, self.line_delay = 0, 0, 0
        self.scan_protect_flag, self.limit = False, 0
        self.tip_protection, self.tip_protect_data = False, 0
        self.match_curr, self.advance_bit = False, 0
        
    def load(self, step_num, step_size, channel_x, channel_y, dir_x, move_delay, measure_delay, line_delay, \
             scan_protect_flag, limit, tip_protection, tip_protect_data, match_curr, advance_bit):
        self.step_num, self.step_size = step_num, step_size
        self.channel_x, self.channel_y, self.dir_x = channel_x, channel_y, dir_x
        self.move_delay, self.measure_delay, self.line_delay = move_delay, measure_delay, line_delay
        self.scan_protect_flag, self.limit = scan_protect_flag, limit
        self.tip_protection, self.tip_protect_data = tip_protection, tip_protect_data
        self.match_curr, self.advance_bit = match_curr, advance_bit
        # Need to load_status before load options
        self.data = np.zeros((self.seq.read_num, step_num, step_num))
        self.point = 0 if self.dir_x else (self.step_num - 1)
        self.line = 0
        
    def update_data(self, rdata):
        # axis 0 is image index
        # axis 1 is x pixel number
        # axis 2 is y pixel number
        if (self.point >= self.step_num) or (self.point <= -1):
            self.point = 0 if self.dir_x else (self.step_num - 1)
            self.line += 1
        if self.channel_x == 0x1f:
            self.data[:, self.line, self.point] = np.array(rdata)
        else:
            self.data[:, self.point, self.line] = np.array(rdata)
            
        self.point = self.point + 1 if self.dir_x else self.point - 1
        
class SpcData(STMData):
    def __init__(self):
        super().__init__()
        # self.fdata = np.array([])
        # self.bdata = np.array([])
        self.num_pt = 0
        self.data_pt = 0
        self.dir_flag = True
        
        self.start, self.step, self.data_num, self.pass_num = 0, 1, 1, 1
        self.ramp_ch, self.delta_data = 0, 0
        self.move_dealy, self.read_dealy = 0, 0
        self.scan_dir = 2
        
        self.spec_num = 6

        self.corr_pass_num = 1
        self.feedback_delay = 0
        self.z_flag, self.match_flag, self.track_flag = False, False, False
        
        self.rescan = 0xffffffff
        self.pattern = [0, 0, 1]
        self.point_list = [[0, 0]]
        
        self.prescan = ScanData()
        
    def load(self, start, step, data_num, pass_num, ramp_ch, delta_data, move_delay, measure_delay,\
             forward, backward, average, corr_pass_num, z_flag, match_flag, feedback_delay, track_flag,\
                 rescan, pre_scan, point_list, pattern):
        self.start, self.step, self.data_num, self.pass_num = start, step, data_num, pass_num
        self.ramp_ch, self.delta_data = ramp_ch, delta_data
        self.move_dealy, self.read_dealy = move_delay, measure_delay
        
        self.data_pt = 0
        if forward and (not backward):
            self.scan_dir = 0
        elif backward and (not forward):
            self.scan_dir = 1
            self.data_pt = self.data_num - 1
        # data_ ignore average, always save both passes
        elif backward and forward and (not average):
            self.scan_dir = 2
        else:
            self.scan_dir = 3
        self.spec_num = self.seq.read_num * (1 + (self.scan_dir == 2))
        
        self.corr_pass_num = corr_pass_num
        self.feedback_delay = feedback_delay
        self.z_flag, self.match_flag, self.track_flag = z_flag, match_flag, track_flag
        
        self.rescan, self.pre_scan, self.point_list, self.pattern = rescan, pre_scan, point_list, pattern
        
        self.data = np.zeros((len(self.point_list), self.spec_num, self.data_num))
        self.num_pt = 0

        
    # def update_data_(self, rdata):
    #     # axis 0 is point index
    #     # axis 1 is data num
    #     # axis 2 is read channel
    #     se
    #     if (self.data_pt >= self.data_num) or (self.data_pt <= -1):
    #         self.point = 0 if self.dir_x else (self.step_num - 1)
    #         self.line += 1
    #     if self.channel_x == 0x1f:
    #         self.data[:, self.line, self.point] = np.array(rdata)
    #     else:
    #         self.data[:, self.point, self.line] = np.array(rdata)
            
    #     self.point = self.point + 1 if self.dir_x else self.point - 1

        
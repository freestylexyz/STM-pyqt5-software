# -*- coding: utf-8 -*-
"""
@Date     : 2021/3/24 10:14:36
@Author   : milier00
@FileName : fake_spc_data.py
"""
import sys
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication , QWidget
import cv2 as cv
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image
from matplotlib import cm
from PIL import Image, ImageDraw
from numpy import genfromtxt
import pandas as pd
from DataStruct import *
import pickle
import copy

''' Make fake data1: multi-plot '''
x1 = np.linspace(-100, 100, 10)
data1 = np.sin(x1) / x1
data2 = np.cos(x1) / x1

pt1 = np.vstack((x1, data1))
pt1 = pt1.transpose()
pt1 = pt1.reshape((1, 10, 2))

pt2 = np.vstack((x1, data2))
pt2 = pt2.transpose()
pt2 = pt2.reshape((1, 10, 2))

demo1 = np.vstack((pt1, pt2))

''' Make fake data2: single-plot '''
x2 = np.linspace(-100, 100, 10)
data3 = np.random.normal(size=10)/100
pt3 = np.vstack((x2, data3))
pt3 = pt3.transpose()
pt3 = pt3.reshape((1, 10, 2))
demo2 = pt3
print(pt3)

''' Preview data in plot '''
fig1 = plt.figure()
ax1 = fig1.add_subplot(211)
ax1.plot(demo1[0, :, 0], demo1[0, :, 1])
ax1.plot(demo1[1, :, 0], demo1[1, :, 1])
ax2 = fig1.add_subplot(212)
ax2.plot(demo2[0, :, 0], demo2[0, :, 1])
plt.show()

'''
# Load options for scan
def load(self, step_num, step_size, channel_x, channel_y, dir_x, move_delay, measure_delay, line_delay, \
         scan_protect_flag, limit, tip_protection, tip_protect_data, match_curr, advance_bit):

# Load options for spectroscopy    
def load(self, start, step, data_num, ramp_ch, delta_data, move_delay, measure_delay, \
         forward, backward, average, corr_pass_num, z_flag, match_flag, feedback_delay, track_flag, \
         rescan, pre_scan, point_list, pattern):
'''

''' Make fake data sequence '''
spc_data = SpcData()
# # spc_data.load(0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, False, False, 0, False, 0 ,)
spc_data.data_ = demo2
# !!! set up sequence params
fname = '../Scan/seq data/seq_1.seq'
if fname != '':
    with open(fname, 'rb') as input:
        spc_data.seq = pickle.load(input)
spc_data.seq.name = 'Fake data Sequence'
#
# print(spc_data.seq.name, spc_data.seq.path)
# print(spc_data.seq.command_list, spc_data.seq.data_list)
# print(spc_data.seq.command)
# print(spc_data.seq.channel)
# print(spc_data.seq.option1)
# print(spc_data.seq.option2)
# print(spc_data.seq.data)
#

''' Save fake data '''
fname = '../data/03242101.spc'
if fname != '':
    with open(fname, 'wb') as output:
        pickle.dump(spc_data, output, pickle.HIGHEST_PROTOCOL)

''' Open fake data '''
with open('../data/03242101.spc', 'rb') as input:
    spc_data = pickle.load(input)
print(spc_data.data_.shape[:])
# -*- coding: utf-8 -*-
"""
@Date     : 2021/3/24 10:15:37
@Author   : milier00
@FileName : fake_dep_data.py
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

''' Make fake data '''
x = np.linspace(-100, 100, 100)
data1 = np.random.normal(size=100)/100
data2 = np.cos(x) / x
data3 = np.sin(x/5) / x

dep_data = DepData()
# scan_data.load(127, 256, 0x1f, 0x10, True, 0, 0, 0, False, 0, False, 0, False, 0)
dep_data.data = data1

''' Preview data in plot '''
fig1 = plt.figure()
ax1 = fig1.add_subplot(311)
ax1.plot(x, data1)
ax1 = fig1.add_subplot(312)
ax1.plot(x, data2)
ax1 = fig1.add_subplot(313)
ax1.plot(x, data3)
plt.show()

''' Make fake data sequence '''
fname = '../Scan/seq data/seq_1.seq'
if fname != '':
    with open(fname, 'rb') as input:
        dep_data.seq = pickle.load(input)
dep_data.seq.name = 'Fake data Sequence'

print(dep_data.seq.name, dep_data.seq.path)
print(dep_data.seq.command_list, dep_data.seq.data_list)
print(dep_data.seq.command)
print(dep_data.seq.channel)
print(dep_data.seq.option1)
print(dep_data.seq.option2)
print(dep_data.seq.data)

''' Save fake data '''
fname = './test data/03252100.dep'
if fname != '':
    with open(fname, 'wb') as output:
        pickle.dump(dep_data, output, pickle.HIGHEST_PROTOCOL)

''' Open fake data '''
with open('./test data/03252100.dep', 'rb') as input:
    spc_data = pickle.load(input)
print(spc_data.data.shape[:])
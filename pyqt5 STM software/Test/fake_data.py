# -*- coding: utf-8 -*-
"""
@Date     : 2021/2/3 06:08:17
@Author   : milier00
@FileName : fake_data.py
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

''' Read original data and resize '''
g = open('../data/real_stm_img.csv', 'r')
temp = genfromtxt(g, delimiter=',')
im = Image.fromarray(temp).convert('RGB')
pix = im.load()
rows, cols = im.size
for x in range(cols):
    for y in range(rows):
        print
        str(x) + " " + str(y)
        pix[x, y] = (int(temp[y, x] // 256 // 256 % 256), int(temp[y, x] // 256 % 256), int(temp[y, x] % 256))
im.save(g.name[0:-4] + '.jpeg')
img = cv.imread(g.name[0:-4] + '.jpeg')
img = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
img = np.reshape(img, (1, rows, cols))


''' Make fake data '''
scan_data = ScanData()
scan_data.load(127, 256, 0x1f, 0x10, True, 0, 0, 0, False, 0, False, 0, False, 0)
scan_data.data = img
# !!! set up sequence params


fname = '../Scan/seq data/seq_1.seq'
if fname != '':
    with open(fname, 'rb') as input:
        scan_data.seq = pickle.load(input)
scan_data.seq.name = 'Fake data Sequence'

print(scan_data.seq.name,scan_data.seq.path)
print(scan_data.seq.command_list,scan_data.seq.data_list)
print(scan_data.seq.command)
print(scan_data.seq.channel)
print(scan_data.seq.option1)
print(scan_data.seq.option2)
print(scan_data.seq.data)

fname = '../data/02012100.stm'
if fname != '':
    with open(fname, 'wb') as output:
        pickle.dump(scan_data, output, pickle.HIGHEST_PROTOCOL)


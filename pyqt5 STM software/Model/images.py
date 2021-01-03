# -*- coding: utf-8 -*-
"""
@Date     : 2021/1/3 22:22:05
@Author   : milier00
@FileName : images.py
"""
import numpy as np
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

class myImages(QWidget):

    def __init__(self):
        super().__init__()

    #
    # color -> gray
    #
    def color2gray(self, img):
        # img = cv.imread(path)
        # img = cv.resize(img, dsize=(440, 440))
        # cv.imshow('read_img',img)
        gray_img = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
        # cv.imshow('gray_img', gray_img)
        psudo_gray_img = cv.cvtColor(gray_img, cv.COLOR_GRAY2BGR)
        # cv.imshow('psudo_gray_img', psudo_gray_img)
        # cv.imwrite('..\data\scan_example_gray.jpg', psudo_gray_img)
        # cv.waitKey(0)
        # cv.destroyAllWindows()
        return psudo_gray_img

    #
    # gray -> color
    #
    def gey2color(self, path):
        img = cv.imread(path)
        img = cv.resize(img, dsize=(440, 440))
        # cv.imshow('read_img',img)
        # img = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
        # img=cv2.cvtColor(img,cv2.COLOR_GRAY2BGR)
        img = cv.applyColorMap(img, 5)
        # cv.imshow('color_img', img)
        # cv.waitKey(0)
        # cv.destroyAllWindows()
        return img



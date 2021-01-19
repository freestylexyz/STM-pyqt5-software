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
import numpy as np
import matplotlib.pyplot as plt
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
    def gray2color(self, path):
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
    #
    # gray -> reversed
    #
    def gray2reverse(self, path):
        img = cv.imread(path, 0)  #读取一张图片，灰度
        height, width = img.shape
        dst = np.zeros((height, width, 1), np.uint8)
        for i in range(height):
            for j in range(width):
                dst[i, j] = 255-img[i,j]
        cv.imwrite('..\data\scan_example_reverse.png', dst)
        # cv.imshow('img',img)
        # cv.imshow('dst',dst)
        # cv.waitKey()
        return dst

    #
    # color -> reversed
    #
    def color2reverse(self, path):
        img = cv.imread(path,1)  #读取一张图片，彩色
        cha = img.shape
        height, width, deep = cha
        dst = np.zeros((height, width, 3), np.uint8)
        for i in range(height):  #色彩反转
            for j in range(width):
                b, g, r = img[i, j]
                dst[i, j] = (255-b, 255-g, 255-r)
        # cv.imshow('img', img)
        # cv.imshow('dst', dst)
        # cv.waitKey()
        return dst
    #
    # gray --> scharr operator --> diff
    #
    def illuminated(self, path):
        img = cv.imread(path)
        # scharr operator
        scharrx = cv.Scharr(img, cv.CV_64F, 1, 0)
        scharry = cv.Scharr(img, cv.CV_64F, 0, 1)
        scharrx = cv.convertScaleAbs(scharrx)
        scharry = cv.convertScaleAbs(scharry)
        scharrxy = cv.addWeighted(scharrx, 0.5, scharry, 0.5, 0)
        cv.imwrite('..\data\scan_example_illuminated.png', scharrxy)
        # res = np.hstack((img, scharrx, scharry, scharrxy))
        # cv.imshow(res, 'res')
        # cv.waitKey()
        # cv.destroyAllWindows()
        return scharrxy

    #
    # plane fit
    #
    def plane_fit(self, path):
        # path = "../data/scan_example.png"
        img = cv.imread(path)
        img = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
        # img = cv.resize(img, dsize=(128, 128))
        x = np.array([[i + 1] * img.shape[0] for i in range(img.shape[0])]).flatten()
        y = np.array(list(np.arange(img.shape[1]) + 1) * img.shape[1])
        z = img.flatten()
        index = np.argmin(z)

        # 创建系数矩阵A
        A = np.zeros((3, 3))
        for i in range(0, img.size):
            A[0, 0] = A[0, 0] + x[i] ** 2
            A[0, 1] = A[0, 1] + x[i] * y[i]
            A[0, 2] = A[0, 2] + x[i]
            A[1, 0] = A[0, 1]
            A[1, 1] = A[1, 1] + y[i] ** 2
            A[1, 2] = A[1, 2] + y[i]
            A[2, 0] = A[0, 2]
            A[2, 1] = A[1, 2]
            A[2, 2] = img.size

        # 创建b
        b = np.zeros((3, 1))
        for i in range(0, img.size):
            b[0, 0] = b[0, 0] + x[i] * z[i]
            b[1, 0] = b[1, 0] + y[i] * z[i]
            b[2, 0] = b[2, 0] + z[i]
        # print(b)

        # 求解X
        A_inv = np.linalg.inv(A)
        X = np.dot(A_inv, b)
        print('平面拟合结果为：z = %.3f * x + %.3f * y + %.3f' % (X[0, 0], X[1, 0], X[2, 0]))

        # 计算方差
        R = 0
        for i in range(0, img.size):
            R = R + (X[0, 0] * x[i] + X[1, 0] * y[i] + X[2, 0] - z[i]) ** 2
        print('方差为：%.*f' % (3, R))

        # 展示图像
        fig1 = plt.figure()
        ax1 = fig1.add_subplot(111, projection='3d')
        ax1.set_xlabel("x")
        ax1.set_ylabel("y")
        ax1.set_zlabel("z")
        ax1.scatter(x, y, z, c='r', marker='.')
        ax1.scatter(3, 6, 2, c='k', marker='^')
        x_p = np.linspace(1, img.shape[0]+1, 100)
        y_p = np.linspace(1, img.shape[1]+1, 100)
        x_p, y_p = np.meshgrid(x_p, y_p)
        # X[2, 0] = z[index] - X[0, 0] * x[index] - X[1, 0] * y[index]
        z_p = X[0, 0] * x_p + X[1, 0] * y_p + X[2, 0]
        new_z = X[0, 0] * x + X[1, 0] * y + X[2, 0]
        new_z = z - new_z
        new_z = np.reshape(new_z, (img.shape[0], img.shape[1]))
        ax1.plot_wireframe(x_p, y_p, z_p, rstride=10, cstride=10)
        ax1.scatter(x, y, new_z, c='g', marker='.')
        plt.show()
        cv.imwrite('..\data\scan_example_planefit.png', new_z)
        return new_z


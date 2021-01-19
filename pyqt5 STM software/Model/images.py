# -*- coding: utf-8 -*-
"""
@Date     : 2021/1/9 19:59:25
@Author   : milier00
@FileName : image.py
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
        # cv.imwrite('..\data\scan_example_reverse.png', dst)
        # cv.imshow('img',img)
        # cv.imshow('dst',dst)
        # cv.waitKey()
        return dst

    #
    # color -> reversed
    #
    def color2reverse(self, path):
        img = cv.imread(path, 1)  #读取一张图片，彩色
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
        # cv.imwrite('..\data\scan_example_illuminated.png', scharrxy)
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
        # cv.imwrite('..\data\scan_example_planefit.png', new_z)
        return new_z

    def prepare_data(self, array):
        xmax = max(map(max, array))
        xmin = min(map(min, array))
        # print(xmax, xmin)
        if xmax-xmin != 0:
            for i in range(array.shape[0]):
                for j in range(array.shape[1]):
                    array[i][j] = (array[i][j] - xmin) / (xmax - xmin)
        # print(array)
        return array

    def color_map(self, img, index):
        # img = cv.imread(path, 0)
        # img = cv.cvtColor(img, cv.COLOR_GRAY2BGR)
        # if index >= 0 and index <= 11:
        #     img = cv.applyColorMap(img, index)
        # elif index == 12:
        #     img = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
        #     img = cv.cvtColor(img, cv.COLOR_GRAY2BGR)
        # return img
        if index == 0:
            img = np.uint8(cm.viridis(img) * 255)
        elif index == 1:
            img = np.uint8(cm.plasma(img) * 255)
        elif index == 2:
            img = np.uint8(cm.inferno(img) * 255)
        elif index == 3:
            img = np.uint8(cm.magma(img) * 255)
        elif index == 4:
            img = np.uint8(cm.cividis(img) * 255)
        elif index == 5:
            img = np.uint8(cm.Greys(img) * 255)
        elif index == 6:
            img = np.uint8(cm.Purples(img) * 255)
        elif index == 7:
            img = np.uint8(cm.Blues(img) * 255)
        elif index == 8:
            img = np.uint8(cm.Greens(img) * 255)
        elif index == 9:
            img = np.uint8(cm.Oranges(img) * 255)
        elif index == 10:
            img = np.uint8(cm.Reds(img) * 255)
        elif index == 11:
            img = np.uint8(cm.YlOrBr(img) * 255)
        elif index == 12:
            img = np.uint8(cm.YlOrRd(img) * 255)
        elif index == 13:
            img = np.uint8(cm.OrRd(img) * 255)
        elif index == 14:
            img = np.uint8(cm.PuRd(img) * 255)
        elif index == 15:
            img = np.uint8(cm.RdPu(img) * 255)
        elif index == 16:
            img = np.uint8(cm.BuPu(img) * 255)
        elif index == 17:
            img = np.uint8(cm.GnBu(img) * 255)
        elif index == 18:
            img = np.uint8(cm.PuBu(img) * 255)
        elif index == 19:
            img = np.uint8(cm.YlGnBu(img) * 255)
        elif index == 20:
            img = np.uint8(cm.PuBuGn(img) * 255)
        elif index == 21:
            img = np.uint8(cm.BuGn(img) * 255)
        elif index == 22:
            img = np.uint8(cm.YlGn(img) * 255)
        elif index == 23:
            img = np.uint8(cm.binary(img) * 255)
        elif index == 24:
            img = np.uint8(cm.gist_yarg(img) * 255)
        elif index == 25:
            img = np.uint8(cm.gist_gray(img) * 255)
        elif index == 26:
            img = np.uint8(cm.gray(img) * 255)
        elif index == 27:
            img = np.uint8(cm.bone(img) * 255)
        elif index == 28:
            img = np.uint8(cm.pink(img) * 255)
        elif index == 29:
            img = np.uint8(cm.spring(img) * 255)
        elif index == 30:
            img = np.uint8(cm.summer(img) * 255)
        elif index == 31:
            img = np.uint8(cm.autumn(img) * 255)
        elif index == 32:
            img = np.uint8(cm.winter(img) * 255)
        elif index == 33:
            img = np.uint8(cm.cool(img) * 255)
        elif index == 34:
            img = np.uint8(cm.Wistia(img) * 255)
        elif index == 35:
            img = np.uint8(cm.hot(img) * 255)
        elif index == 36:
            img = np.uint8(cm.afmhot(img) * 255)
        elif index == 37:
            img = np.uint8(cm.gist_heat(img) * 255)
        elif index == 38:
            img = np.uint8(cm.copper(img) * 255)
        elif index == 39:
            img = np.uint8(cm.PiYG(img) * 255)
        elif index == 40:
            img = np.uint8(cm.PRGn(img) * 255)
        elif index == 41:
            img = np.uint8(cm.BrBG(img) * 255)
        elif index == 42:
            img = np.uint8(cm.PuOr(img) * 255)
        elif index == 43:
            img = np.uint8(cm.RdGy(img) * 255)
        elif index == 44:
            img = np.uint8(cm.RdBu(img) * 255)
        elif index == 45:
            img = np.uint8(cm.RdYlBu(img) * 255)
        elif index == 46:
            img = np.uint8(cm.RdYlGn(img) * 255)
        elif index == 47:
            img = np.uint8(cm.Spectral(img) * 255)
        elif index == 48:
            img = np.uint8(cm.coolwarm(img) * 255)
        elif index == 49:
            img = np.uint8(cm.bwr(img) * 255)
        elif index == 50:
            img = np.uint8(cm.seismic(img) * 255)
        elif index == 51:
            img = np.uint8(cm.twilight(img) * 255)
        elif index == 52:
            img = np.uint8(cm.twilight_shifted(img) * 255)
        elif index == 53:
            img = np.uint8(cm.hsv(img) * 255)
        elif index == 54:
            img = np.uint8(cm.Pastel1(img) * 255)
        elif index == 55:
            img = np.uint8(cm.Pastel2(img) * 255)
        elif index == 56:
            img = np.uint8(cm.Paired(img) * 255)
        elif index == 57:
            img = np.uint8(cm.Accent(img) * 255)
        elif index == 58:
            img = np.uint8(cm.Dark2(img) * 255)
        elif index == 59:
            img = np.uint8(cm.Set1(img) * 255)
        elif index == 60:
            img = np.uint8(cm.Set2(img) * 255)
        elif index == 61:
            img = np.uint8(cm.Set3(img) * 255)
        elif index == 62:
            img = np.uint8(cm.tab10(img) * 255)
        elif index == 63:
            img = np.uint8(cm.tab20(img) * 255)
        elif index == 64:
            img = np.uint8(cm.tab20b(img) * 255)
        elif index == 65:
            img = np.uint8(cm.tab20c(img) * 255)
        elif index == 66:
            img = np.uint8(cm.flag(img) * 255)
        elif index == 67:
            img = np.uint8(cm.prism(img) * 255)
        elif index == 68:
            img = np.uint8(cm.ocean(img) * 255)
        elif index == 69:
            img = np.uint8(cm.gist_earth(img) * 255)
        elif index == 70:
            img = np.uint8(cm.terrain(img) * 255)
        elif index == 71:
            img = np.uint8(cm.gist_stern(img) * 255)
        elif index == 72:
            img = np.uint8(cm.gnuplot(img) * 255)
        elif index == 73:
            img = np.uint8(cm.gnuplot2(img) * 255)
        elif index == 74:
            img = np.uint8(cm.CMRmap(img) * 255)
        elif index == 75:
            img = np.uint8(cm.cubehelix(img) * 255)
        elif index == 76:
            img = np.uint8(cm.brg(img) * 255)
        elif index == 77:
            img = np.uint8(cm.gist_rainbow(img) * 255)
        elif index == 78:
            img = np.uint8(cm.rainbow(img) * 255)
        elif index == 79:
            img = np.uint8(cm.jet(img) * 255)
        elif index == 80:
            img = np.uint8(cm.turbo(img) * 255)
        elif index == 81:
            img = np.uint8(cm.nipy_spectral(img) * 255)
        elif index == 82:
            img = np.uint8(cm.gist_ncar(img) * 255)
        else:
            pass
        return img

'''
cmaps = [('Perceptually Uniform Sequential', [
            'viridis', 'plasma', 'inferno', 'magma', 'cividis']),
         ('Sequential', [
            'Greys', 'Purples', 'Blues', 'Greens', 'Oranges', 'Reds',
            'YlOrBr', 'YlOrRd', 'OrRd', 'PuRd', 'RdPu', 'BuPu',
            'GnBu', 'PuBu', 'YlGnBu', 'PuBuGn', 'BuGn', 'YlGn']),
         ('Sequential (2)', [
            'binary', 'gist_yarg', 'gist_gray', 'gray', 'bone', 'pink',
            'spring', 'summer', 'autumn', 'winter', 'cool', 'Wistia',
            'hot', 'afmhot', 'gist_heat', 'copper']),
         ('Diverging', [
            'PiYG', 'PRGn', 'BrBG', 'PuOr', 'RdGy', 'RdBu',
            'RdYlBu', 'RdYlGn', 'Spectral', 'coolwarm', 'bwr', 'seismic']),
         ('Cyclic', ['twilight', 'twilight_shifted', 'hsv']),
         ('Qualitative', [
            'Pastel1', 'Pastel2', 'Paired', 'Accent',
            'Dark2', 'Set1', 'Set2', 'Set3',
            'tab10', 'tab20', 'tab20b', 'tab20c']),
         ('Miscellaneous', [
            'flag', 'prism', 'ocean', 'gist_earth', 'terrain', 'gist_stern',
            'gnuplot', 'gnuplot2', 'CMRmap', 'cubehelix', 'brg',
            'gist_rainbow', 'rainbow', 'jet', 'turbo', 'nipy_spectral',
            'gist_ncar'])]'''


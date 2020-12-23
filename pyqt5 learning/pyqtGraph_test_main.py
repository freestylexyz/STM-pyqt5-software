# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/23 09:35:14
@Author   : milier00
@FileName : pyqtGraph_test_main.py
"""
import sys
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup
from PyQt5.QtCore import pyqtSignal, Qt
from pyqtGraph_test import Ui_pgwidget
from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg

class myPG(QWidget, Ui_pgwidget):
    timer = pg.QtCore.QTimer()

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.p1 = self.graphicsView.addPlot(1,1)
        self.data1 = np.random.normal(size=300)
        self.curve1 = self.p1.plot(self.data1)
        self.ptr1 = 0

        self.p2 = self.graphicsView.addPlot(2,1)
        self.p2.setDownsampling(mode='peak')
        self.p2.setClipToView(True)
        self.curve2 = self.p2.plot()
        self.data2 = np.empty(100)
        self.ptr2 = 0

        self.timer.timeout.connect(self.update)
        self.timer.start(50)

    def update1(self):
        self.data1[:-1] = self.data1[1:]  # shift data in the array one sample left
        # (see also: np.roll)
        self.data1[-1] = np.random.normal()
        self.curve1.setData(self.data1)
        self.ptr1 += 1
        self.curve1.setPos(self.ptr1, 0)

    def update2(self):
        self.data2[self.ptr2] = np.random.normal()
        self.ptr2 += 1
        if self.ptr2 >= self.data2.shape[0]:
            tmp = self.data2
            self.data2 = np.empty(self.data2.shape[0] * 2)
            self.data2[:tmp.shape[0]] = tmp
        self.curve2.setData(self.data2[:self.ptr2])
        self.curve2.setPos(0, self.ptr2)

    # update all plots
    def update(self):
        self.update1()
        self.update2()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    myWin = myPG()
    myWin.show()
    sys.exit(app.exec_())
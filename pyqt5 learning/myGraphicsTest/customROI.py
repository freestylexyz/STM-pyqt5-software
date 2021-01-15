# -*- coding: utf-8 -*-
"""
@Date     : 2021/1/6 20:08:55
@Author   : milier00
@FileName : customROI.py
"""
from pyqtgraph import ROI
from pyqtgraph import ViewBox

from PyQt5 import QtCore, QtGui
import numpy as np
#from numpy.linalg import norm

from pyqtgraph import Point
from pyqtgraph import SRTTransform
from math import cos, sin
from pyqtgraph import functions as fn
from pyqtgraph import GraphicsObject
from pyqtgraph import UIGraphicsItem
from pyqtgraph import getConfigOption
from PyQt5.QtCore import Qt


class CrossCenterROI(ROI):
    def __init__(self, pos, size, centered=False, sideScalers=False, **args):
        ROI.__init__(self, pos, size, **args)
        if centered:
            center = [0.5, 0.5]
        else:
            center = [0, 0]

        self.addScaleHandle([1, 1], center)
        if sideScalers:
            self.addScaleHandle([1, 0.5], [center[0], 0.5])
            self.addScaleHandle([0.5, 1], [0.5, center[1]])

    def paint(self, p, opt, widget):
        # Note: don't use self.boundingRect here, because subclasses may need to redefine it.
        r = QtCore.QRectF(0, 0, self.state['size'][0], self.state['size'][1]).normalized()
        p.setRenderHint(QtGui.QPainter.Antialiasing)
        p.setPen(self.currentPen)
        p.translate(r.left(), r.top())
        p.scale(r.width() / 20, r.height() / 20)  # Dan 1/4/2021
        p.drawRect(0, 0, 20, 20)                  # Dan 1/4/2021
        p.drawLine(Point(10, 9), Point(10, 11))  # Dan 1/4/2021
        p.drawLine(Point(9, 10), Point(11, 10))  # Dan 1/4/2021

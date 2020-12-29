# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/29 19:13:42
@Author   : milier00
@FileName : symbols.py
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

class mySymbols(QWidget):

    def __init__(self):
        super().__init__()

    def cross(self):
        cross = np.zeros((23, 23))
        for i in range(23):
            cross[i, i] = 1.0
            cross[22-i, i] = 1.0
        return cross

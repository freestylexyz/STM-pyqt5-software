# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/24 17:41:01
@Author   : milier00
@FileName : SendOptions.py
"""
import sys

sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup
from PyQt5.QtCore import pyqtSignal, Qt
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
from PointEditor_ui import Ui_PointEditor
import numpy as np
import conversion as cnv
import functools as ft


class myPointEditor(QWidget, Ui_PointEditor):
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.idling = True
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        pass

    def init_sendoptions(self):
        pass

    # Emit close signal
    def closeEvent(self, event):
        if self.idling:
            self.close_signal.emit()
            event.accept()
        else:
            event.ignore()

    def enable_serial(self, enable):
        pass

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myPointEditor()
    window.show()
    sys.exit(app.exec_())
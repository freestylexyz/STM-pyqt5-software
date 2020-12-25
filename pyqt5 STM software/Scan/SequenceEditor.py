# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/24 17:43:11
@Author   : milier00
@FileName : SequenceEditor.py
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
from SequenceEditor_ui import Ui_SequenceEditor
import numpy as np
import conversion as cnv
import functools as ft


class mySequenceEditor(QWidget, Ui_SequenceEditor):
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.idling = True
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        # set up table widget
        self.table_Content_SeqEditor.setColumnWidth(0, 70)
        self.table_Content_SeqEditor.setColumnWidth(1, 60)
        self.table_Content_SeqEditor.setColumnWidth(2, 60)
        self.table_Content_SeqEditor.setColumnWidth(3, 60)
        self.table_Content_SeqEditor.setColumnWidth(4, 40)
        self.table_Content_SeqEditor.setColumnWidth(5, 40)
        self.table_Content_SeqEditor.setColumnWidth(6, 40)

    def init_seqeditor(self):
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
    window = mySequenceEditor()
    window.show()
    sys.exit(app.exec_())
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
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QAbstractItemView, QFileDialog, QHeaderView, QMessageBox, QTableWidgetItem
from PyQt5.QtCore import pyqtSignal, Qt
from PyQt5.QtGui import *
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
from PointEditor_ui import Ui_PointEditor
import numpy as np
import conversion as cnv
import functools as ft
import pickle


class myPointEditor(QWidget, Ui_PointEditor):
    close_signal = pyqtSignal()
    points_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.idling = True
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.pushButton_Add.clicked.connect(self.add)
        self.pushButton_Remove.clicked.connect(self.remove)
        self.pushButton_Insert.clicked.connect(self.insert)
        self.pushButton_Clear.clicked.connect(self.clear)
        self.pushButton_Save.clicked.connect(self.save)
        self.pushButton_Load.clicked.connect(self.load)
        self.pushButton_Done.clicked.connect(self.done)

        # self.table_Content.currentCellChanged.connect(self.set_alignment)

        self.table_Content.setAlternatingRowColors(True)
        self.table_Content.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.table_Content.resizeColumnsToContents()
        self.table_Content.resizeRowsToContents()

        trig = QAbstractItemView.DoubleClicked | QAbstractItemView.SelectedClicked
        self.table_Content.setEditTriggers(trig)


    def init_sendoptions(self):
        pass

    # Add point button slot
    def add(self):
        curRow = self.table_Content.rowCount()
        self.table_Content.insertRow(curRow)

    # Remove slelcted button slot
    def remove(self):
        curRow = self.table_Content.currentRow()
        self.table_Content.removeRow(curRow)

    # Insert point button slot
    def insert(self):
        curRow = self.table_Content.currentRow()
        self.table_Content.insertRow(curRow)

    # Clear all button slot
    def clear(self):
        self.table_Content.clearContents()

    # Save as .pts button slot
    def save(self):
        self.points = []
        for i in range(self.table_Content.rowCount()):
            x = int(self.table_Content.item(i, 0).text())
            y = int(self.table_Content.item(i, 1).text())
            if x < -32768 or x > 32768 or y < -32768 or y > 32768:
                QMessageBox.warning(None, "Point Editor", "Points out of range !!!", QMessageBox.Retry)
                return
            else:
                self.points += [(x, y)]
        self.points_signal.emit()   # update point list variable

        fname = QFileDialog.getSaveFileName(self, 'Save file', '', 'PTS Files (*.pts)')[0]  # Save file and get file name
        if fname != '':  # Savable
            with open(fname, 'wb') as output:
                pickle.dump(self.points, output, pickle.HIGHEST_PROTOCOL)  # Save points

    # Load points button slot
    def load(self):
        self.points = []
        fname = QFileDialog.getOpenFileName(self, 'Open file', '', 'PTS Files (*.pts)')[0]  # Open file and get file name
        if fname != '':  # Savable
            with open(fname, 'rb') as input:
                self.points = pickle.load(input)

        self.table_Content.setRowCount(len(self.points))
        for i in range(len(self.points)):
            x = QTableWidgetItem(str(self.points[i][0]))
            y = QTableWidgetItem(str(self.points[i][1]))
            # x.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            # y.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.table_Content.setItem(i, 0, x)
            self.table_Content.setItem(i, 1, y)
        # self.table_Content.currentCellChanged.disconnect(self.set_alignment)

    # Done button slot
    def done(self):
        self.points = []
        for i in range(self.table_Content.rowCount()):
            x = int(self.table_Content.item(i, 0).text())
            y = int(self.table_Content.item(i, 1).text())
            if x < -32768 or x > 32768 or y < -32768 or y > 32768:
                QMessageBox.warning(None, "Point Editor", "Points out of range !!!", QMessageBox.Retry)
                return
            else:
                self.points += [(x, y)]
        self.points_signal.emit()   # update point list variable
        self.close()

    # set current cell align center
    # !!! not connected
    def set_alignment(self, curRow, curCol):
        newItem = QTableWidgetItem()
        self.table_Content.setItem(curRow, curCol, newItem)
        newItem.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)

    def enable_serial(self, enable):
        pass

    # Emit close signal
    # !!! need to tell scan
    def closeEvent(self, event):
        if self.idling:
            self.close_signal.emit()
            event.accept()
        else:
            event.ignore()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myPointEditor()
    window.show()
    sys.exit(app.exec_())
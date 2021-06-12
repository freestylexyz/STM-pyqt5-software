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
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QAbstractItemView, QFileDialog, QHeaderView, QMessageBox, QTableWidgetItem, QShortcut
from PyQt5.QtWidgets import QWidget, QComboBox, QCheckBox, QLabel, QSpinBox, QDoubleSpinBox, QApplication, QFileDialog, QButtonGroup, QMessageBox
from PyQt5.QtCore import pyqtSignal, Qt

from PyQt5.QtGui import *
from PyQt5 import QtCore
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
from PointEditor_ui import Ui_PointEditor
import numpy as np
import conversion as cnv
import functools as ft
import pickle


class myPointEditor(QWidget, Ui_PointEditor):
    close_signal = pyqtSignal(int)
    points_signal = pyqtSignal(int)
    one_point_signal = pyqtSignal(int)

    def __init__(self):
        super().__init__()
        self.idling = True
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        QShortcut(QtGui.QKeySequence('Esc', ), self, self.close)
        self.setWindowFlags(Qt.WindowMinimizeButtonHint)

        self.pushButton_Add.clicked.connect(self.add)
        self.pushButton_Remove.clicked.connect(self.remove)
        self.pushButton_Insert.clicked.connect(self.insert)
        self.pushButton_Clear.clicked.connect(self.clear)
        self.pushButton_Save.clicked.connect(self.save)
        self.pushButton_Load.clicked.connect(self.load)
        self.pushButton_Done.clicked.connect(self.done)

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
        x = QSpinBox()
        x.setMinimum(-32768)
        x.setMaximum(32767)
        x.setValue(0)
        y = QSpinBox()
        y.setMinimum(-32768)
        y.setMaximum(32767)
        y.setValue(0)
        self.table_Content.setCellWidget(curRow, 0, x)
        self.table_Content.setCellWidget(curRow, 1, y)
        x.editingFinished.connect(self.update_graphics)
        y.editingFinished.connect(self.update_graphics)
        self.update_graphics()

    # Remove selected button slot
    def remove(self):
        curRow = self.table_Content.currentRow()
        self.table_Content.removeRow(curRow)
        self.update_graphics()

    # Insert point button slot
    def insert(self):
        curRow = self.table_Content.currentRow()
        self.table_Content.insertRow(curRow)
        x = QSpinBox()
        x.setMinimum(-32768)
        x.setMaximum(32767)
        x.setValue(0)
        y = QSpinBox()
        y.setMinimum(-32768)
        y.setMaximum(32767)
        y.setValue(0)
        self.table_Content.setCellWidget(curRow, 0, x)
        self.table_Content.setCellWidget(curRow, 1, y)
        x.editingFinished.connect(self.update_graphics)
        y.editingFinished.connect(self.update_graphics)
        self.update_graphics()

    # Clear all button slot
    def clear(self):
        self.table_Content.setRowCount(1)
        self.table_Content.clearContents()
        x = QSpinBox()
        x.setMinimum(-32768)
        x.setMaximum(32767)
        x.setValue(100)
        y = QSpinBox()
        y.setMinimum(-32768)
        y.setMaximum(32767)
        y.setValue(100)
        self.table_Content.setCellWidget(0, 0, x)
        self.table_Content.setCellWidget(0, 1, y)
        x.editingFinished.connect(self.update_graphics)
        y.editingFinished.connect(self.update_graphics)
        self.one_point_signal.emit(2)

    # Save as .pts button slot
    def save(self):
        self.points = []
        for i in range(self.table_Content.rowCount()):
            x = int(self.table_Content.cellWidget(i, 0).value())
            y = int(self.table_Content.cellWidget(i, 1).value())
            self.points += [(x, y)]

        fname = QFileDialog.getSaveFileName(self, 'Save file', '', 'PTS Files (*.pts)')[0]  # Save file and get file name
        if fname != '':  # Savable
            with open(fname, 'wb') as output:
                pickle.dump(self.points, output, pickle.HIGHEST_PROTOCOL)  # Save points

    # Load points button slot
    def load(self):
        self.points = []
        fname = QFileDialog.getOpenFileName(self, 'Open file', '', ['PTS Files (*.pts)', 'SPC Files (*.spc)'])[0]  # Open file and get file name
        if fname != '':
            with open(fname, 'rb') as input:
                if fname[-4:] == '.pts':
                    self.points = pickle.load(input)
                elif fname[-4:] == '.spc':
                    self.points = pickle.load(input).point_list

        self.table_Content.setRowCount(len(self.points))
        for i in range(len(self.points)):
            x = QSpinBox()
            x.setMinimum(-32768)
            x.setMaximum(32767)
            x.setValue(self.points[i][0])
            y = QSpinBox()
            y.setMinimum(-32768)
            y.setMaximum(32767)
            y.setValue(self.points[i][1])
            self.table_Content.setCellWidget(i, 0, x)
            self.table_Content.setCellWidget(i, 1, y)
            x.editingFinished.connect(self.update_graphics)
            y.editingFinished.connect(self.update_graphics)

    # Update table widget when scan view changed
    def update_from_graphics(self, points):
        self.points = []
        for point in points:
            x = point[0]
            y = point[1]
            self.points += [(x, y)]
        self.table_Content.setRowCount(len(self.points))
        for i in range(len(self.points)):
            x = QSpinBox()
            x.setMinimum(-32768)
            x.setMaximum(32767)
            x.setValue(self.points[i][0])
            y = QSpinBox()
            y.setMinimum(-32768)
            y.setMaximum(32767)
            y.setValue(self.points[i][1])
            self.table_Content.setCellWidget(i, 0, x)
            self.table_Content.setCellWidget(i, 1, y)
            x.editingFinished.connect(self.update_graphics)
            y.editingFinished.connect(self.update_graphics)

    # Done button slot
    def done(self):
        self.points = []
        for i in range(self.table_Content.rowCount()):
            x = int(self.table_Content.cellWidget(i, 0).value())
            y = int(self.table_Content.cellWidget(i, 1).value())
            self.points += [(x, y)]
        self.points_signal.emit(0)   # update point list variable
        self.close_signal.emit(-1)
        self.close()

    # Update Scan graphics view when each point finished
    def update_graphics(self):
        self.points = []
        if self.table_Content.rowCount() >= 0:
            if self.table_Content.rowCount() == 1 or self.table_Content.rowCount() == 0:
                self.one_point_signal.emit(2)
            else:
                for i in range(self.table_Content.rowCount()):
                    x = int(self.table_Content.cellWidget(i, 0).value())
                    y = int(self.table_Content.cellWidget(i, 1).value())
                    self.points += [(x, y)]
                self.points_signal.emit(0)  # update point list variable


    def enable_serial(self, enable):
        pass

    # def keyPressEvent(self, event):
    #     print("按下：" + str(event.key()))
    #     if event.key() == Qt.Key_Z:
    #         if QApplication.keyboardModifiers() == Qt.ControlModifier:
    #             self.undoStack.createUndoAction(self, '撤销')
    #
    #     if event.key() == Qt.Key_Y:
    #         if QApplication.keyboardModifiers() == Qt.ControlModifier:
    #             self.undoStack.createRedoAction(self, '重做')

    # Emit close signal
    def closeEvent(self, event):
        if self.idling:
            self.close_signal.emit(-1)
            event.accept()
        else:
            event.ignore()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myPointEditor()
    window.show()
    sys.exit(app.exec_())
# -*- coding: utf-8 -*-
"""
@Date     : 2021/3/24 09:44:08
@Author   : milier00
@FileName : DataWindow.py
"""
import sys
import os
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup, QMainWindow, QAction, QGridLayout, QCheckBox, QRadioButton, QFileDialog, QShortcut, QListWidget
from PyQt5.QtCore import pyqtSignal, Qt, QDir, QRectF, QThread, QTimer
from PyQt5.QtGui import QIntValidator, QIcon
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
import pyqtgraph.opengl as gl
from DataWindow_ui import Ui_DataWindow
from GraphWindow import myGraphWindow
import numpy as np
import conversion as cnv
import functools as ft
import math
import pickle

class DropInList(QListWidget):
    list_changed_signal = pyqtSignal()
    def __init__(self):
        super(DropInList, self).__init__()
        self.setAcceptDrops(True)

    def dropEvent(self, QDropEvent):
        source_Widget=QDropEvent.source()
        items=source_Widget.selectedItems()

        for i in items:
            source_Widget.takeItem(source_Widget.indexFromItem(i).row())
            self.addItem(i)

        self.list_changed_signal.emit() # only used for drop event

class myDataWindow(QWidget, Ui_DataWindow):
    # Common signal
    close_signal = pyqtSignal()
    list_changed_signal = pyqtSignal()

    def __init__(self, data_list):
        super().__init__()
        self.setupUi(self)
        self.init_UI(data_list)

    def init_UI(self, data_list):
        self.parent_data_list = data_list

        # listWidget | Plot list
        self.listWidget = DropInList()
        self.listWidget.list_changed_signal.connect(self.refresh_list)
        grid = QGridLayout()
        grid.addWidget(self.listWidget)
        self.groupBox.setLayout(grid)

        self.listWidget.setContextMenuPolicy(Qt.CustomContextMenu)
        self.listWidget.customContextMenuRequested[QtCore.QPoint].connect(self.rightMenuShow)

    # Plot List | right click menu
    def rightMenuShow(self):
        rightMenu = QtGui.QMenu(self.listWidget)
        deleteAction = QtGui.QAction("Delete", self, triggered=self.delete)
        rightMenu.addAction(deleteAction)
        rightMenu.exec_(QtGui.QCursor.pos())

    # Plot List | delete action slot
    def delete(self):
        # get clicked item index
        item = self.listWidget.currentItem().text()

        # get list of index to delete
        delete_items = []
        if self.listWidget.currentItem().text()[0] != ' ':
            for plot in self.plot_list:
                if plot.find(self.listWidget.currentItem().text()) != -1:
                    delete_items.append(plot)
            print(delete_items)
        elif self.listWidget.currentItem().text()[0] == ' ':
            delete_items = [item]

        # delete from listWidget
        print(self.plot_list)
        for i in range(len(self.plot_list) - 1, -1, -1):
            if self.plot_list[i] in delete_items:
                self.plot_list.pop(i)
        self.listWidget.clear()
        self.listWidget.addItems(self.plot_list)
        self.list_changed_signal.emit()

    # Plot List | drop items signal slot and menu add2win slot
    def refresh_list(self):
        # get current plot list from listWidget
        self.plot_list = []
        for row in range(self.listWidget.count()):
            self.plot_list.append(self.listWidget.item(row).text())
            # print(self.listWidget.item(row).text())

        # remove repeated items
        self.plot_list = list(set(self.plot_list))

        # sort plot list
        self.plot_list = sorted(self.plot_list, key=self.parent_data_list.index)
        self.listWidget.clear()
        self.listWidget.addItems(self.plot_list)
        self.list_changed_signal.emit()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myDataWindow(data_list=['1', '2', '3'])
    window.show()
    sys.exit(app.exec_())
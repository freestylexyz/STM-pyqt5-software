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
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup, QMainWindow, QAction, QAbstractItemView, QGridLayout, QComboBox, QCheckBox, QRadioButton, QFileDialog, QShortcut, QListWidget
from PyQt5.QtCore import pyqtSignal, Qt, QDir, QRectF, QThread, QTimer, QPointF
from PyQt5.QtGui import QIntValidator, QIcon, QKeySequence, QColor
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
import pyqtgraph.opengl as gl
from DataWindow_ui import Ui_DataWindow
from GraphWindow import myGraphWindow
import numpy as np
import conversion as cnv
import functools as ft
import pyqtgraph as pg
import math
import pickle
import copy

class DropInList(QListWidget):
    list_changed_signal = pyqtSignal()
    def __init__(self):
        super(DropInList, self).__init__()
        self.setAcceptDrops(True)

    def dropEvent(self, QDropEvent):
        source_Widget = QDropEvent.source()
        items = source_Widget.selectedItems()

        for i in items:
            source_Widget.takeItem(source_Widget.indexFromItem(i).row())
            self.addItem(i)

        self.list_changed_signal.emit() # only used for drop event

class myDataWindow(QWidget, Ui_DataWindow):
    # Common signal
    close_signal = pyqtSignal()
    list_changed_signal = pyqtSignal()

    def __init__(self, data_list, data_paths):
        super().__init__()
        self.setupUi(self)
        self.init_UI(data_list, data_paths)

    def init_UI(self, data_list, data_paths):
        self.parent_data_list = data_list
        self.parent_data_paths = data_paths
        self.parent_data_colors = [QColor('greenyellow'), QColor('mediumspringgreen'), QColor('cyan'), \
                                   QColor('pink'), QColor('violet'), QColor('khaki'), QColor('tomato') ]

        self.plot_list = []
        self.displayed_plot_list = []
        self.data = []
        self.file_type = 0  # file visibility in dir, 0: (.spc); 1: (.dep)

        # signals |
        # self.list_changed_signal.connect(self.update_graph)
        # self.list_changed_signal.connect(self.edit_check_state)

        # pushButton |
        self.pushButton_xScale.clicked.connect(lambda: self.scale(0))
        self.pushButton_yScale.clicked.connect(lambda: self.scale(1))

        # listWidget | Plot list
        self.listWidget = DropInList()
        self.listWidget.list_changed_signal.connect(self.pre_treat_list)
        self.listWidget.itemClicked.connect(self.edit_check_state)
        self.listWidget.itemSelectionChanged.connect(self.edit_selection)
        self.listWidget.setSelectionMode(QAbstractItemView.ExtendedSelection)
        grid = QGridLayout()
        grid.addWidget(self.listWidget, 2, 1)

        # comboBox | file type selection
        self.comboBox = QComboBox()
        self.comboBox.addItems(['Show .spc only', 'Show .dep only'])
        self.comboBox.currentIndexChanged.connect(self.file_type_changed)
        grid.addWidget(self.comboBox, 1, 1)
        self.groupBox.setLayout(grid)

        self.listWidget.setContextMenuPolicy(Qt.CustomContextMenu)
        self.listWidget.customContextMenuRequested[QtCore.QPoint].connect(self.rightMenuShow)

        # graphicsView |
        self.plot = self.graphicsView.addPlot()
        self.plot.disableAutoRange()

        # ROI | date scanner
        self.scanner = pg.InfiniteLine(angle=90, movable=True)
        self.plot.vb.addItem(self.scanner)
        self.scanner.sigPositionChangeFinished.connect(self.scanner_moved)
        self.scanner.addMarker('<|', position=0.1)
        self.scanner.addMarker('|>', position=0.2)
        self.scanner.addMarker('>|', position=0.3)
        self.scanner.addMarker('|<', position=0.4)
        self.scanner.addMarker('<|>', position=0.5)
        self.scanner.addMarker('>|<', position=0.6)
        self.scanner.addMarker('v', position=0.7)
        self.scanner.addMarker('^', position=0.8)
        self.scanner.addMarker('o', position=0.9)

        # self.scanner.setBounds([0, 200])

        # keyBoard event | Delete
        self.shortcut = QShortcut(QKeySequence('Delete'), self)
        self.shortcut.activated.connect(self.delete)

    # Plot List | file type in comboBox changed slot
    def file_type_changed(self, index):
        # msg = QMessageBox.question(None, "Data window", "Changing visible file type will clear all your current data!!!\n\n" +
        #                      "Do you really want to change it?", QMessageBox.Yes | QMessageBox.No, QMessageBox.Yes)
        # if msg == QMessageBox.Yes:
        #     self.file_type = index
        #     self.refresh_list()
        # else:
        #     self.comboBox.setCurrentIndex(self.file_type)
        self.file_type = index
        self.refresh_list()

    # Plot List | set all items in listWidget checked
    def set_list_checked(self):
        for row in range(self.listWidget.count()):
            self.listWidget.item(row).setCheckState(2)

    # Plot List | set check state by logic
    def edit_check_state(self, item):

        # collect multi-channel file names
        multi_file_names = [plot for plot in self.plot_list if plot[0] != '\t']
        multi_file_left = [0] * len(multi_file_names)

        # count number of multi-channel file data left in current window
        # multi-channel file itself is not included
        for i in range(len(multi_file_names)):
            for plot in self.plot_list:
                if plot.find(multi_file_names[i]) != -1 and len(plot) > 12:
                    multi_file_left[i] += 1

        if item.text()[0] == '\t':
            child_states = []
            parent_text = multi_file_names[multi_file_names.index(item.text()[1:13])]
            parent_index = self.plot_list.index(parent_text)
            # print(parent_text)
            # print(parent_index)
            for i in range(multi_file_left[multi_file_names.index(parent_text)]):
                child_states.append(self.listWidget.item(parent_index + i + 1).checkState())
            if sum(child_states) == 0:  # All Unchecked
                self.listWidget.item(parent_index).setCheckState(0)
            elif sum(child_states) == 2 * multi_file_left[multi_file_names.index(parent_text)]:  # All Checked
                self.listWidget.item(parent_index).setCheckState(2)
            else:  # PartiallyChecked
                self.listWidget.item(parent_index).setCheckState(1)

        elif item.text()[0] != '\t':
            if (item.text() in multi_file_names) and (multi_file_left[multi_file_names.index(item.text())] > 0):
                if item.checkState() == 0:
                    for i in range(multi_file_left[multi_file_names.index(item.text())]):
                        self.listWidget.item(i + 1).setCheckState(0)
                elif item.checkState() == 2:
                    for i in range(multi_file_left[multi_file_names.index(item.text())]):
                        self.listWidget.item(i + 1).setCheckState(2)

        self.displayed_plot_list = []
        for row in range(self.listWidget.count()):
            if self.listWidget.item(row).checkState() != 0:
                self.displayed_plot_list.append(self.listWidget.item(row).text())

        # print(self.displayed_plot_list)

        self.update_graph()

    # Plot List | set selection by logic
    def edit_selection(self):
        items = self.listWidget.selectedItems()
        for item in items:
            if item.text()[0] == '\t':
                index = self.plot_list.index(item.text()[1:13])
                self.listWidget.item(index).setSelected(True)

    # Plot List | right click menu
    def rightMenuShow(self):
        rightMenu = QtGui.QMenu(self.listWidget)
        deleteAction = QtGui.QAction("Delete", self, triggered=self.delete)
        rightMenu.addAction(deleteAction)
        rightMenu.exec_(QtGui.QCursor.pos())

    # Plot List | delete action slot
    def delete(self):
        items = self.listWidget.selectedItems()

        for item in items:
            # get list of index to delete
            delete_items = []
            if item.text()[0] != '\t':
                for plot in self.plot_list:
                    if plot.find(item.text()) != -1:
                        delete_items.append(plot)

            # delete from listWidget
            for i in range(len(self.plot_list) - 1, -1, -1):
                if self.plot_list[i] in delete_items:
                    self.plot_list.pop(i)

            # make sure multi-channel file names doesn't exit alone
            multi_file_names = [plot for plot in self.plot_list if plot[0] != '\t']
            multi_file_num = [0]*len(multi_file_names)
            multi_file_left = [0]*len(multi_file_names)

            for i in range(len(multi_file_names)):
                for plot in self.plot_list:
                    if plot.find(multi_file_names[i]) != -1:
                        multi_file_left[i] += 1

            for i in range(len(multi_file_names)):
                for file in self.parent_data_list:
                    if file.find(multi_file_names[i]) != -1:
                        multi_file_num[i] += 1
                if multi_file_num[i] > 1 and multi_file_left[i] == 1:
                    index = self.plot_list.index(multi_file_names[i])
                    self.plot_list.pop(index)

        self.listWidget.clear()
        self.listWidget.addItems(self.plot_list)
        self.set_list_checked()
        self.refresh_list()

    # Plot list | drop items signal slot
    def pre_treat_list(self):
        # deal with multi-channel data
        for row in range(self.listWidget.count()):
            for data in self.parent_data_list:
                if data.find(self.listWidget.item(row).text()) != -1 and len(data) > 12:
                    self.listWidget.addItem(data)
        self.refresh_list()

    # Plot List | pre_treat_list slot and menu add2win slot
    def refresh_list(self):     # if spc if dep

        # get current plot list from listWidget
        self.plot_list = []
        if self.file_type == 0:  # .spc
            for row in range(self.listWidget.count()):
                if self.listWidget.item(row).text().find('.spc') != -1:
                    self.plot_list.append(self.listWidget.item(row).text())

        elif self.file_type == 1:  # .dep
            for row in range(self.listWidget.count()):
                if self.listWidget.item(row).text().find('.dep') != -1:
                    self.plot_list.append(self.listWidget.item(row).text())

        # remove repeated items
        self.plot_list = list(set(self.plot_list))
        # sort plot list
        self.plot_list = sorted(self.plot_list, key=self.parent_data_list.index)
        # add items to listWidget
        self.listWidget.clear()
        self.listWidget.addItems(self.plot_list)
        # set color
        for row in range(self.listWidget.count()):
            # count child
            is_multi = 0
            for name in self.parent_data_list:
                if name.find(self.listWidget.item(row).text()+'_No') != -1:
                    is_multi += 1
            # multi-channel file has white background, others are colorful
            if self.listWidget.item(row).text()[-4:] == '.spc' and is_multi > 0:
                self.listWidget.item(row).setBackground(QColor('white'))
            else:
                color = self.parent_data_colors[self.parent_data_list.index(self.listWidget.item(row).text())]
                self.listWidget.item(row).setBackground(color)
        self.set_list_checked()
        self.listWidget.setCurrentRow(-1)
        self.displayed_plot_list.clear()
        self.displayed_plot_list = copy.deepcopy(self.plot_list)
        self.update_graph()

    # Graphics | update plot in current window
    def update_graph(self):
        self.data = []
        # collect (.spc) data in self.data
        for plot in self.displayed_plot_list:
            if plot[0] != '\t':
                index = self.parent_data_list.index(plot)
                data_path = self.parent_data_paths[index]
                with open(data_path, 'rb') as input:
                    self.data.append(pickle.load(input))
                    self.data[-1].path = data_path  # Change file path

        # collect multi-channel file names
        multi_file_names = [plot for plot in self.displayed_plot_list if plot[0] != '\t']
        multi_file_left = [0] * len(multi_file_names)
        multi_file_num = [0] * len(multi_file_names)

        # count number of multi-channel file data left in current window
        # multi-channel file itself is not included
        for i in range(len(multi_file_names)):
            for plot in self.displayed_plot_list:
                if plot.find(multi_file_names[i]) != -1 and len(plot) > 12:
                    multi_file_left[i] += 1

        for i in range(len(multi_file_names)):
            for file in self.parent_data_list:
                if file.find(multi_file_names[i]) != -1 and len(file) > 12:
                    multi_file_num[i] += 1

        # plot in graphicsView
        self.plot.clear()
        for i in range(len(self.data)):         # self.data is a list of SpcData
            if self.data[i].path[-4:] == '.spc':
                # print("shape[0]: ", self.data[i].data_.shape[0])
                # print("left: ", multi_file_left[i])
                if multi_file_left[i] > 0:      # multi-channel file
                    for j in range(multi_file_num[i]):
                        if ('\t'+multi_file_names[i] + '_No' + str(j + 1)) in self.displayed_plot_list:
                            color = self.parent_data_colors[self.parent_data_list.index('\t'+multi_file_names[i] + '_No' + str(j + 1))]
                            self.plot.plot(x=self.data[i].data_[j, :, 0], y=self.data[i].data_[j, :, 1], name='\t'+multi_file_names[i] + '_No' + str(j + 1), pen=color)
                elif multi_file_left[i] == 0:   # single channel file
                    color = self.parent_data_colors[self.parent_data_list.index(multi_file_names[i])]
                    self.plot.plot(x=self.data[i].data_[0, :, 0], y=self.data[i].data_[0, :, 1], name=multi_file_names[i], pen = color)
            elif self.data[i].path[-4:] == '.dep':
                color = self.parent_data_colors[self.parent_data_list.index(multi_file_names[i])]
                self.plot.plot(self.data[i].data, name=multi_file_names[i], pen=color)

        self.plot.listDataItems()[0].curve.setClickable(True, width=10)
        self.plot.listDataItems()[0].sigPointsClicked.connect(self.curve_point_clicked)
        self.plot.listDataItems()[0].sigClicked.connect(self.curve_clicked)


    # Graphics | scale by X/Y axis
    def scale(self, index):
        if index == 0:  # scale x
            self.plot.vb.enableAutoRange(axis='x', enable=True)
            self.plot.vb.updateAutoRange()
        elif index == 1:    # sacle y
            self.plot.vb.enableAutoRange(axis='y', enable=True)
            self.plot.vb.updateAutoRange()

    # Graphics | scanner moved slot
    def scanner_moved(self):
        cursor_x = self.scanner.value()     # X coordinate
        data_x = []*len(self.plot.listDataItems())
        data_y = []*len(self.plot.listDataItems())
        for curve in self.plot.listDataItems():
            near_x = min(curve.xData, key=lambda x: abs(x - cursor_x))
            index = list(curve.xData).index(near_x)
            data_x += [near_x]
            data_y += [curve.yData[index]]
        print("-------------")
        print("mapFromViewToItem !!!", self.plot.vb.mapFromViewToItem(self.scanner, QPointF(cursor_x,data_y[0])))
        print("mapFromView !!!", self.plot.vb.mapFromView(QPointF(cursor_x,data_y[0])))
        print("mapSceneToView !!!", self.plot.vb.mapSceneToView(QPointF(cursor_x, data_y[0])))
        print("mapToView !!!", self.plot.vb.mapToView(QPointF(cursor_x, data_y[0])))
        print("mapViewToScene !!!", self.plot.vb.mapViewToScene(QPointF(cursor_x, data_y[0])))

        # print("Approximation:")
        # print(data_x, data_y)

    def curve_point_clicked(self, pt, ev):
        print(pt)

    def curve_clicked(self, ev):
        print("!!!")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    paths =  ['C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03242100.spc',\
 'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03242100.spc',\
 'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03242100.spc', \
'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03242101.spc',\
 'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03252100.dep',\
 'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03252101.dep', \
'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03252102.dep']
    data = ['03242100.spc', '\t03242100.spc_No1', '\t03242100.spc_No2', '03242101.spc', '03252100.dep', '03252101.dep', '03252102.dep']
    window = myDataWindow(data_list=data, data_paths=paths)
    window.listWidget.addItems(data)
    # window.comboBox.setCurrentIndex(1)
    # window.file_type = 1
    window.refresh_list()

    # print("Original:")
    # print(window.plot.listDataItems()[0].xData)
    # print(window.plot.listDataItems()[0].yData)
    window.show()
    sys.exit(app.exec_())
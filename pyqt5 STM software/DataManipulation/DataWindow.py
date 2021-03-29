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
from PyQt5.QtWidgets import QApplication, QWidget, QInputDialog, QDesktopWidget, QMessageBox, QButtonGroup, QMainWindow, QAction, QAbstractItemView, QGridLayout, QComboBox, QCheckBox, QRadioButton, QFileDialog, QShortcut, QListWidget
from PyQt5.QtCore import pyqtSignal, Qt, QDir, QRectF, QThread, QTimer, QPointF
from PyQt5.QtGui import QIntValidator, QIcon, QKeySequence, QColor
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
import pyqtgraph.opengl as gl
from images import myImages
from DataWindow_ui import Ui_DataWindow
from SpectroscopyInfo import mySpectroscopyInfo
from DepositionInfo import myDepositionInfo
from GraphWindow import myGraphWindow
import numpy as np
import conversion as cnv
import functools as ft
import pyqtgraph as pg
from scipy.interpolate import UnivariateSpline
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
        ''' Add these 2 lines to DataWindow_ui.py '''
        # pg.setConfigOption('background', 'w')
        # pg.setConfigOption('foreground', 'k')

    def init_UI(self, data_list, data_paths):
        self.img = myImages()
        self.parent_data_list = data_list
        self.parent_data_paths = data_paths
        self.parent_data_colors = [QColor(106, 4, 15), QColor(0, 150, 199), QColor(60, 9, 108), \
                                   QColor(157, 2, 8), QColor(8, 28, 21), QColor(90, 24, 154), QColor(45, 106, 79) ]
        self.processed_data_colors = [QColor(3, 7, 30), QColor(3, 4, 94), QColor(16, 0, 43), QColor(208, 0, 0), QColor(123, 44, 191), QColor(232, 93, 4), \
                                      QColor(55, 6, 23), QColor(2, 62, 138), QColor(27, 67, 50), QColor(220, 47, 2), QColor(64, 145, 108), QColor(250, 163, 7), QColor(157, 78, 221)]
        '''['aliceblue', 'antiquewhite', 'aqua', 'aquamarine', 'azure', 'beige', 'bisque', 'black', 'blanchedalmond',
         'blue', 'blueviolet', 'brown', 'burlywood', 'cadetblue', 'chartreuse', 'chocolate', 'coral', 'cornflowerblue',
         'cornsilk', 'crimson', 'cyan', 'darkblue', 'darkcyan', 'darkgoldenrod', 'darkgray', 'darkgreen', 'darkgrey',
         'darkkhaki', 'darkmagenta', 'darkolivegreen', 'darkorange', 'darkorchid', 'darkred', 'darksalmon',
         'darkseagreen', 'darkslateblue', 'darkslategray', 'darkslategrey', 'darkturquoise', 'darkviolet', 'deeppink',
         'deepskyblue', 'dimgray', 'dimgrey', 'dodgerblue', 'firebrick', 'floralwhite', 'forestgreen', 'fuchsia',
         'gainsboro', 'ghostwhite', 'gold', 'goldenrod', 'gray', 'green', 'greenyellow', 'grey', 'honeydew', 'hotpink',
         'indianred', 'indigo', 'ivory', 'khaki', 'lavender', 'lavenderblush', 'lawngreen', 'lemonchiffon', 'lightblue',
         'lightcoral', 'lightcyan', 'lightgoldenrodyellow', 'lightgray', 'lightgreen', 'lightgrey', 'lightpink',
         'lightsalmon', 'lightseagreen', 'lightskyblue', 'lightslategray', 'lightslategrey', 'lightsteelblue',
         'lightyellow', 'lime', 'limegreen', 'linen', 'magenta', 'maroon', 'mediumaquamarine', 'mediumblue',
         'mediumorchid', 'mediumpurple', 'mediumseagreen', 'mediumslateblue', 'mediumspringgreen', 'mediumturquoise',
         'mediumvioletred', 'midnightblue', 'mintcream', 'mistyrose', 'moccasin', 'navajowhite', 'navy', 'oldlace',
         'olive', 'olivedrab', 'orange', 'orangered', 'orchid', 'palegoldenrod', 'palegreen', 'paleturquoise',
         'palevioletred', 'papayawhip', 'peachpuff', 'peru', 'pink', 'plum', 'powderblue', 'purple', 'red', 'rosybrown',
         'royalblue', 'saddlebrown', 'salmon', 'sandybrown', 'seagreen', 'seashell', 'sienna', 'silver', 'skyblue',
         'slateblue', 'slategray', 'slategrey', 'snow', 'springgreen', 'steelblue', 'tan', 'teal', 'thistle', 'tomato',
         'transparent', 'turquoise', 'violet', 'wheat', 'white', 'whitesmoke', 'yellow', 'yellowgreen']'''

        self.plot_list = []
        self.displayed_plot_list = []
        self.processed_list = []
        self.displayed_pro_list = []
        self.processed_data = []
        self.data = []
        self.file_type = 0  # file visibility in dir, 0: (.spc); 1: (.dep)

        self.spc_info = mySpectroscopyInfo()
        self.dep_info = myDepositionInfo()

        # signals |
        # self.list_changed_signal.connect(self.update_graph)
        # self.list_changed_signal.connect(self.edit_check_state)

        # pushButton |
        self.pushButton_xScale.clicked.connect(lambda: self.scale(0))
        self.pushButton_yScale.clicked.connect(lambda: self.scale(1))
        self.pushButton_Scanner.clicked.connect(self.show_scanner)

        # listWidget | Plot list
        self.listWidget = DropInList()
        self.listWidget.list_changed_signal.connect(self.pre_treat_list)
        self.listWidget.itemClicked.connect(ft.partial(self.edit_check_state, 0))
        self.listWidget.itemSelectionChanged.connect(self.edit_selection)
        self.listWidget.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.listWidget.setContextMenuPolicy(Qt.CustomContextMenu)
        self.listWidget.customContextMenuRequested[QtCore.QPoint].connect(lambda: self.rightMenuShow(0))
        grid = QGridLayout()
        grid.addWidget(self.listWidget, 2, 1)

        # listWidget | Processed list
        self.listWidget_Processed.itemClicked.connect(ft.partial(self.edit_check_state, 1))
        self.listWidget_Processed.itemDoubleClicked.connect(self.rename)
        self.listWidget_Processed.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.listWidget_Processed.setContextMenuPolicy(Qt.CustomContextMenu)
        self.listWidget_Processed.customContextMenuRequested[QtCore.QPoint].connect(lambda: self.rightMenuShow(1))

        # comboBox | file type selection
        self.comboBox = QComboBox()
        self.comboBox.addItems(['Show .spc only', 'Show .dep only'])
        self.comboBox.currentIndexChanged.connect(self.file_type_changed)
        grid.addWidget(self.comboBox, 1, 1)
        self.groupBox.setLayout(grid)

        # label | display scanner coordinates
        self.label = pg.LabelItem(justify='right')
        self.graphicsView.addItem(self.label, row=0, col=0)

        # graphicsView |
        self.plot = self.graphicsView.addPlot(row=1, col=0)
        self.plot.disableAutoRange()

        # ROI | data scanner
        gray_pen = pg.mkPen((150,150,150,255), width=1)
        self.scanner_vLine = pg.InfiniteLine(angle=90, movable=False, pen=gray_pen)
        self.plot.addItem(self.scanner_vLine, ignoreBounds=True)

        # keyBoard event | Delete
        self.shortcut = QShortcut(QKeySequence('Delete'), self)
        self.shortcut.activated.connect(self.delete)

    # Graphics | scanner mouse moved signal slot
    def mouseMoved(self, evt):
        pos = evt[0]  ## using signal proxy turns original arguments into a tuple
        if self.plot.sceneBoundingRect().contains(pos):
            mousePoint = self.plot.vb.mapSceneToView(pos)
            data_x = [] * len(self.plot.listDataItems())
            data_y = [] * len(self.plot.listDataItems())
            color = [] * len(self.plot.listDataItems())
            for curve in self.plot.listDataItems():
                near_x = min(curve.xData, key=lambda x: abs(x - mousePoint.x()))
                index = list(curve.xData).index(near_x)
                data_x += [near_x]
                data_y += [curve.yData[index]]
                color += [self.img.RGB_to_Hex(curve.curve.opts['pen'].brush().color().getRgb())]

            text = ''
            num = 0
            for i in range(len(data_x)):
                num += 1
                text += "<span style='font-size: 9pt'><span style='color: " + color[i] + "'>(%0.2f, %0.2f)   </span>" % (data_x[i], data_y[i])
                if num%3 == 0:
                    text += "<br />"
            self.label.setText(text)

            self.scanner_vLine.setPos(mousePoint.x())

    # Plot List | file type in comboBox changed slot
    def file_type_changed(self, index):
        self.file_type = index
        self.refresh_list(0)
        self.processed_data.clear()
        self.listWidget_Processed.clear()
        self.refresh_list(1)

    # Plot List(0) & Processed List(1) | set all items in listWidget checked
    def set_list_checked(self, index):
        if index == 0:
            for row in range(self.listWidget.count()):
                self.listWidget.item(row).setCheckState(2)
        elif index == 1:
            for row in range(self.listWidget_Processed.count()):
                self.listWidget_Processed.item(row).setCheckState(2)
                self.listWidget_Processed.item(row).setFlags(
                    self.listWidget_Processed.item(row).flags() | QtCore.Qt.ItemIsUserCheckable | QtCore.Qt.ItemIsEditable)

    # Plot List(0) & Processed List(1) | set check state by logic
    def edit_check_state(self, index, item):
        if index == 0:

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

        elif index == 1:
            self.displayed_pro_list = []
            for row in range(self.listWidget_Processed.count()):
                if self.listWidget_Processed.item(row).checkState() == 2:
                    self.displayed_pro_list.append(self.listWidget_Processed.item(row).text())

            self.update_graph()

    # Plot List | set selection by logic
    def edit_selection(self):
        items = self.listWidget.selectedItems()
        for item in items:
            if item.text()[0] == '\t':
                index = self.plot_list.index(item.text()[1:13])
                self.listWidget.item(index).setSelected(True)

    # Plot List(0) & Processed List(1) | right click menu, 0: Plot list; 1: Processed list
    def rightMenuShow(self, index):
        if index == 0:  # plot list
            rightMenu = QtGui.QMenu(self.listWidget)
            saveasAction = QtGui.QAction("Save as", self, triggered=lambda: self.save_as(0))
            deleteAction = QtGui.QAction("Delete", self, triggered=lambda: self.delete(0))
            infoAction = QtGui.QAction("Info", self, triggered=self.info)
            rightMenu.addAction(saveasAction)
            rightMenu.addAction(deleteAction)
            rightMenu.addAction(infoAction)
            rightMenu.exec_(QtGui.QCursor.pos())
        elif index == 1:    # processed list
            rightMenu = QtGui.QMenu(self.listWidget_Processed)
            saveasAction = QtGui.QAction("Save as", self, triggered=lambda: self.save_as(1))
            renameAction = QtGui.QAction("Rename", self, triggered=self.rename)
            deleteAction = QtGui.QAction("Delete", self, triggered=lambda: self.delete(1))
            rightMenu.addAction(saveasAction)
            rightMenu.addAction(renameAction)
            rightMenu.addAction(deleteAction)
            rightMenu.exec_(QtGui.QCursor.pos())

    # Plot List(0) & Processed List(1) | save as action slot
    def save_as(self, index):

        if index == 0:  # Plot list
            index = self.parent_data_list.index(self.listWidget.currentItem().text())
            data_path = self.parent_data_paths[index]
            with open(data_path, 'rb') as input:
                data = pickle.load(input)
            if self.listWidget.currentItem().text().find('.spc') != -1:
                data2save = data.data_
                if data2save.shape[0] > 1:
                    for i in range(data2save.shape[0]):
                        default_name = data_path[:-4] + '_No' + str(i + 1)
                        fileName, ok = QFileDialog.getSaveFileName(self, "Save", default_name, "DAT(*.dat)")
                        np.savetxt(fileName, data2save[i])
                elif data2save.shape[0] == 1:
                    default_name = data_path[:-4]
                    fileName, ok = QFileDialog.getSaveFileName(self, "Save", default_name, "DAT(*.dat)")
                    np.savetxt(fileName, data2save[0])
            elif self.listWidget.currentItem().text().find('.dep') != -1:
                data2save = data.data
                default_name = data_path[:-4]
                fileName, ok = QFileDialog.getSaveFileName(self, "Save", default_name, "DAT(*.dat)")
                np.savetxt(fileName, data2save)

        elif index == 1:    # Processed list
            file_name = self.listWidget_Processed.currentItem().text()
            data2save = self.processed_data[self.processed_list.index(file_name)]
            default_name = "../Test/test data/" + file_name
            fileName, ok = QFileDialog.getSaveFileName(self, "Save", default_name, "DAT(*.dat)")
            np.savetxt(fileName, data2save)

    # Processed List | info action slot
    def info(self):
        data_path = self.parent_data_paths[self.parent_data_list.index(self.listWidget.currentItem().text())]
        with open(data_path, 'rb') as input:
            data = pickle.load(input)
        if self.file_type == 0:
            self.spc_info.init_spcInfo(data)
            self.spc_info.setWindowTitle('Info: ' + self.listWidget.currentItem().text())
            self.spc_info.setWindowModality(2)
            self.spc_info.show()
        elif self.file_type == 1:
            self.dep_info.init_depInfo(data)
            self.dep_info.setWindowTitle('Info: ' + self.listWidget.currentItem().text())
            self.dep_info.setWindowModality(2)
            self.dep_info.show()

    # Processed List | rename action slot
    def rename(self):
        item = self.listWidget_Processed.currentItem()
        text, okPressed = QInputDialog.getText(self, "New name", "New name:", text=item.text())
        if okPressed and text != '':
            item.setText(text)
        self.set_list_checked(1)
        self.refresh_list(1)

    # Plot List(0) & Processed List(1) | delete action slot
    def delete(self, index):
        if index == 0:  # plot list
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
            self.set_list_checked(0)
            self.refresh_list(0)

        elif index == 1:    # processed list
            items = self.listWidget_Processed.selectedItems()

            for item in items:
                # get list of index to delete
                delete_items = []
                for plot in self.processed_list:
                    if plot.find(item.text()) != -1:
                        delete_items.append(plot)

                # delete from listWidget
                for i in range(len(self.processed_list) - 1, -1, -1):
                    if self.processed_list[i] in delete_items:
                        self.processed_list.pop(i)
                        self.processed_data.pop(i)
                        
            self.listWidget_Processed.clear()
            self.listWidget_Processed.addItems(self.processed_list)
            self.set_list_checked(1)
            self.refresh_list(1)

    # Plot list | drop items signal slot
    def pre_treat_list(self):
        # deal with multi-channel data
        for row in range(self.listWidget.count()):
            for data in self.parent_data_list:
                if data.find(self.listWidget.item(row).text()) != -1 and len(data) > 12:
                    self.listWidget.addItem(data)
        self.refresh_list(0)

    # Plot List(0) & Processed List(1) | pre_treat_list slot and menu add2win slot
    def refresh_list(self, index):     # if spc if dep

        if index == 0:  # Plot list
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
                    self.listWidget.item(row).setForeground(QColor('black'))
                    # self.listWidget.item(row).setBackground(QColor('aliceblue'))
                else:
                    color = self.parent_data_colors[self.parent_data_list.index(self.listWidget.item(row).text())]
                    self.listWidget.item(row).setForeground(color)
                    # self.listWidget.item(row).setBackground(QColor('aliceblue'))
            self.set_list_checked(0)
            self.listWidget.setCurrentRow(-1)
            self.displayed_plot_list.clear()
            self.displayed_plot_list = copy.deepcopy(self.plot_list)
            self.update_graph()

        elif index == 1:    # Processed list

            self.processed_list = []
            for row in range(self.listWidget_Processed.count()):
                self.processed_list.append(self.listWidget_Processed.item(row).text())
            self.listWidget_Processed.clear()
            self.listWidget_Processed.addItems(self.processed_list)

            # set color
            for row in range(self.listWidget_Processed.count()):
                color = self.processed_data_colors[self.processed_list.index(self.listWidget_Processed.item(row).text())]
                self.listWidget_Processed.item(row).setForeground(color)

            self.set_list_checked(1)
            self.displayed_pro_list = copy.deepcopy(self.processed_list)
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

        for i in range(len(self.processed_data)):
            if self.processed_list[i] in self.displayed_pro_list:
                print("update graph: ", self.processed_data[i].shape[:])    # (2, 1000)
                if self.processed_data[i].shape[0] == 2:    # spc
                    color = self.processed_data_colors[i]
                    self.plot.plot(x=self.processed_data[i][0,:], y=self.processed_data[i][1, :], name=self.processed_list[i], pen=color)
                else:   # dep
                    color = self.processed_data_colors[i]
                    self.plot.plot(self.processed_data[i], name=self.processed_list[i], pen=color)

    # Graphics | scale by X/Y axis
    def scale(self, index):
        if index == 0:  # scale x
            self.plot.vb.enableAutoRange(axis='x', enable=True)
            self.plot.vb.updateAutoRange()
        elif index == 1:    # sacle y
            self.plot.vb.enableAutoRange(axis='y', enable=True)
            self.plot.vb.updateAutoRange()

    # Graphics | show crosshair data scanner
    def show_scanner(self):
        if self.pushButton_Scanner.isChecked():
            self.plot.addItem(self.scanner_vLine, ignoreBounds=True)
            self.proxy = pg.SignalProxy(self.plot.scene().sigMouseMoved, rateLimit=60, slot=self.mouseMoved)
            self.label.show()
        else:
            self.plot.removeItem(self.scanner_vLine)
            # self.plot.scene().sigMouseMoved.disconnect(self.mouseMoved)
            self.label.hide()

    # Processing | Smooth 1D data
    def smooth1D(self, data, smooth):
        print("data shape: ", data.shape[0])
        x = np.linspace(0, 100, data.shape[0])
        y = data
        s = UnivariateSpline(x, y, s=smooth)
        xs = np.linspace(min(x), max(x), len(data))
        ys = s(xs)
        new_data = ys
        return new_data

    # Processing | Smooth 2D data
    def smooth2D(self, data, smooth):
        x = data[:, 0]
        y = data[:, 1]
        s = UnivariateSpline(x, y, s=smooth)
        xs = np.linspace(min(x), max(x), 1000)
        ys = s(xs)
        new_data = np.vstack((xs, ys))
        return new_data

    # Processing | Smooth data
    def smooth(self, degree):
        # deal with multi-channel file
        multi_file_names = [plot for plot in self.plot_list if plot[0] != '\t']
        multi_file_num = [0] * len(multi_file_names)

        for i in range(len(multi_file_names)):
            for file in self.parent_data_list:
                if file.find(multi_file_names[i]) != -1 and len(file) > 12:
                    multi_file_num[i] += 1

        # get list of index to delete
        items = self.listWidget.selectedItems()
        smooth_items = []
        for item in items:
            if item.text()[0] == '\t':
                smooth_items.append(item.text())
            elif item.text()[0] != '\t':
                index = multi_file_names.index(item.text())
                if multi_file_num[index] == 0:
                    smooth_items.append(item.text())

        data2smooth = []
        for plot in smooth_items:
            index = self.parent_data_list.index(plot)
            data_path = self.parent_data_paths[index]
            if plot.find('.spc') != -1:
                if len(plot) > 12:  # child name
                    no = int(plot[-1]) - 1
                    with open(data_path, 'rb') as input:
                        data = pickle.load(input)
                        data2smooth.append(data.data_[no])
                else:               # parent name
                    with open(data_path, 'rb') as input:
                        data = pickle.load(input)
                        data2smooth.append(data.data_[0])
            elif plot.find('.dep') != -1:
                with open(data_path, 'rb') as input:
                    data = pickle.load(input)
                    data2smooth.append(data.data)

        for data in data2smooth:
            print(len(data.shape[:]))
            if len(data.shape[:]) == 1: # dep
                new = self.smooth1D(data, degree)
            elif data.shape[1] == 2:  # spc
                new = self.smooth2D(data, degree)
            self.processed_data.append(new)

        num = 0
        for i in range(len(data2smooth)):
            for row in range(self.listWidget_Processed.count()):
                if self.listWidget_Processed.item(row).text().find('smooth') != -1:
                    num += 1
            self.listWidget_Processed.addItem('smooth_' + str(num+1))
        self.refresh_list(1)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    paths =  ['C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03242100.spc',\
 'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03242100.spc',\
 'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03242100.spc', \
'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03242101.spc',\
 'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03252100.dep',\
 'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03252101.dep', \
'C:/Users/DAN/Documents/MyCode/PythonScripts/pyqt5 STM software 0325/Test/test data\\03252102.dep']
    data = ['03242100.spc',  '\t03242100.spc_No1', '\t03242100.spc_No2', '03242101.spc', '03252100.dep', '03252101.dep', '03252102.dep']
    window = myDataWindow(data_list=data, data_paths=paths)
    window.listWidget.addItems(data)

    window.refresh_list(0)
    ''' set processed list '''
    # window.listWidget_Processed.addItems(data)
    # window.set_list_checked(1)
    # window.processed_list = data
    ''' show .dep file type '''
    # window.comboBox.setCurrentIndex(1)
    # window.file_type = 1
    window.show()
    sys.exit(app.exec_())
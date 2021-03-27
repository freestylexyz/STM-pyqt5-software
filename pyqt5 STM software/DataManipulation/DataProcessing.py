# -*- coding: utf-8 -*-
"""
@Date     : 2021/3/24 09:43:56
@Author   : milier00
@FileName : DataProcessing.py
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
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup, QMainWindow, QAbstractItemView, QAction, QVBoxLayout, QCheckBox, QRadioButton, QFileDialog, QShortcut
from PyQt5.QtCore import pyqtSignal, Qt, QDir, QRectF, QThread, QTimer
from PyQt5.QtGui import QIntValidator, QIcon
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
import pyqtgraph.opengl as gl
from SpectroscopyInfo import mySpectroscopyInfo
from DepositionInfo import myDepositionInfo
from DataProcessing_ui import Ui_DataProcessing
from DataWindow import myDataWindow
import numpy as np
import conversion as cnv
import functools as ft
import math
import pickle

class myDataProcessing(QMainWindow, Ui_DataProcessing):
    # Common signal
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.dir_path = ''
        self.file_type = 0              # file visibility in dir, 0: (.spc); 1: (.dep)
        self.file_index = 0
        self.file_names = []            # expanded file names for internal use
        self.file_paths = []            # file paths according to file_names
        self.displayed_file_names = []  # folded file names in Data listWidget
        self.displayed_file_paths = []  # folded file paths in Data listWidget
        self.overall_names = []         # all expanded names (.spc) and (.dep) in the folder
        self.overall_paths = []         # paths according to overall_names
        self.windows = []               # window object list
        self.window_names = []          # window names
        self.linecut_option = 1         # 0: option1, 1: option2

        self.current_window = None
        self.previous_window = None

        self.spc_info = mySpectroscopyInfo()
        self.dep_info = myDepositionInfo()

        # signals |
        QApplication.instance().focusChanged.connect(self.focus_window_changed)

        # pushButton |
        self.pushButton_Open.clicked.connect(self.open_file)
        self.pushButton_Saveas.clicked.connect(self.save_as)
        # self.pushButton_Saveall.clicked.connect(self.save_all)
        self.pushButton_Previous.clicked.connect(lambda: self.select_file(0))
        self.pushButton_Next.clicked.connect(lambda: self.select_file(1))
        self.pushButton_Refresh.clicked.connect(self.refresh_file)
        self.pushButton_ShowInfo.clicked.connect(self.show_info)

        # comboBox |
        self.comboBox.currentIndexChanged.connect(self.file_type_changed)

        # listWidget |
        self.listWidget.setDragEnabled(True)
        self.listWidget.setDragDropOverwriteMode(False)
        self.listWidget.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.listWidget.setDefaultDropAction(Qt.MoveAction)

        self.listWidget.setContextMenuPolicy(Qt.CustomContextMenu)
        self.listWidget.customContextMenuRequested[QtCore.QPoint].connect(self.rightMenuShow)

        self.listWidget.itemDoubleClicked.connect(lambda: self.file_changed(0))
        self.listWidget.itemClicked.connect(lambda: self.file_changed(1))
        # self.listWidget.setCurrentRow(-1)

        # keyboard event |
        QShortcut(QtGui.QKeySequence('Up', ), self, lambda: self.select_file(0))
        QShortcut(QtGui.QKeySequence('Down', ), self, lambda: self.select_file(1))
        QShortcut(QtGui.QKeySequence('Left', ), self, lambda: self.select_file(0))
        QShortcut(QtGui.QKeySequence('Right', ), self, lambda: self.select_file(1))

    # Data List | file type in comboBox changed slot
    def file_type_changed(self, index):
        self.file_type = index
        self.refresh_file()

    # Data List | file type in comboBox changed slot
    def select_file(self, index):
        if index == 0:  # previous
            if self.listWidget.currentRow() - 1 == -1:
                self.listWidget.setCurrentRow(len(self.displayed_file_names) - 1)
            else:
                self.listWidget.setCurrentRow(self.listWidget.currentRow() - 1)
        elif index == 1:  # next
            if self.listWidget.currentRow() + 1 == len(self.displayed_file_names):
                self.listWidget.setCurrentRow(0)
            else:
                self.listWidget.setCurrentRow(self.listWidget.currentRow() + 1)
        self.file_index = self.listWidget.currentRow()
        self.file_changed(1)

    # Data List | file selection changed / single click / double click slot
    def file_changed(self, index):
        ''' Double click to open file in a new window.
            Single click / change selection to change selected data in top level window. '''
        if index == 0:    # double click slot
            if self.listWidget.count() > 0:
                self.file_index = self.listWidget.currentRow()

                # build a new data window
                data_window = myDataWindow(self.overall_names, self.overall_paths)
                self.windows.append(data_window)
                self.windows[-1].listWidget.list_changed_signal.connect(self.refresh_file)
                if self.listWidget.currentItem().text().find('.spc') != -1:
                    self.windows[-1].comboBox.setCurrentIndex(0)
                    self.windows[-1].file_type = 0
                elif self.listWidget.currentItem().text().find('.dep') != -1:
                    self.windows[-1].comboBox.setCurrentIndex(1)
                    self.windows[-1].file_type = 1

                # get selected file
                item = self.listWidget.currentItem()
                add_name = [item.text()]

                # deal with multi-channel files
                multi_file_name = []
                for name in add_name:
                    if name[0] != '\t':
                        multi_file_name.append(name)
                for multi_name in multi_file_name:
                    for name in self.file_names:
                        if name.find(multi_name) != -1 and len(name) > 12 and (name not in add_name):
                            add_name.append(name)

                # add files to new window
                self.windows[-1].listWidget.addItems(add_name)
                self.windows[-1].set_list_checked()
                self.windows[-1].refresh_list()
                self.windows[-1].show()

        elif index == 1:  # file selection changed / single click slot
            if self.listWidget.count() > 0:
                self.file_index = self.listWidget.currentRow()

                # update plot list in window
                if self.previous_window != None and self.previous_window.listWidget.currentItem() != None:
                    self.previous_window.delete()
                    self.add2window()
                    self.previous_window.listWidget.setCurrentRow(0)

    # Data List | right click menu
    def rightMenuShow(self):
        rightMenu = QtGui.QMenu(self.listWidget)
        add2winAction = QtGui.QAction("Add to current window", self, triggered=self.add2window)
        rightMenu.addAction(add2winAction)
        open2winAction = QtGui.QAction("Open in new window", self, triggered=self.open2window)
        rightMenu.addAction(open2winAction)
        rightMenu.exec_(QtGui.QCursor.pos())

    def add2window(self):
        # get selected file index
        items = self.listWidget.selectedItems()
        add_index = []
        add_name = []
        for item in items:
            add_index.append(self.file_names.index(item.text()))
            add_name.append(item.text())
        # self.file_index = add_index[-1]

        # deal with multi-channel files
        multi_file_name = []
        for name in add_name:
            if name[0] != '\t':
                multi_file_name.append(name)
        for multi_name in multi_file_name:
            for name in self.file_names:
                if name.find(multi_name) != -1 and len(name) > 12 and (name not in add_name):
                    add_name.append(name)

        self.previous_window.listWidget.addItems(add_name)
        self.previous_window.set_list_checked()
        self.previous_window.refresh_list()

    def open2window(self):
        # get selected file index
        items = self.listWidget.selectedItems()
        add_index = []
        add_name = []
        for item in items:
            add_index.append(self.file_names.index(item.text()))
            add_name.append(item.text())
        # self.file_index = add_index[-1]

        # deal with multi-channel files
        multi_file_name = []
        for name in add_name:
            if name[0] != '\t':
                multi_file_name.append(name)
        for multi_name in multi_file_name:
            for name in self.file_names:
                if name.find(multi_name) != -1 and len(name) > 12 and (name not in add_name):
                    add_name.append(name)

        # open files in new data window
        data_window = myDataWindow(self.overall_names, self.overall_paths)
        self.windows.append(data_window)
        self.windows[-1].listWidget.list_changed_signal.connect(self.refresh_file)
        if self.listWidget.currentItem().text().find('.spc') != -1:
            self.windows[-1].comboBox.setCurrentIndex(0)
            self.windows[-1].file_type = 0
        elif self.listWidget.currentItem().text().find('.dep') != -1:
            self.windows[-1].comboBox.setCurrentIndex(1)
            self.windows[-1].file_type = 1
        self.windows[-1].listWidget.addItems(add_name)
        self.windows[-1].set_list_checked()
        self.windows[-1].refresh_list()
        self.windows[-1].show()

    def get_overall_list(self):
        self.overall_names.clear()
        self.overall_paths.clear()

        for root, dirs, files in os.walk(self.dir_path, topdown=False):
            for file in files:
                if file[-4:] == ".spc":
                    data_path = os.path.join(root, file)
                    with open(data_path, 'rb') as input:
                        data = pickle.load(input)
                        data.path = data_path  # Change file path
                    if data.data_.shape[0] == 1:
                        self.overall_paths.append(data_path)
                        self.overall_names.append(file)
                    else:
                        self.overall_paths.append(data_path)
                        self.overall_names.append(file)
                        for i in range(data.data_.shape[0]):
                            self.overall_paths.append(data_path)
                            # name = file if i==0 else (file+"_"+str(i))
                            self.overall_names.append('\t' + file + "_No" + str(i + 1))

        for root, dirs, files in os.walk(self.dir_path, topdown=False):
            for file in files:
                if file[-4:] == ".dep":
                    data_path = os.path.join(root, file)
                    with open(data_path, 'rb') as input:
                        data = pickle.load(input)
                        data.path = data_path  # Change file path
                    self.overall_paths.append(data_path)
                    self.overall_names.append(file)

        if len(self.overall_paths) <= 0:
            return

        # print("after open file --------------")
        # print("overall_names: ", self.overall_names)
        # print("overall_paths: ", self.overall_paths)

    # Data List | open folder button slot
    def open_file(self):
        self.dir_path = QFileDialog.getExistingDirectory(self, "Please choose folder", "../Test/test data/")
        self.get_overall_list()

        self.file_paths.clear()
        self.file_names.clear()
        self.displayed_file_names.clear()
        self.displayed_file_paths.clear()
        self.listWidget.clear()

        # view (*.spc) files only
        if self.file_type == 0:
            for root, dirs, files in os.walk(self.dir_path, topdown=False):
                for file in files:
                    if file[-4:] == ".spc":
                        data_path = os.path.join(root, file)
                        with open(data_path, 'rb') as input:
                            data = pickle.load(input)
                            data.path = data_path  # Change file path
                        self.displayed_file_names.append(file)
                        self.displayed_file_paths.append(data_path)
                        if data.data_.shape[0] == 1:
                            self.file_paths.append(data_path)
                            self.file_names.append(file)
                        else:
                            self.file_paths.append(data_path)
                            self.file_names.append(file)
                            for i in range(data.data_.shape[0]):
                                self.file_paths.append(data_path)
                                # name = file if i==0 else (file+"_"+str(i))
                                self.file_names.append('\t' + file + "_No" + str(i + 1))

        # view (*.dep) files only
        elif self.file_type == 1:
            for root, dirs, files in os.walk(self.dir_path, topdown=False):
                for file in files:
                    if file[-4:] == ".dep":
                        data_path = os.path.join(root, file)
                        with open(data_path, 'rb') as input:
                            data = pickle.load(input)
                            data.path = data_path  # Change file path
                        self.displayed_file_names.append(file)
                        self.displayed_file_paths.append(data_path)
                        self.file_paths.append(data_path)
                        self.file_names.append(file)

        if len(self.file_paths) <= 0:
            return

        self.lineEdit.setText(self.dir_path)
        self.listWidget.addItems(self.displayed_file_names)

        # refresh file list every second
        # self.timer.start(5000)
        # self.workThread.start()
        # self.workThread.trigger.connect(self.timer.stop)
        # self.timer.timeout.connect(self.refresh_file)

    # Data List | refresh file list
    def refresh_file(self):
        # print("refresh!!!")
        self.file_paths.clear()
        self.file_names.clear()
        self.displayed_file_names.clear()
        self.displayed_file_paths.clear()

        # view (*.spc) files only
        if self.file_type == 0:
            for root, dirs, files in os.walk(self.dir_path, topdown=False):
                for file in files:
                    if file[-4:] == ".spc":
                        data_path = os.path.join(root, file)
                        with open(data_path, 'rb') as input:
                            data = pickle.load(input)
                            data.path = data_path  # Change file path
                        self.displayed_file_names.append(file)
                        self.displayed_file_paths.append(data_path)
                        if data.data_.shape[0] == 1:
                            self.file_paths.append(data_path)
                            self.file_names.append(file)
                        else:
                            self.file_paths.append(data_path)
                            self.file_names.append(file)
                            for i in range(data.data_.shape[0]):
                                self.file_paths.append(data_path)
                                # name = file if i==0 else (file+"_"+str(i))
                                self.file_names.append('\t' + file + "_No" + str(i + 1))

        # view (*.dep) files only
        elif self.file_type == 1:
            for root, dirs, files in os.walk(self.dir_path, topdown=False):
                for file in files:
                    if file[-4:] == ".dep":
                        data_path = os.path.join(root, file)
                        with open(data_path, 'rb') as input:
                            data = pickle.load(input)
                            data.path = data_path  # Change file path
                        self.displayed_file_names.append(file)
                        self.displayed_file_paths.append(data_path)
                        self.file_paths.append(data_path)
                        self.file_names.append(file)

        if len(self.file_paths) <= 0:
            return

        self.listWidget.clear()
        self.listWidget.addItems(self.displayed_file_names)
        self.listWidget.setCurrentRow(self.file_index)

    # Data List | save as button slot
    def save_as(self):
        self.data_window = myDataWindow(self.overall_names, self.overall_paths)
        self.data_window.listWidget.list_changed_signal.connect(self.refresh_file)
        self.data_window.show()

    # Processing | open info button slot
    def show_info(self):
        if self.file_type == 0:
            self.spc_info.show()
        elif self.file_type == 1:
            self.dep_info.show()
            # init_spcInfo(self.data)
            # init_depInfo(self.data)

    # Windows | update previous/current window
    def focus_window_changed(self, old, new):
        ''' Note that current window is the window under focus, previous window is the last focused DataWindow '''
        if new != None:
            self.current_window = new.window()
        if old != None and old.window() != self.window() and old.window().objectName() != 'dockWidget':
            self.previous_window = old.window()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myDataProcessing()
    window.show()
    sys.exit(app.exec_())
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
        self.file_paths = []  # file paths in the list widget
        self.file_index = 0
        self.file_names = []  # file names in the list widget
        self.windows = []  # window object list
        self.window_names = []  # window names
        self.linecut_option = 1  # 0: option1, 1: option2

        self.current_window = None
        self.previous_window = None

        # signals |
        QApplication.instance().focusChanged.connect(self.focus_window_changed)

        # pushButton |
        self.pushButton_Open.clicked.connect(self.open_file)
        self.pushButton_Saveas.clicked.connect(self.save_as)
        # self.pushButton_Saveall.clicked.connect(self.save_all)
        # self.pushButton_Previous.clicked.connect(lambda: self.select_file(0))
        # self.pushButton_Next.clicked.connect(lambda: self.select_file(1))
        # self.pushButton_Refresh.clicked.connect(self.refresh_file)

        # listWidget |
        self.listWidget.setDragEnabled(True)
        self.listWidget.setDragDropOverwriteMode(False)
        self.listWidget.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.listWidget.setDefaultDropAction(Qt.MoveAction)

        self.listWidget.setContextMenuPolicy(Qt.CustomContextMenu)
        self.listWidget.customContextMenuRequested[QtCore.QPoint].connect(self.rightMenuShow)

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
        self.file_index = add_index[-1]

        # deal with multi-channel files
        multi_file_name = []
        for name in add_name:
            if name[0] != ' ':
                multi_file_name.append(name)
        for multi_name in multi_file_name:
            for name in self.file_names:
                if name.find(multi_name) != -1 and len(name)>12 and (name not in add_name):
                    add_name.append(name)

        self.previous_window.listWidget.addItems(add_name)
        self.previous_window.refresh_list()


    def open2window(self):
        pass

    # Data List | open folder button slot
    def open_file(self):
        self.dir_path = QFileDialog.getExistingDirectory(self, "Please choose folder", "../data/")

        self.file_paths.clear()
        self.file_names.clear()
        self.listWidget.clear()

        for root, dirs, files in os.walk(self.dir_path, topdown=False):
            for file in files:
                if file[-4:] == ".spc":
                    data_path = os.path.join(root, file)
                    with open(data_path, 'rb') as input:
                        data = pickle.load(input)
                        data.path = data_path  # Change file path
                    if data.data_.shape[0] == 1:
                        self.file_paths.append(data_path)
                        self.file_names.append(file)
                    else:
                        self.file_paths.append(data_path)
                        self.file_names.append(file)
                        for i in range(data.data_.shape[0]):
                            self.file_paths.append(data_path)
                            # name = file if i==0 else (file+"_"+str(i))
                            self.file_names.append("    " + file + "_No" + str(i + 1))

        if len(self.file_paths) <= 0:
            return

        self.lineEdit.setText(self.dir_path)
        self.listWidget.addItems(self.file_names)

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

        for root, dirs, files in os.walk(self.dir_path, topdown=False):
            for file in files:
                if file[-4:] == ".spc":
                    data_path = os.path.join(root, file)
                    with open(data_path, 'rb') as input:
                        data = pickle.load(input)
                        data.path = data_path  # Change file path
                    if data.data_.shape[0] == 1:
                        self.file_paths.append(data_path)
                        self.file_names.append(file)
                    else:
                        self.file_paths.append(data_path)
                        self.file_names.append(file)
                        for i in range(data.data_.shape[0]):
                            self.file_paths.append(data_path)
                            # name = file if i==0 else (file+"_"+str(i))
                            self.file_names.append("    " + file + "_No" + str(i + 1))

        if len(self.file_paths) <= 0:
            return

        self.listWidget.clear()
        self.listWidget.addItems(self.file_names)
        self.listWidget.setCurrentRow(self.file_index)

    # Data List | save as button slot
    def save_as(self):
        self.data_window = myDataWindow(self.file_names)
        self.data_window.listWidget.list_changed_signal.connect(self.refresh_file)
        self.data_window.show()

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
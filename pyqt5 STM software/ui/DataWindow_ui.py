# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DataWindow.ui'
#
# Created by: PyQt5 UI code generator 5.15.1
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_DataWindow(object):
    def setupUi(self, DataWindow):
        DataWindow.setObjectName("DataWindow")
        DataWindow.resize(481, 458)
        self.gridLayout_3 = QtWidgets.QGridLayout(DataWindow)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.groupBox = QtWidgets.QGroupBox(DataWindow)
        self.groupBox.setMaximumSize(QtCore.QSize(260, 16777215))
        self.groupBox.setObjectName("groupBox")
        self.gridLayout_3.addWidget(self.groupBox, 0, 0, 1, 1)
        self.widget = QtWidgets.QWidget(DataWindow)
        self.widget.setObjectName("widget")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.widget)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.graphicsView = GraphicsLayoutWidget(self.widget)
        self.graphicsView.setObjectName("graphicsView")
        self.gridLayout_2.addWidget(self.graphicsView, 0, 0, 1, 1)
        self.gridLayout_3.addWidget(self.widget, 0, 1, 1, 1)

        self.retranslateUi(DataWindow)
        QtCore.QMetaObject.connectSlotsByName(DataWindow)

    def retranslateUi(self, DataWindow):
        _translate = QtCore.QCoreApplication.translate
        DataWindow.setWindowTitle(_translate("DataWindow", "Data Window"))
        self.groupBox.setTitle(_translate("DataWindow", "Plot list"))
from pyqtgraph import GraphicsLayoutWidget

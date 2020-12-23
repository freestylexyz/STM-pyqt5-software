# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'pyqtGraph_test.ui'
#
# Created by: PyQt5 UI code generator 5.12.3
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_pgwidget(object):
    def setupUi(self, pgwidget):
        pgwidget.setObjectName("pgwidget")
        pgwidget.resize(679, 441)
        self.gridLayout = QtWidgets.QGridLayout(pgwidget)
        self.gridLayout.setObjectName("gridLayout")
        self.graphicsView = GraphicsLayoutWidget(pgwidget)
        self.graphicsView.setObjectName("graphicsView")
        self.gridLayout.addWidget(self.graphicsView, 0, 0, 1, 1)

        self.retranslateUi(pgwidget)
        QtCore.QMetaObject.connectSlotsByName(pgwidget)

    def retranslateUi(self, pgwidget):
        _translate = QtCore.QCoreApplication.translate
        pgwidget.setWindowTitle(_translate("pgwidget", "pyqtGraph_test"))
from pyqtgraph import GraphicsLayoutWidget

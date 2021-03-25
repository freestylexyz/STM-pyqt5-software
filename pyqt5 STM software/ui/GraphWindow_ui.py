# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'GraphWindow.ui'
#
# Created by: PyQt5 UI code generator 5.15.1
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_Graph(object):
    def setupUi(self, Graph):
        Graph.setObjectName("Graph")
        Graph.resize(534, 592)
        self.gridLayout = QtWidgets.QGridLayout(Graph)
        self.gridLayout.setObjectName("gridLayout")
        self.graphicsView = GraphicsLayoutWidget(Graph)
        self.graphicsView.setMinimumSize(QtCore.QSize(512, 512))
        self.graphicsView.setMaximumSize(QtCore.QSize(512, 512))
        self.graphicsView.setObjectName("graphicsView")
        self.gridLayout.addWidget(self.graphicsView, 1, 0, 1, 1)
        self.widget = QtWidgets.QWidget(Graph)
        self.widget.setObjectName("widget")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.widget)
        self.horizontalLayout.setSpacing(7)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.pushButton_Rotate = QtWidgets.QPushButton(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Rotate.sizePolicy().hasHeightForWidth())
        self.pushButton_Rotate.setSizePolicy(sizePolicy)
        self.pushButton_Rotate.setText("")
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/toolbar/data/rotate.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.pushButton_Rotate.setIcon(icon)
        self.pushButton_Rotate.setFlat(True)
        self.pushButton_Rotate.setObjectName("pushButton_Rotate")
        self.horizontalLayout.addWidget(self.pushButton_Rotate)
        self.checkBox_PlaneFit = QtWidgets.QCheckBox(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.checkBox_PlaneFit.sizePolicy().hasHeightForWidth())
        self.checkBox_PlaneFit.setSizePolicy(sizePolicy)
        self.checkBox_PlaneFit.setObjectName("checkBox_PlaneFit")
        self.horizontalLayout.addWidget(self.checkBox_PlaneFit)
        self.checkBox_Illuminated = QtWidgets.QCheckBox(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.checkBox_Illuminated.sizePolicy().hasHeightForWidth())
        self.checkBox_Illuminated.setSizePolicy(sizePolicy)
        self.checkBox_Illuminated.setObjectName("checkBox_Illuminated")
        self.horizontalLayout.addWidget(self.checkBox_Illuminated)
        self.checkBox_Reverse = QtWidgets.QCheckBox(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.checkBox_Reverse.sizePolicy().hasHeightForWidth())
        self.checkBox_Reverse.setSizePolicy(sizePolicy)
        self.checkBox_Reverse.setObjectName("checkBox_Reverse")
        self.horizontalLayout.addWidget(self.checkBox_Reverse)
        self.radioButton_Gray = QtWidgets.QRadioButton(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.radioButton_Gray.sizePolicy().hasHeightForWidth())
        self.radioButton_Gray.setSizePolicy(sizePolicy)
        self.radioButton_Gray.setChecked(True)
        self.radioButton_Gray.setObjectName("radioButton_Gray")
        self.horizontalLayout.addWidget(self.radioButton_Gray)
        self.radioButton_Color = QtWidgets.QRadioButton(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.radioButton_Color.sizePolicy().hasHeightForWidth())
        self.radioButton_Color.setSizePolicy(sizePolicy)
        self.radioButton_Color.setObjectName("radioButton_Color")
        self.horizontalLayout.addWidget(self.radioButton_Color)
        self.gridLayout.addWidget(self.widget, 0, 0, 1, 1)

        self.retranslateUi(Graph)
        QtCore.QMetaObject.connectSlotsByName(Graph)

    def retranslateUi(self, Graph):
        _translate = QtCore.QCoreApplication.translate
        Graph.setWindowTitle(_translate("Graph", "Graph"))
        self.checkBox_PlaneFit.setText(_translate("Graph", "Planefit"))
        self.checkBox_Illuminated.setText(_translate("Graph", "Illu."))
        self.checkBox_Reverse.setText(_translate("Graph", "Reverse"))
        self.radioButton_Gray.setText(_translate("Graph", "Gray"))
        self.radioButton_Color.setText(_translate("Graph", "Color"))
from pyqtgraph import GraphicsLayoutWidget
import logo_rc

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'Spectroscopy.ui'
#
# Created by: PyQt5 UI code generator 5.15.1
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_Spectroscopy(object):
    def setupUi(self, Spectroscopy):
        Spectroscopy.setObjectName("Spectroscopy")
        Spectroscopy.resize(532, 563)
        self.gridLayout_6 = QtWidgets.QGridLayout(Spectroscopy)
        self.gridLayout_6.setObjectName("gridLayout_6")
        self.widget_2 = QtWidgets.QWidget(Spectroscopy)
        self.widget_2.setObjectName("widget_2")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.widget_2)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.graphicsView_2 = QtWidgets.QGraphicsView(self.widget_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.graphicsView_2.sizePolicy().hasHeightForWidth())
        self.graphicsView_2.setSizePolicy(sizePolicy)
        self.graphicsView_2.setMinimumSize(QtCore.QSize(180, 180))
        self.graphicsView_2.setObjectName("graphicsView_2")
        self.gridLayout_2.addWidget(self.graphicsView_2, 0, 0, 2, 2)
        self.horizontalScrollBar_2 = QtWidgets.QScrollBar(self.widget_2)
        self.horizontalScrollBar_2.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalScrollBar_2.setObjectName("horizontalScrollBar_2")
        self.gridLayout_2.addWidget(self.horizontalScrollBar_2, 2, 0, 1, 2)
        self.verticalScrollBar_2 = QtWidgets.QScrollBar(self.widget_2)
        self.verticalScrollBar_2.setOrientation(QtCore.Qt.Vertical)
        self.verticalScrollBar_2.setObjectName("verticalScrollBar_2")
        self.gridLayout_2.addWidget(self.verticalScrollBar_2, 0, 2, 2, 1)
        self.gridLayout_6.addWidget(self.widget_2, 2, 0, 1, 1)
        self.groupBox = QtWidgets.QGroupBox(Spectroscopy)
        self.groupBox.setObjectName("groupBox")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.groupBox)
        self.gridLayout_3.setSpacing(9)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.label = QtWidgets.QLabel(self.groupBox)
        self.label.setObjectName("label")
        self.gridLayout_3.addWidget(self.label, 0, 0, 1, 1)
        self.spinBox = QtWidgets.QSpinBox(self.groupBox)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox.sizePolicy().hasHeightForWidth())
        self.spinBox.setSizePolicy(sizePolicy)
        self.spinBox.setMinimumSize(QtCore.QSize(60, 20))
        self.spinBox.setObjectName("spinBox")
        self.gridLayout_3.addWidget(self.spinBox, 0, 1, 1, 1)
        self.horizontalScrollBar_3 = QtWidgets.QScrollBar(self.groupBox)
        self.horizontalScrollBar_3.setMinimumSize(QtCore.QSize(120, 0))
        self.horizontalScrollBar_3.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalScrollBar_3.setObjectName("horizontalScrollBar_3")
        self.gridLayout_3.addWidget(self.horizontalScrollBar_3, 0, 2, 1, 2)
        self.label_2 = QtWidgets.QLabel(self.groupBox)
        self.label_2.setObjectName("label_2")
        self.gridLayout_3.addWidget(self.label_2, 1, 0, 1, 1)
        self.spinBox_2 = QtWidgets.QSpinBox(self.groupBox)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_2.sizePolicy().hasHeightForWidth())
        self.spinBox_2.setSizePolicy(sizePolicy)
        self.spinBox_2.setMinimumSize(QtCore.QSize(60, 20))
        self.spinBox_2.setObjectName("spinBox_2")
        self.gridLayout_3.addWidget(self.spinBox_2, 1, 1, 1, 1)
        self.horizontalScrollBar_4 = QtWidgets.QScrollBar(self.groupBox)
        self.horizontalScrollBar_4.setMinimumSize(QtCore.QSize(120, 0))
        self.horizontalScrollBar_4.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalScrollBar_4.setObjectName("horizontalScrollBar_4")
        self.gridLayout_3.addWidget(self.horizontalScrollBar_4, 1, 2, 1, 2)
        self.label_3 = QtWidgets.QLabel(self.groupBox)
        self.label_3.setObjectName("label_3")
        self.gridLayout_3.addWidget(self.label_3, 2, 0, 1, 1)
        self.horizontalScrollBar_5 = QtWidgets.QScrollBar(self.groupBox)
        self.horizontalScrollBar_5.setMinimumSize(QtCore.QSize(120, 0))
        self.horizontalScrollBar_5.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalScrollBar_5.setObjectName("horizontalScrollBar_5")
        self.gridLayout_3.addWidget(self.horizontalScrollBar_5, 2, 1, 1, 2)
        self.pushButton = QtWidgets.QPushButton(self.groupBox)
        self.pushButton.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton.setObjectName("pushButton")
        self.gridLayout_3.addWidget(self.pushButton, 2, 3, 1, 1)
        self.label_4 = QtWidgets.QLabel(self.groupBox)
        self.label_4.setObjectName("label_4")
        self.gridLayout_3.addWidget(self.label_4, 3, 0, 1, 1)
        self.horizontalScrollBar_6 = QtWidgets.QScrollBar(self.groupBox)
        self.horizontalScrollBar_6.setMinimumSize(QtCore.QSize(120, 0))
        self.horizontalScrollBar_6.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalScrollBar_6.setObjectName("horizontalScrollBar_6")
        self.gridLayout_3.addWidget(self.horizontalScrollBar_6, 3, 1, 1, 2)
        self.pushButton_2 = QtWidgets.QPushButton(self.groupBox)
        self.pushButton_2.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_2.setObjectName("pushButton_2")
        self.gridLayout_3.addWidget(self.pushButton_2, 3, 3, 2, 1)
        self.label_5 = QtWidgets.QLabel(self.groupBox)
        self.label_5.setObjectName("label_5")
        self.gridLayout_3.addWidget(self.label_5, 4, 0, 2, 1)
        self.horizontalScrollBar_7 = QtWidgets.QScrollBar(self.groupBox)
        self.horizontalScrollBar_7.setMinimumSize(QtCore.QSize(120, 0))
        self.horizontalScrollBar_7.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalScrollBar_7.setObjectName("horizontalScrollBar_7")
        self.gridLayout_3.addWidget(self.horizontalScrollBar_7, 5, 1, 1, 3)
        self.gridLayout_6.addWidget(self.groupBox, 0, 1, 1, 1)
        self.groupBox_2 = QtWidgets.QGroupBox(Spectroscopy)
        self.groupBox_2.setObjectName("groupBox_2")
        self.gridLayout_5 = QtWidgets.QGridLayout(self.groupBox_2)
        self.gridLayout_5.setSpacing(9)
        self.gridLayout_5.setObjectName("gridLayout_5")
        self.label_6 = QtWidgets.QLabel(self.groupBox_2)
        self.label_6.setObjectName("label_6")
        self.gridLayout_5.addWidget(self.label_6, 0, 0, 1, 1)
        self.label_7 = QtWidgets.QLabel(self.groupBox_2)
        self.label_7.setObjectName("label_7")
        self.gridLayout_5.addWidget(self.label_7, 1, 0, 1, 1)
        self.label_8 = QtWidgets.QLabel(self.groupBox_2)
        self.label_8.setObjectName("label_8")
        self.gridLayout_5.addWidget(self.label_8, 2, 0, 1, 1)
        self.pushButton_4 = QtWidgets.QPushButton(self.groupBox_2)
        self.pushButton_4.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_4.setObjectName("pushButton_4")
        self.gridLayout_5.addWidget(self.pushButton_4, 6, 0, 1, 2)
        self.spinBox_3 = QtWidgets.QSpinBox(self.groupBox_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_3.sizePolicy().hasHeightForWidth())
        self.spinBox_3.setSizePolicy(sizePolicy)
        self.spinBox_3.setMinimumSize(QtCore.QSize(60, 20))
        self.spinBox_3.setObjectName("spinBox_3")
        self.gridLayout_5.addWidget(self.spinBox_3, 0, 1, 1, 1)
        self.label_9 = QtWidgets.QLabel(self.groupBox_2)
        self.label_9.setObjectName("label_9")
        self.gridLayout_5.addWidget(self.label_9, 3, 0, 1, 1)
        self.spinBox_5 = QtWidgets.QSpinBox(self.groupBox_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_5.sizePolicy().hasHeightForWidth())
        self.spinBox_5.setSizePolicy(sizePolicy)
        self.spinBox_5.setMinimumSize(QtCore.QSize(60, 20))
        self.spinBox_5.setObjectName("spinBox_5")
        self.gridLayout_5.addWidget(self.spinBox_5, 2, 1, 1, 1)
        self.horizontalScrollBar_8 = QtWidgets.QScrollBar(self.groupBox_2)
        self.horizontalScrollBar_8.setMinimumSize(QtCore.QSize(120, 0))
        self.horizontalScrollBar_8.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalScrollBar_8.setObjectName("horizontalScrollBar_8")
        self.gridLayout_5.addWidget(self.horizontalScrollBar_8, 0, 2, 1, 2)
        self.groupBox_3 = QtWidgets.QGroupBox(self.groupBox_2)
        self.groupBox_3.setCheckable(True)
        self.groupBox_3.setObjectName("groupBox_3")
        self.gridLayout_4 = QtWidgets.QGridLayout(self.groupBox_3)
        self.gridLayout_4.setObjectName("gridLayout_4")
        self.spinBox_7 = QtWidgets.QSpinBox(self.groupBox_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_7.sizePolicy().hasHeightForWidth())
        self.spinBox_7.setSizePolicy(sizePolicy)
        self.spinBox_7.setMinimumSize(QtCore.QSize(60, 20))
        self.spinBox_7.setObjectName("spinBox_7")
        self.gridLayout_4.addWidget(self.spinBox_7, 0, 0, 1, 1)
        self.horizontalScrollBar_11 = QtWidgets.QScrollBar(self.groupBox_3)
        self.horizontalScrollBar_11.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalScrollBar_11.setObjectName("horizontalScrollBar_11")
        self.gridLayout_4.addWidget(self.horizontalScrollBar_11, 0, 1, 1, 2)
        self.label_11 = QtWidgets.QLabel(self.groupBox_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_11.sizePolicy().hasHeightForWidth())
        self.label_11.setSizePolicy(sizePolicy)
        self.label_11.setObjectName("label_11")
        self.gridLayout_4.addWidget(self.label_11, 1, 0, 1, 2)
        self.label_12 = QtWidgets.QLabel(self.groupBox_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_12.sizePolicy().hasHeightForWidth())
        self.label_12.setSizePolicy(sizePolicy)
        self.label_12.setMinimumSize(QtCore.QSize(0, 20))
        self.label_12.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.label_12.setFrameShadow(QtWidgets.QFrame.Sunken)
        self.label_12.setText("")
        self.label_12.setObjectName("label_12")
        self.gridLayout_4.addWidget(self.label_12, 1, 2, 1, 1)
        self.gridLayout_5.addWidget(self.groupBox_3, 4, 0, 1, 4)
        self.pushButton_6 = QtWidgets.QPushButton(self.groupBox_2)
        self.pushButton_6.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_6.setObjectName("pushButton_6")
        self.gridLayout_5.addWidget(self.pushButton_6, 7, 0, 1, 2)
        self.horizontalScrollBar_10 = QtWidgets.QScrollBar(self.groupBox_2)
        self.horizontalScrollBar_10.setMinimumSize(QtCore.QSize(120, 0))
        self.horizontalScrollBar_10.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalScrollBar_10.setObjectName("horizontalScrollBar_10")
        self.gridLayout_5.addWidget(self.horizontalScrollBar_10, 2, 2, 1, 2)
        self.spinBox_6 = QtWidgets.QSpinBox(self.groupBox_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_6.sizePolicy().hasHeightForWidth())
        self.spinBox_6.setSizePolicy(sizePolicy)
        self.spinBox_6.setMinimumSize(QtCore.QSize(60, 20))
        self.spinBox_6.setObjectName("spinBox_6")
        self.gridLayout_5.addWidget(self.spinBox_6, 3, 1, 1, 1)
        self.checkBox = QtWidgets.QCheckBox(self.groupBox_2)
        self.checkBox.setObjectName("checkBox")
        self.gridLayout_5.addWidget(self.checkBox, 5, 0, 1, 2)
        self.pushButton_5 = QtWidgets.QPushButton(self.groupBox_2)
        self.pushButton_5.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_5.setObjectName("pushButton_5")
        self.gridLayout_5.addWidget(self.pushButton_5, 6, 2, 1, 2)
        self.pushButton_3 = QtWidgets.QPushButton(self.groupBox_2)
        self.pushButton_3.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_3.setObjectName("pushButton_3")
        self.gridLayout_5.addWidget(self.pushButton_3, 5, 2, 1, 2)
        self.label_10 = QtWidgets.QLabel(self.groupBox_2)
        self.label_10.setObjectName("label_10")
        self.gridLayout_5.addWidget(self.label_10, 3, 2, 1, 1)
        self.horizontalScrollBar_9 = QtWidgets.QScrollBar(self.groupBox_2)
        self.horizontalScrollBar_9.setMinimumSize(QtCore.QSize(120, 0))
        self.horizontalScrollBar_9.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalScrollBar_9.setObjectName("horizontalScrollBar_9")
        self.gridLayout_5.addWidget(self.horizontalScrollBar_9, 1, 2, 1, 2)
        self.comboBox = QtWidgets.QComboBox(self.groupBox_2)
        self.comboBox.setObjectName("comboBox")
        self.gridLayout_5.addWidget(self.comboBox, 3, 3, 1, 1)
        self.pushButton_8 = QtWidgets.QPushButton(self.groupBox_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_8.sizePolicy().hasHeightForWidth())
        self.pushButton_8.setSizePolicy(sizePolicy)
        self.pushButton_8.setObjectName("pushButton_8")
        self.gridLayout_5.addWidget(self.pushButton_8, 7, 2, 2, 2)
        self.spinBox_4 = QtWidgets.QSpinBox(self.groupBox_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_4.sizePolicy().hasHeightForWidth())
        self.spinBox_4.setSizePolicy(sizePolicy)
        self.spinBox_4.setMinimumSize(QtCore.QSize(60, 20))
        self.spinBox_4.setObjectName("spinBox_4")
        self.gridLayout_5.addWidget(self.spinBox_4, 1, 1, 1, 1)
        self.pushButton_7 = QtWidgets.QPushButton(self.groupBox_2)
        self.pushButton_7.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_7.setObjectName("pushButton_7")
        self.gridLayout_5.addWidget(self.pushButton_7, 8, 0, 1, 2)
        self.gridLayout_6.addWidget(self.groupBox_2, 1, 1, 2, 1)
        self.widget = QtWidgets.QWidget(Spectroscopy)
        self.widget.setObjectName("widget")
        self.gridLayout = QtWidgets.QGridLayout(self.widget)
        self.gridLayout.setObjectName("gridLayout")
        self.graphicsView = QtWidgets.QGraphicsView(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.graphicsView.sizePolicy().hasHeightForWidth())
        self.graphicsView.setSizePolicy(sizePolicy)
        self.graphicsView.setMinimumSize(QtCore.QSize(180, 180))
        self.graphicsView.setObjectName("graphicsView")
        self.gridLayout.addWidget(self.graphicsView, 0, 0, 1, 1)
        self.verticalScrollBar = QtWidgets.QScrollBar(self.widget)
        self.verticalScrollBar.setOrientation(QtCore.Qt.Vertical)
        self.verticalScrollBar.setObjectName("verticalScrollBar")
        self.gridLayout.addWidget(self.verticalScrollBar, 0, 1, 1, 1)
        self.horizontalScrollBar = QtWidgets.QScrollBar(self.widget)
        self.horizontalScrollBar.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalScrollBar.setObjectName("horizontalScrollBar")
        self.gridLayout.addWidget(self.horizontalScrollBar, 1, 0, 1, 1)
        self.gridLayout_6.addWidget(self.widget, 0, 0, 2, 1)

        self.retranslateUi(Spectroscopy)
        QtCore.QMetaObject.connectSlotsByName(Spectroscopy)

    def retranslateUi(self, Spectroscopy):
        _translate = QtCore.QCoreApplication.translate
        Spectroscopy.setWindowTitle(_translate("Spectroscopy", "Spectroscopy"))
        self.groupBox.setTitle(_translate("Spectroscopy", "Graph"))
        self.label.setText(_translate("Spectroscopy", "Pointer 1"))
        self.label_2.setText(_translate("Spectroscopy", "Pointer 2"))
        self.label_3.setText(_translate("Spectroscopy", "X scale"))
        self.pushButton.setText(_translate("Spectroscopy", "Auto"))
        self.label_4.setText(_translate("Spectroscopy", "Y scale"))
        self.pushButton_2.setText(_translate("Spectroscopy", "Auto"))
        self.label_5.setText(_translate("Spectroscopy", "Channel"))
        self.groupBox_2.setTitle(_translate("Spectroscopy", "General options"))
        self.label_6.setText(_translate("Spectroscopy", "Min"))
        self.label_7.setText(_translate("Spectroscopy", "Max"))
        self.label_8.setText(_translate("Spectroscopy", "Step size"))
        self.pushButton_4.setText(_translate("Spectroscopy", " Advance option"))
        self.label_9.setText(_translate("Spectroscopy", "Pass #"))
        self.groupBox_3.setTitle(_translate("Spectroscopy", "Delta"))
        self.label_11.setText(_translate("Spectroscopy", "Channel:"))
        self.pushButton_6.setText(_translate("Spectroscopy", "Save"))
        self.checkBox.setText(_translate("Spectroscopy", "Mapping"))
        self.pushButton_5.setText(_translate("Spectroscopy", "Between passes"))
        self.pushButton_3.setText(_translate("Spectroscopy", "Edit points"))
        self.label_10.setText(_translate("Spectroscopy", "Ramp Ch"))
        self.pushButton_8.setText(_translate("Spectroscopy", "Scan"))
        self.pushButton_7.setText(_translate("Spectroscopy", "Info"))

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'SequenceList.ui'
#
# Created by: PyQt5 UI code generator 5.15.1
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_SequenceList(object):
    def setupUi(self, SequenceList):
        SequenceList.setObjectName("SequenceList")
        SequenceList.resize(479, 318)
        self.gridLayout_2 = QtWidgets.QGridLayout(SequenceList)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.widget_2 = QtWidgets.QWidget(SequenceList)
        self.widget_2.setObjectName("widget_2")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.widget_2)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.listView = QtWidgets.QListView(self.widget_2)
        self.listView.setObjectName("listView")
        self.gridLayout_3.addWidget(self.listView, 0, 0, 1, 1)
        self.gridLayout_2.addWidget(self.widget_2, 0, 0, 1, 1)
        self.widget = QtWidgets.QWidget(SequenceList)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.widget.sizePolicy().hasHeightForWidth())
        self.widget.setSizePolicy(sizePolicy)
        self.widget.setMinimumSize(QtCore.QSize(180, 300))
        self.widget.setObjectName("widget")
        self.gridLayout = QtWidgets.QGridLayout(self.widget)
        self.gridLayout.setVerticalSpacing(14)
        self.gridLayout.setObjectName("gridLayout")
        self.pushButton = QtWidgets.QPushButton(self.widget)
        self.pushButton.setMinimumSize(QtCore.QSize(160, 36))
        self.pushButton.setObjectName("pushButton")
        self.gridLayout.addWidget(self.pushButton, 0, 0, 1, 1)
        self.pushButton_2 = QtWidgets.QPushButton(self.widget)
        self.pushButton_2.setMinimumSize(QtCore.QSize(160, 36))
        self.pushButton_2.setObjectName("pushButton_2")
        self.gridLayout.addWidget(self.pushButton_2, 1, 0, 1, 1)
        self.pushButton_3 = QtWidgets.QPushButton(self.widget)
        self.pushButton_3.setMinimumSize(QtCore.QSize(160, 36))
        self.pushButton_3.setObjectName("pushButton_3")
        self.gridLayout.addWidget(self.pushButton_3, 2, 0, 1, 1)
        self.pushButton_4 = QtWidgets.QPushButton(self.widget)
        self.pushButton_4.setMinimumSize(QtCore.QSize(160, 36))
        self.pushButton_4.setObjectName("pushButton_4")
        self.gridLayout.addWidget(self.pushButton_4, 3, 0, 1, 1)
        self.pushButton_5 = QtWidgets.QPushButton(self.widget)
        self.pushButton_5.setMinimumSize(QtCore.QSize(160, 36))
        self.pushButton_5.setObjectName("pushButton_5")
        self.gridLayout.addWidget(self.pushButton_5, 4, 0, 1, 1)
        self.gridLayout_2.addWidget(self.widget, 0, 1, 1, 1)

        self.retranslateUi(SequenceList)
        QtCore.QMetaObject.connectSlotsByName(SequenceList)

    def retranslateUi(self, SequenceList):
        _translate = QtCore.QCoreApplication.translate
        SequenceList.setWindowTitle(_translate("SequenceList", "Sequence List"))
        self.pushButton.setText(_translate("SequenceList", "Load Sequence to list"))
        self.pushButton_2.setText(_translate("SequenceList", "Build New Sequence"))
        self.pushButton_3.setText(_translate("SequenceList", "Edit Current Sequence"))
        self.pushButton_4.setText(_translate("SequenceList", "Remove Current Sequence"))
        self.pushButton_5.setText(_translate("SequenceList", "Select Current Sequence"))

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DataProcessing.ui'
#
# Created by: PyQt5 UI code generator 5.15.1
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_DataProcessing(object):
    def setupUi(self, DataProcessing):
        DataProcessing.setObjectName("DataProcessing")
        DataProcessing.resize(477, 492)
        self.centralwidget = QtWidgets.QWidget(DataProcessing)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout_5 = QtWidgets.QGridLayout(self.centralwidget)
        self.gridLayout_5.setObjectName("gridLayout_5")
        self.groupBox_3 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_3.setMinimumSize(QtCore.QSize(0, 0))
        self.groupBox_3.setMaximumSize(QtCore.QSize(260, 16777215))
        self.groupBox_3.setObjectName("groupBox_3")
        self.gridLayout_4 = QtWidgets.QGridLayout(self.groupBox_3)
        self.gridLayout_4.setObjectName("gridLayout_4")
        self.pushButton_Open = QtWidgets.QPushButton(self.groupBox_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Open.sizePolicy().hasHeightForWidth())
        self.pushButton_Open.setSizePolicy(sizePolicy)
        self.pushButton_Open.setText("")
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/toolbar/data/open.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.pushButton_Open.setIcon(icon)
        self.pushButton_Open.setFlat(True)
        self.pushButton_Open.setObjectName("pushButton_Open")
        self.gridLayout_4.addWidget(self.pushButton_Open, 0, 4, 1, 1)
        self.listWidget = QtWidgets.QListWidget(self.groupBox_3)
        self.listWidget.setObjectName("listWidget")
        self.gridLayout_4.addWidget(self.listWidget, 3, 0, 1, 5)
        self.pushButton_Saveas = QtWidgets.QPushButton(self.groupBox_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Saveas.sizePolicy().hasHeightForWidth())
        self.pushButton_Saveas.setSizePolicy(sizePolicy)
        self.pushButton_Saveas.setText("")
        icon1 = QtGui.QIcon()
        icon1.addPixmap(QtGui.QPixmap(":/toolbar/data/save.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.pushButton_Saveas.setIcon(icon1)
        self.pushButton_Saveas.setFlat(True)
        self.pushButton_Saveas.setObjectName("pushButton_Saveas")
        self.gridLayout_4.addWidget(self.pushButton_Saveas, 1, 0, 1, 1)
        self.pushButton_Previous = QtWidgets.QPushButton(self.groupBox_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Previous.sizePolicy().hasHeightForWidth())
        self.pushButton_Previous.setSizePolicy(sizePolicy)
        self.pushButton_Previous.setText("")
        icon2 = QtGui.QIcon()
        icon2.addPixmap(QtGui.QPixmap(":/toolbar/data/left.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.pushButton_Previous.setIcon(icon2)
        self.pushButton_Previous.setFlat(True)
        self.pushButton_Previous.setObjectName("pushButton_Previous")
        self.gridLayout_4.addWidget(self.pushButton_Previous, 1, 3, 1, 1)
        self.pushButton_Refresh = QtWidgets.QPushButton(self.groupBox_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Refresh.sizePolicy().hasHeightForWidth())
        self.pushButton_Refresh.setSizePolicy(sizePolicy)
        self.pushButton_Refresh.setText("")
        icon3 = QtGui.QIcon()
        icon3.addPixmap(QtGui.QPixmap(":/toolbar/data/Reset.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.pushButton_Refresh.setIcon(icon3)
        self.pushButton_Refresh.setFlat(True)
        self.pushButton_Refresh.setObjectName("pushButton_Refresh")
        self.gridLayout_4.addWidget(self.pushButton_Refresh, 1, 2, 1, 1)
        self.pushButton_Next = QtWidgets.QPushButton(self.groupBox_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Next.sizePolicy().hasHeightForWidth())
        self.pushButton_Next.setSizePolicy(sizePolicy)
        self.pushButton_Next.setText("")
        icon4 = QtGui.QIcon()
        icon4.addPixmap(QtGui.QPixmap(":/toolbar/data/right.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.pushButton_Next.setIcon(icon4)
        self.pushButton_Next.setFlat(True)
        self.pushButton_Next.setObjectName("pushButton_Next")
        self.gridLayout_4.addWidget(self.pushButton_Next, 1, 4, 1, 1)
        self.lineEdit = QtWidgets.QLineEdit(self.groupBox_3)
        self.lineEdit.setObjectName("lineEdit")
        self.gridLayout_4.addWidget(self.lineEdit, 0, 0, 1, 4)
        self.pushButton_Saveall = QtWidgets.QPushButton(self.groupBox_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Saveall.sizePolicy().hasHeightForWidth())
        self.pushButton_Saveall.setSizePolicy(sizePolicy)
        self.pushButton_Saveall.setMouseTracking(True)
        self.pushButton_Saveall.setStatusTip("")
        self.pushButton_Saveall.setAccessibleName("")
        self.pushButton_Saveall.setAccessibleDescription("")
        self.pushButton_Saveall.setAutoFillBackground(False)
        self.pushButton_Saveall.setText("")
        icon5 = QtGui.QIcon()
        icon5.addPixmap(QtGui.QPixmap(":/toolbar/data/saveas.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.pushButton_Saveall.setIcon(icon5)
        self.pushButton_Saveall.setFlat(True)
        self.pushButton_Saveall.setObjectName("pushButton_Saveall")
        self.gridLayout_4.addWidget(self.pushButton_Saveall, 1, 1, 1, 1)
        self.comboBox = QtWidgets.QComboBox(self.groupBox_3)
        self.comboBox.setObjectName("comboBox")
        self.comboBox.addItem("")
        self.comboBox.addItem("")
        self.gridLayout_4.addWidget(self.comboBox, 2, 0, 1, 5)
        self.gridLayout_5.addWidget(self.groupBox_3, 0, 0, 1, 1)
        self.groupBox_4 = QtWidgets.QGroupBox(self.centralwidget)
        self.groupBox_4.setObjectName("groupBox_4")
        self.gridLayout_6 = QtWidgets.QGridLayout(self.groupBox_4)
        self.gridLayout_6.setObjectName("gridLayout_6")
        self.pushButton_2 = QtWidgets.QPushButton(self.groupBox_4)
        self.pushButton_2.setObjectName("pushButton_2")
        self.gridLayout_6.addWidget(self.pushButton_2, 0, 0, 1, 1)
        self.gridLayout_5.addWidget(self.groupBox_4, 1, 0, 1, 1)
        DataProcessing.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(DataProcessing)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 477, 26))
        self.menubar.setObjectName("menubar")
        DataProcessing.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(DataProcessing)
        self.statusbar.setObjectName("statusbar")
        DataProcessing.setStatusBar(self.statusbar)
        self.dockWidget = QtWidgets.QDockWidget(DataProcessing)
        self.dockWidget.setObjectName("dockWidget")
        self.dockWidgetContents = QtWidgets.QWidget()
        self.dockWidgetContents.setObjectName("dockWidgetContents")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.dockWidgetContents)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.groupBox = QtWidgets.QGroupBox(self.dockWidgetContents)
        self.groupBox.setObjectName("groupBox")
        self.gridLayout = QtWidgets.QGridLayout(self.groupBox)
        self.gridLayout.setObjectName("gridLayout")
        self.pushButton = QtWidgets.QPushButton(self.groupBox)
        self.pushButton.setObjectName("pushButton")
        self.gridLayout.addWidget(self.pushButton, 0, 0, 1, 1)
        self.gridLayout_3.addWidget(self.groupBox, 0, 0, 1, 1)
        self.groupBox_2 = QtWidgets.QGroupBox(self.dockWidgetContents)
        self.groupBox_2.setObjectName("groupBox_2")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.groupBox_2)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.checkBox = QtWidgets.QCheckBox(self.groupBox_2)
        self.checkBox.setObjectName("checkBox")
        self.gridLayout_2.addWidget(self.checkBox, 0, 0, 1, 1)
        self.gridLayout_3.addWidget(self.groupBox_2, 1, 0, 1, 1)
        self.dockWidget.setWidget(self.dockWidgetContents)
        DataProcessing.addDockWidget(QtCore.Qt.DockWidgetArea(1), self.dockWidget)

        self.retranslateUi(DataProcessing)
        QtCore.QMetaObject.connectSlotsByName(DataProcessing)

    def retranslateUi(self, DataProcessing):
        _translate = QtCore.QCoreApplication.translate
        DataProcessing.setWindowTitle(_translate("DataProcessing", "Data Processing"))
        self.groupBox_3.setTitle(_translate("DataProcessing", "Data list"))
        self.pushButton_Saveall.setWhatsThis(_translate("DataProcessing", "Save all"))
        self.comboBox.setItemText(0, _translate("DataProcessing", "Show .spc only"))
        self.comboBox.setItemText(1, _translate("DataProcessing", "Show .dep only"))
        self.groupBox_4.setTitle(_translate("DataProcessing", "Info"))
        self.pushButton_2.setText(_translate("DataProcessing", "Open info"))
        self.dockWidget.setWindowTitle(_translate("DataProcessing", "Processing"))
        self.groupBox.setTitle(_translate("DataProcessing", "Copy to new window"))
        self.pushButton.setText(_translate("DataProcessing", "Do it"))
        self.groupBox_2.setTitle(_translate("DataProcessing", "Select data"))
        self.checkBox.setText(_translate("DataProcessing", "Show scanner"))
import logo_rc

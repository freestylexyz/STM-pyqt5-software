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
        Spectroscopy.setWindowModality(QtCore.Qt.NonModal)
        Spectroscopy.resize(778, 759)
        self.gridLayout = QtWidgets.QGridLayout(Spectroscopy)
        self.gridLayout.setObjectName("gridLayout")
        self.pushButton_Scan = QtWidgets.QPushButton(Spectroscopy)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Scan.sizePolicy().hasHeightForWidth())
        self.pushButton_Scan.setSizePolicy(sizePolicy)
        self.pushButton_Scan.setMaximumSize(QtCore.QSize(250, 16777215))
        self.pushButton_Scan.setObjectName("pushButton_Scan")
        self.gridLayout.addWidget(self.pushButton_Scan, 2, 3, 2, 1)
        self.pushButton_Info = QtWidgets.QPushButton(Spectroscopy)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Info.sizePolicy().hasHeightForWidth())
        self.pushButton_Info.setSizePolicy(sizePolicy)
        self.pushButton_Info.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_Info.setObjectName("pushButton_Info")
        self.gridLayout.addWidget(self.pushButton_Info, 3, 2, 1, 1)
        self.General = QtWidgets.QGroupBox(Spectroscopy)
        self.General.setObjectName("General")
        self.gridLayout_5 = QtWidgets.QGridLayout(self.General)
        self.gridLayout_5.setSpacing(9)
        self.gridLayout_5.setObjectName("gridLayout_5")
        self.pushButton_LockIn_General = QtWidgets.QPushButton(self.General)
        self.pushButton_LockIn_General.setObjectName("pushButton_LockIn_General")
        self.gridLayout_5.addWidget(self.pushButton_LockIn_General, 8, 2, 1, 2)
        self.spinBox_Pass_General = QtWidgets.QSpinBox(self.General)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_Pass_General.sizePolicy().hasHeightForWidth())
        self.spinBox_Pass_General.setSizePolicy(sizePolicy)
        self.spinBox_Pass_General.setMinimumSize(QtCore.QSize(60, 20))
        self.spinBox_Pass_General.setMinimum(1)
        self.spinBox_Pass_General.setMaximum(1024)
        self.spinBox_Pass_General.setObjectName("spinBox_Pass_General")
        self.gridLayout_5.addWidget(self.spinBox_Pass_General, 4, 1, 1, 1)
        self.scrollBar_StepSize_General = QtWidgets.QScrollBar(self.General)
        self.scrollBar_StepSize_General.setMinimumSize(QtCore.QSize(120, 0))
        self.scrollBar_StepSize_General.setMinimum(1)
        self.scrollBar_StepSize_General.setMaximum(8192)
        self.scrollBar_StepSize_General.setOrientation(QtCore.Qt.Horizontal)
        self.scrollBar_StepSize_General.setObjectName("scrollBar_StepSize_General")
        self.gridLayout_5.addWidget(self.scrollBar_StepSize_General, 2, 2, 1, 2)
        self.spiBox_StepSize_General = QtWidgets.QDoubleSpinBox(self.General)
        self.spiBox_StepSize_General.setDecimals(7)
        self.spiBox_StepSize_General.setMaximum(10.0)
        self.spiBox_StepSize_General.setObjectName("spiBox_StepSize_General")
        self.gridLayout_5.addWidget(self.spiBox_StepSize_General, 2, 1, 1, 1)
        self.label_6 = QtWidgets.QLabel(self.General)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_6.sizePolicy().hasHeightForWidth())
        self.label_6.setSizePolicy(sizePolicy)
        self.label_6.setObjectName("label_6")
        self.gridLayout_5.addWidget(self.label_6, 0, 0, 1, 1)
        self.label_13 = QtWidgets.QLabel(self.General)
        self.label_13.setObjectName("label_13")
        self.gridLayout_5.addWidget(self.label_13, 3, 0, 1, 2)
        self.comboBox_RampCh_General = QtWidgets.QComboBox(self.General)
        self.comboBox_RampCh_General.setObjectName("comboBox_RampCh_General")
        self.comboBox_RampCh_General.addItem("")
        self.comboBox_RampCh_General.addItem("")
        self.gridLayout_5.addWidget(self.comboBox_RampCh_General, 4, 3, 1, 1)
        self.label_9 = QtWidgets.QLabel(self.General)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_9.sizePolicy().hasHeightForWidth())
        self.label_9.setSizePolicy(sizePolicy)
        self.label_9.setObjectName("label_9")
        self.gridLayout_5.addWidget(self.label_9, 4, 0, 1, 1)
        self.label_DataNum_General = QtWidgets.QLabel(self.General)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_DataNum_General.sizePolicy().hasHeightForWidth())
        self.label_DataNum_General.setSizePolicy(sizePolicy)
        self.label_DataNum_General.setMinimumSize(QtCore.QSize(0, 25))
        self.label_DataNum_General.setMaximumSize(QtCore.QSize(16777215, 25))
        self.label_DataNum_General.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.label_DataNum_General.setText("")
        self.label_DataNum_General.setObjectName("label_DataNum_General")
        self.gridLayout_5.addWidget(self.label_DataNum_General, 3, 2, 1, 2)
        self.scrollBar_Max_General = QtWidgets.QScrollBar(self.General)
        self.scrollBar_Max_General.setMinimumSize(QtCore.QSize(120, 0))
        self.scrollBar_Max_General.setMinimum(1)
        self.scrollBar_Max_General.setMaximum(65535)
        self.scrollBar_Max_General.setOrientation(QtCore.Qt.Horizontal)
        self.scrollBar_Max_General.setObjectName("scrollBar_Max_General")
        self.gridLayout_5.addWidget(self.scrollBar_Max_General, 1, 2, 1, 2)
        self.label_7 = QtWidgets.QLabel(self.General)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_7.sizePolicy().hasHeightForWidth())
        self.label_7.setSizePolicy(sizePolicy)
        self.label_7.setObjectName("label_7")
        self.gridLayout_5.addWidget(self.label_7, 1, 0, 1, 1)
        self.spinBox_Max_General = QtWidgets.QDoubleSpinBox(self.General)
        self.spinBox_Max_General.setDecimals(7)
        self.spinBox_Max_General.setMinimum(-10.0)
        self.spinBox_Max_General.setMaximum(10.0)
        self.spinBox_Max_General.setObjectName("spinBox_Max_General")
        self.gridLayout_5.addWidget(self.spinBox_Max_General, 1, 1, 1, 1)
        self.label_10 = QtWidgets.QLabel(self.General)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_10.sizePolicy().hasHeightForWidth())
        self.label_10.setSizePolicy(sizePolicy)
        self.label_10.setObjectName("label_10")
        self.gridLayout_5.addWidget(self.label_10, 4, 2, 1, 1)
        self.spinBox_Min_General = QtWidgets.QDoubleSpinBox(self.General)
        self.spinBox_Min_General.setDecimals(7)
        self.spinBox_Min_General.setMinimum(-10.0)
        self.spinBox_Min_General.setMaximum(10.0)
        self.spinBox_Min_General.setObjectName("spinBox_Min_General")
        self.gridLayout_5.addWidget(self.spinBox_Min_General, 0, 1, 1, 1)
        self.scrollBar_Min_General = QtWidgets.QScrollBar(self.General)
        self.scrollBar_Min_General.setMinimumSize(QtCore.QSize(120, 0))
        self.scrollBar_Min_General.setMinimum(0)
        self.scrollBar_Min_General.setMaximum(65534)
        self.scrollBar_Min_General.setOrientation(QtCore.Qt.Horizontal)
        self.scrollBar_Min_General.setObjectName("scrollBar_Min_General")
        self.gridLayout_5.addWidget(self.scrollBar_Min_General, 0, 2, 1, 2)
        self.pushButton_Advance = QtWidgets.QPushButton(self.General)
        self.pushButton_Advance.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_Advance.setObjectName("pushButton_Advance")
        self.gridLayout_5.addWidget(self.pushButton_Advance, 7, 2, 1, 2)
        self.label_8 = QtWidgets.QLabel(self.General)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_8.sizePolicy().hasHeightForWidth())
        self.label_8.setSizePolicy(sizePolicy)
        self.label_8.setObjectName("label_8")
        self.gridLayout_5.addWidget(self.label_8, 2, 0, 1, 1)
        self.groupBox_Delta_General = QtWidgets.QGroupBox(self.General)
        self.groupBox_Delta_General.setMaximumSize(QtCore.QSize(16777215, 250))
        self.groupBox_Delta_General.setCheckable(True)
        self.groupBox_Delta_General.setObjectName("groupBox_Delta_General")
        self.gridLayout_4 = QtWidgets.QGridLayout(self.groupBox_Delta_General)
        self.gridLayout_4.setObjectName("gridLayout_4")
        self.label_11 = QtWidgets.QLabel(self.groupBox_Delta_General)
        self.label_11.setObjectName("label_11")
        self.gridLayout_4.addWidget(self.label_11, 0, 0, 1, 1)
        self.scrollBar_Z_Delta = QtWidgets.QScrollBar(self.groupBox_Delta_General)
        self.scrollBar_Z_Delta.setMinimum(-32768)
        self.scrollBar_Z_Delta.setMaximum(32767)
        self.scrollBar_Z_Delta.setOrientation(QtCore.Qt.Horizontal)
        self.scrollBar_Z_Delta.setObjectName("scrollBar_Z_Delta")
        self.gridLayout_4.addWidget(self.scrollBar_Z_Delta, 1, 0, 1, 2)
        self.spinBox_Bias_Delta = QtWidgets.QDoubleSpinBox(self.groupBox_Delta_General)
        self.spinBox_Bias_Delta.setDecimals(7)
        self.spinBox_Bias_Delta.setMinimum(-10.0)
        self.spinBox_Bias_Delta.setMaximum(10.0)
        self.spinBox_Bias_Delta.setObjectName("spinBox_Bias_Delta")
        self.gridLayout_4.addWidget(self.spinBox_Bias_Delta, 2, 1, 1, 1)
        self.spinBox_Delta_Z = QtWidgets.QSpinBox(self.groupBox_Delta_General)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_Delta_Z.sizePolicy().hasHeightForWidth())
        self.spinBox_Delta_Z.setSizePolicy(sizePolicy)
        self.spinBox_Delta_Z.setMinimumSize(QtCore.QSize(60, 20))
        self.spinBox_Delta_Z.setMinimum(-32768)
        self.spinBox_Delta_Z.setMaximum(32767)
        self.spinBox_Delta_Z.setObjectName("spinBox_Delta_Z")
        self.gridLayout_4.addWidget(self.spinBox_Delta_Z, 0, 1, 1, 1)
        self.label_12 = QtWidgets.QLabel(self.groupBox_Delta_General)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_12.sizePolicy().hasHeightForWidth())
        self.label_12.setSizePolicy(sizePolicy)
        self.label_12.setObjectName("label_12")
        self.gridLayout_4.addWidget(self.label_12, 2, 0, 1, 1)
        self.scrollBar_Bias_Delta = QtWidgets.QScrollBar(self.groupBox_Delta_General)
        self.scrollBar_Bias_Delta.setMinimum(0)
        self.scrollBar_Bias_Delta.setMaximum(65535)
        self.scrollBar_Bias_Delta.setProperty("value", 32768)
        self.scrollBar_Bias_Delta.setOrientation(QtCore.Qt.Horizontal)
        self.scrollBar_Bias_Delta.setObjectName("scrollBar_Bias_Delta")
        self.gridLayout_4.addWidget(self.scrollBar_Bias_Delta, 3, 0, 1, 2)
        self.gridLayout_5.addWidget(self.groupBox_Delta_General, 6, 0, 3, 2)
        self.groupBox_Mapping = QtWidgets.QGroupBox(self.General)
        self.groupBox_Mapping.setMaximumSize(QtCore.QSize(16777215, 110))
        self.groupBox_Mapping.setCheckable(True)
        self.groupBox_Mapping.setChecked(False)
        self.groupBox_Mapping.setObjectName("groupBox_Mapping")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.groupBox_Mapping)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.pushButton_EditPoints = QtWidgets.QPushButton(self.groupBox_Mapping)
        self.pushButton_EditPoints.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_EditPoints.setObjectName("pushButton_EditPoints")
        self.gridLayout_2.addWidget(self.pushButton_EditPoints, 0, 0, 1, 1)
        self.gridLayout_5.addWidget(self.groupBox_Mapping, 6, 2, 1, 2)
        self.gridLayout.addWidget(self.General, 1, 2, 1, 2)
        self.pushButton_Save = QtWidgets.QPushButton(Spectroscopy)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Save.sizePolicy().hasHeightForWidth())
        self.pushButton_Save.setSizePolicy(sizePolicy)
        self.pushButton_Save.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_Save.setObjectName("pushButton_Save")
        self.gridLayout.addWidget(self.pushButton_Save, 2, 2, 1, 1)
        self.Graph = QtWidgets.QGroupBox(Spectroscopy)
        self.Graph.setMaximumSize(QtCore.QSize(16777215, 100))
        self.Graph.setObjectName("Graph")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.Graph)
        self.gridLayout_3.setSpacing(9)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.label_point_Graph = QtWidgets.QLabel(self.Graph)
        self.label_point_Graph.setMinimumSize(QtCore.QSize(0, 24))
        self.label_point_Graph.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.label_point_Graph.setFrameShadow(QtWidgets.QFrame.Plain)
        self.label_point_Graph.setObjectName("label_point_Graph")
        self.gridLayout_3.addWidget(self.label_point_Graph, 3, 3, 1, 1)
        self.scrollBar_point_Graph = QtWidgets.QScrollBar(self.Graph)
        self.scrollBar_point_Graph.setMaximum(0)
        self.scrollBar_point_Graph.setOrientation(QtCore.Qt.Horizontal)
        self.scrollBar_point_Graph.setObjectName("scrollBar_point_Graph")
        self.gridLayout_3.addWidget(self.scrollBar_point_Graph, 3, 1, 1, 2)
        self.label_14 = QtWidgets.QLabel(self.Graph)
        self.label_14.setObjectName("label_14")
        self.gridLayout_3.addWidget(self.label_14, 3, 0, 1, 1)
        self.gridLayout.addWidget(self.Graph, 0, 2, 1, 2)
        self.widget = QtWidgets.QWidget(Spectroscopy)
        self.widget.setObjectName("widget")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.widget)
        self.verticalLayout.setObjectName("verticalLayout")
        self.widget_3 = QtWidgets.QWidget(self.widget)
        self.widget_3.setObjectName("widget_3")
        self.gridLayout_7 = QtWidgets.QGridLayout(self.widget_3)
        self.gridLayout_7.setContentsMargins(0, 0, 0, 0)
        self.gridLayout_7.setObjectName("gridLayout_7")
        self.pushButton_Scanner_Cur = QtWidgets.QPushButton(self.widget_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Scanner_Cur.sizePolicy().hasHeightForWidth())
        self.pushButton_Scanner_Cur.setSizePolicy(sizePolicy)
        self.pushButton_Scanner_Cur.setMaximumSize(QtCore.QSize(20, 20))
        self.pushButton_Scanner_Cur.setText("")
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/toolbar/data/scanner.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.pushButton_Scanner_Cur.setIcon(icon)
        self.pushButton_Scanner_Cur.setIconSize(QtCore.QSize(20, 20))
        self.pushButton_Scanner_Cur.setCheckable(True)
        self.pushButton_Scanner_Cur.setFlat(True)
        self.pushButton_Scanner_Cur.setObjectName("pushButton_Scanner_Cur")
        self.gridLayout_7.addWidget(self.pushButton_Scanner_Cur, 0, 3, 1, 1)
        self.label_16 = QtWidgets.QLabel(self.widget_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_16.sizePolicy().hasHeightForWidth())
        self.label_16.setSizePolicy(sizePolicy)
        self.label_16.setObjectName("label_16")
        self.gridLayout_7.addWidget(self.label_16, 0, 0, 1, 1)
        self.pushButton_xScale_Cur = QtWidgets.QPushButton(self.widget_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_xScale_Cur.sizePolicy().hasHeightForWidth())
        self.pushButton_xScale_Cur.setSizePolicy(sizePolicy)
        self.pushButton_xScale_Cur.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_xScale_Cur.setText("")
        icon1 = QtGui.QIcon()
        icon1.addPixmap(QtGui.QPixmap(":/toolbar/data/left_right.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.pushButton_xScale_Cur.setIcon(icon1)
        self.pushButton_xScale_Cur.setFlat(True)
        self.pushButton_xScale_Cur.setObjectName("pushButton_xScale_Cur")
        self.gridLayout_7.addWidget(self.pushButton_xScale_Cur, 0, 1, 1, 1)
        self.pushButton_yScale_Cur = QtWidgets.QPushButton(self.widget_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_yScale_Cur.sizePolicy().hasHeightForWidth())
        self.pushButton_yScale_Cur.setSizePolicy(sizePolicy)
        self.pushButton_yScale_Cur.setMinimumSize(QtCore.QSize(0, 25))
        self.pushButton_yScale_Cur.setText("")
        icon2 = QtGui.QIcon()
        icon2.addPixmap(QtGui.QPixmap(":/toolbar/data/up_down.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.pushButton_yScale_Cur.setIcon(icon2)
        self.pushButton_yScale_Cur.setFlat(True)
        self.pushButton_yScale_Cur.setObjectName("pushButton_yScale_Cur")
        self.gridLayout_7.addWidget(self.pushButton_yScale_Cur, 0, 2, 1, 1)
        self.verticalLayout.addWidget(self.widget_3)
        self.graphicsView_current_Spectro = GraphicsLayoutWidget(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.graphicsView_current_Spectro.sizePolicy().hasHeightForWidth())
        self.graphicsView_current_Spectro.setSizePolicy(sizePolicy)
        self.graphicsView_current_Spectro.setMinimumSize(QtCore.QSize(320, 320))
        self.graphicsView_current_Spectro.setObjectName("graphicsView_current_Spectro")
        self.verticalLayout.addWidget(self.graphicsView_current_Spectro)
        self.widget_2 = QtWidgets.QWidget(self.widget)
        self.widget_2.setObjectName("widget_2")
        self.gridLayout_6 = QtWidgets.QGridLayout(self.widget_2)
        self.gridLayout_6.setContentsMargins(0, 0, 0, 0)
        self.gridLayout_6.setObjectName("gridLayout_6")
        self.pushButton_xScale_Avg = QtWidgets.QPushButton(self.widget_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_xScale_Avg.sizePolicy().hasHeightForWidth())
        self.pushButton_xScale_Avg.setSizePolicy(sizePolicy)
        self.pushButton_xScale_Avg.setText("")
        self.pushButton_xScale_Avg.setIcon(icon1)
        self.pushButton_xScale_Avg.setFlat(True)
        self.pushButton_xScale_Avg.setObjectName("pushButton_xScale_Avg")
        self.gridLayout_6.addWidget(self.pushButton_xScale_Avg, 0, 1, 1, 1)
        self.label_15 = QtWidgets.QLabel(self.widget_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_15.sizePolicy().hasHeightForWidth())
        self.label_15.setSizePolicy(sizePolicy)
        self.label_15.setObjectName("label_15")
        self.gridLayout_6.addWidget(self.label_15, 0, 0, 1, 1)
        self.pushButton_Scanner_Avg = QtWidgets.QPushButton(self.widget_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_Scanner_Avg.sizePolicy().hasHeightForWidth())
        self.pushButton_Scanner_Avg.setSizePolicy(sizePolicy)
        self.pushButton_Scanner_Avg.setMaximumSize(QtCore.QSize(20, 20))
        self.pushButton_Scanner_Avg.setText("")
        self.pushButton_Scanner_Avg.setIcon(icon)
        self.pushButton_Scanner_Avg.setIconSize(QtCore.QSize(20, 20))
        self.pushButton_Scanner_Avg.setCheckable(True)
        self.pushButton_Scanner_Avg.setFlat(True)
        self.pushButton_Scanner_Avg.setObjectName("pushButton_Scanner_Avg")
        self.gridLayout_6.addWidget(self.pushButton_Scanner_Avg, 0, 3, 1, 1)
        self.pushButton_yScale_Avg = QtWidgets.QPushButton(self.widget_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_yScale_Avg.sizePolicy().hasHeightForWidth())
        self.pushButton_yScale_Avg.setSizePolicy(sizePolicy)
        self.pushButton_yScale_Avg.setText("")
        self.pushButton_yScale_Avg.setIcon(icon2)
        self.pushButton_yScale_Avg.setFlat(True)
        self.pushButton_yScale_Avg.setObjectName("pushButton_yScale_Avg")
        self.gridLayout_6.addWidget(self.pushButton_yScale_Avg, 0, 2, 1, 1)
        self.verticalLayout.addWidget(self.widget_2)
        self.graphicsView_avg_Spectro = GraphicsLayoutWidget(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.graphicsView_avg_Spectro.sizePolicy().hasHeightForWidth())
        self.graphicsView_avg_Spectro.setSizePolicy(sizePolicy)
        self.graphicsView_avg_Spectro.setMinimumSize(QtCore.QSize(320, 320))
        self.graphicsView_avg_Spectro.setObjectName("graphicsView_avg_Spectro")
        self.verticalLayout.addWidget(self.graphicsView_avg_Spectro)
        self.gridLayout.addWidget(self.widget, 0, 0, 4, 1)

        self.retranslateUi(Spectroscopy)
        QtCore.QMetaObject.connectSlotsByName(Spectroscopy)

    def retranslateUi(self, Spectroscopy):
        _translate = QtCore.QCoreApplication.translate
        Spectroscopy.setWindowTitle(_translate("Spectroscopy", "Spectroscopy"))
        self.pushButton_Scan.setText(_translate("Spectroscopy", "Scan"))
        self.pushButton_Info.setText(_translate("Spectroscopy", "Info"))
        self.General.setTitle(_translate("Spectroscopy", "General options"))
        self.pushButton_LockIn_General.setText(_translate("Spectroscopy", "Lock-in paramenters"))
        self.label_6.setText(_translate("Spectroscopy", "Min"))
        self.label_13.setText(_translate("Spectroscopy", "Number of data points"))
        self.comboBox_RampCh_General.setItemText(0, _translate("Spectroscopy", "Bias"))
        self.comboBox_RampCh_General.setItemText(1, _translate("Spectroscopy", "Z fine"))
        self.label_9.setText(_translate("Spectroscopy", "Pass #"))
        self.label_7.setText(_translate("Spectroscopy", "Max"))
        self.label_10.setText(_translate("Spectroscopy", "Ramp Ch"))
        self.pushButton_Advance.setText(_translate("Spectroscopy", " Advance option"))
        self.label_8.setText(_translate("Spectroscopy", "Step size"))
        self.groupBox_Delta_General.setTitle(_translate("Spectroscopy", "Delta Z and Bias"))
        self.label_11.setText(_translate("Spectroscopy", "Z (bit)"))
        self.label_12.setText(_translate("Spectroscopy", "Bias (V)"))
        self.groupBox_Mapping.setTitle(_translate("Spectroscopy", "Mapping"))
        self.pushButton_EditPoints.setText(_translate("Spectroscopy", "Edit points"))
        self.pushButton_Save.setText(_translate("Spectroscopy", "Save"))
        self.Graph.setTitle(_translate("Spectroscopy", "Graph"))
        self.label_point_Graph.setText(_translate("Spectroscopy", "0"))
        self.label_14.setText(_translate("Spectroscopy", "Point"))
        self.label_16.setText(_translate("Spectroscopy", "Current"))
        self.label_15.setText(_translate("Spectroscopy", "Averaged"))
from pyqtgraph import GraphicsLayoutWidget
import logo_rc

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ScanOptions.ui'
#
# Created by: PyQt5 UI code generator 5.12.3
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_ScanOptions(object):
    def setupUi(self, ScanOptions):
        ScanOptions.setObjectName("ScanOptions")
        ScanOptions.resize(715, 355)
        self.gridLayout_8 = QtWidgets.QGridLayout(ScanOptions)
        self.gridLayout_8.setObjectName("gridLayout_8")
        self.groupBox_Scan_ScanOptions = QtWidgets.QGroupBox(ScanOptions)
        self.groupBox_Scan_ScanOptions.setCheckable(True)
        self.groupBox_Scan_ScanOptions.setObjectName("groupBox_Scan_ScanOptions")
        self.gridLayout_6 = QtWidgets.QGridLayout(self.groupBox_Scan_ScanOptions)
        self.gridLayout_6.setObjectName("gridLayout_6")
        self.radioButton_Auto0_Scan = QtWidgets.QRadioButton(self.groupBox_Scan_ScanOptions)
        self.radioButton_Auto0_Scan.setObjectName("radioButton_Auto0_Scan")
        self.gridLayout_6.addWidget(self.radioButton_Auto0_Scan, 2, 0, 1, 3)
        self.radioButton_AutoPre_Scan = QtWidgets.QRadioButton(self.groupBox_Scan_ScanOptions)
        self.radioButton_AutoPre_Scan.setChecked(True)
        self.radioButton_AutoPre_Scan.setObjectName("radioButton_AutoPre_Scan")
        self.gridLayout_6.addWidget(self.radioButton_AutoPre_Scan, 3, 0, 1, 3)
        self.radioButton_Stop_Scan = QtWidgets.QRadioButton(self.groupBox_Scan_ScanOptions)
        self.radioButton_Stop_Scan.setObjectName("radioButton_Stop_Scan")
        self.gridLayout_6.addWidget(self.radioButton_Stop_Scan, 1, 0, 1, 1)
        self.spinBox_ZoutLmit_Scan = QtWidgets.QDoubleSpinBox(self.groupBox_Scan_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_ZoutLmit_Scan.sizePolicy().hasHeightForWidth())
        self.spinBox_ZoutLmit_Scan.setSizePolicy(sizePolicy)
        self.spinBox_ZoutLmit_Scan.setMinimumSize(QtCore.QSize(110, 20))
        self.spinBox_ZoutLmit_Scan.setDecimals(5)
        self.spinBox_ZoutLmit_Scan.setMinimum(5.12)
        self.spinBox_ZoutLmit_Scan.setMaximum(10.24)
        self.spinBox_ZoutLmit_Scan.setObjectName("spinBox_ZoutLmit_Scan")
        self.gridLayout_6.addWidget(self.spinBox_ZoutLmit_Scan, 0, 1, 1, 1)
        self.label_5 = QtWidgets.QLabel(self.groupBox_Scan_ScanOptions)
        self.label_5.setObjectName("label_5")
        self.gridLayout_6.addWidget(self.label_5, 0, 2, 1, 1)
        self.label_22 = QtWidgets.QLabel(self.groupBox_Scan_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_22.sizePolicy().hasHeightForWidth())
        self.label_22.setSizePolicy(sizePolicy)
        self.label_22.setObjectName("label_22")
        self.gridLayout_6.addWidget(self.label_22, 0, 0, 1, 1)
        self.gridLayout_8.addWidget(self.groupBox_Scan_ScanOptions, 0, 2, 3, 1)
        self.groupBox_Delay_ScanOptions = QtWidgets.QGroupBox(ScanOptions)
        self.groupBox_Delay_ScanOptions.setObjectName("groupBox_Delay_ScanOptions")
        self.gridLayout_5 = QtWidgets.QGridLayout(self.groupBox_Delay_ScanOptions)
        self.gridLayout_5.setHorizontalSpacing(9)
        self.gridLayout_5.setObjectName("gridLayout_5")
        self.label_11 = QtWidgets.QLabel(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Maximum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_11.sizePolicy().hasHeightForWidth())
        self.label_11.setSizePolicy(sizePolicy)
        self.label_11.setAlignment(QtCore.Qt.AlignCenter)
        self.label_11.setObjectName("label_11")
        self.gridLayout_5.addWidget(self.label_11, 1, 2, 1, 1)
        self.label_9 = QtWidgets.QLabel(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_9.sizePolicy().hasHeightForWidth())
        self.label_9.setSizePolicy(sizePolicy)
        self.label_9.setObjectName("label_9")
        self.gridLayout_5.addWidget(self.label_9, 6, 0, 1, 1)
        self.label_10 = QtWidgets.QLabel(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Maximum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_10.sizePolicy().hasHeightForWidth())
        self.label_10.setSizePolicy(sizePolicy)
        self.label_10.setAlignment(QtCore.Qt.AlignCenter)
        self.label_10.setObjectName("label_10")
        self.gridLayout_5.addWidget(self.label_10, 1, 1, 1, 1)
        self.label_8 = QtWidgets.QLabel(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_8.sizePolicy().hasHeightForWidth())
        self.label_8.setSizePolicy(sizePolicy)
        self.label_8.setObjectName("label_8")
        self.gridLayout_5.addWidget(self.label_8, 4, 0, 1, 1)
        self.radioButton_Variable_Delay = QtWidgets.QRadioButton(self.groupBox_Delay_ScanOptions)
        self.radioButton_Variable_Delay.setObjectName("radioButton_Variable_Delay")
        self.gridLayout_5.addWidget(self.radioButton_Variable_Delay, 0, 1, 1, 1)
        self.label_12 = QtWidgets.QLabel(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Maximum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_12.sizePolicy().hasHeightForWidth())
        self.label_12.setSizePolicy(sizePolicy)
        self.label_12.setAlignment(QtCore.Qt.AlignCenter)
        self.label_12.setObjectName("label_12")
        self.gridLayout_5.addWidget(self.label_12, 1, 3, 1, 1)
        self.label_7 = QtWidgets.QLabel(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_7.sizePolicy().hasHeightForWidth())
        self.label_7.setSizePolicy(sizePolicy)
        self.label_7.setObjectName("label_7")
        self.gridLayout_5.addWidget(self.label_7, 2, 0, 1, 1)
        self.radioButton_Fixed_Delay = QtWidgets.QRadioButton(self.groupBox_Delay_ScanOptions)
        self.radioButton_Fixed_Delay.setChecked(True)
        self.radioButton_Fixed_Delay.setObjectName("radioButton_Fixed_Delay")
        self.gridLayout_5.addWidget(self.radioButton_Fixed_Delay, 0, 0, 1, 1)
        self.spinBox_MoveControl_Delay = QtWidgets.QSpinBox(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_MoveControl_Delay.sizePolicy().hasHeightForWidth())
        self.spinBox_MoveControl_Delay.setSizePolicy(sizePolicy)
        self.spinBox_MoveControl_Delay.setMinimumSize(QtCore.QSize(110, 20))
        self.spinBox_MoveControl_Delay.setWrapping(False)
        self.spinBox_MoveControl_Delay.setFrame(True)
        self.spinBox_MoveControl_Delay.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.spinBox_MoveControl_Delay.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spinBox_MoveControl_Delay.setMaximum(65535)
        self.spinBox_MoveControl_Delay.setProperty("value", 1)
        self.spinBox_MoveControl_Delay.setObjectName("spinBox_MoveControl_Delay")
        self.gridLayout_5.addWidget(self.spinBox_MoveControl_Delay, 2, 1, 1, 1)
        self.spinBox_MoveOFF_Delay = QtWidgets.QSpinBox(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_MoveOFF_Delay.sizePolicy().hasHeightForWidth())
        self.spinBox_MoveOFF_Delay.setSizePolicy(sizePolicy)
        self.spinBox_MoveOFF_Delay.setMinimumSize(QtCore.QSize(110, 20))
        self.spinBox_MoveOFF_Delay.setWrapping(False)
        self.spinBox_MoveOFF_Delay.setFrame(True)
        self.spinBox_MoveOFF_Delay.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.spinBox_MoveOFF_Delay.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spinBox_MoveOFF_Delay.setMaximum(65535)
        self.spinBox_MoveOFF_Delay.setProperty("value", 1)
        self.spinBox_MoveOFF_Delay.setObjectName("spinBox_MoveOFF_Delay")
        self.gridLayout_5.addWidget(self.spinBox_MoveOFF_Delay, 2, 2, 1, 1)
        self.spinBox_MoveON_Delay = QtWidgets.QSpinBox(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_MoveON_Delay.sizePolicy().hasHeightForWidth())
        self.spinBox_MoveON_Delay.setSizePolicy(sizePolicy)
        self.spinBox_MoveON_Delay.setMinimumSize(QtCore.QSize(110, 20))
        self.spinBox_MoveON_Delay.setWrapping(False)
        self.spinBox_MoveON_Delay.setFrame(True)
        self.spinBox_MoveON_Delay.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.spinBox_MoveON_Delay.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spinBox_MoveON_Delay.setMaximum(65535)
        self.spinBox_MoveON_Delay.setProperty("value", 1)
        self.spinBox_MoveON_Delay.setObjectName("spinBox_MoveON_Delay")
        self.gridLayout_5.addWidget(self.spinBox_MoveON_Delay, 2, 3, 1, 1)
        self.spinBox_ReadControl_Delay = QtWidgets.QSpinBox(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_ReadControl_Delay.sizePolicy().hasHeightForWidth())
        self.spinBox_ReadControl_Delay.setSizePolicy(sizePolicy)
        self.spinBox_ReadControl_Delay.setMinimumSize(QtCore.QSize(110, 20))
        self.spinBox_ReadControl_Delay.setWrapping(False)
        self.spinBox_ReadControl_Delay.setFrame(True)
        self.spinBox_ReadControl_Delay.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.spinBox_ReadControl_Delay.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spinBox_ReadControl_Delay.setMaximum(65535)
        self.spinBox_ReadControl_Delay.setProperty("value", 100)
        self.spinBox_ReadControl_Delay.setObjectName("spinBox_ReadControl_Delay")
        self.gridLayout_5.addWidget(self.spinBox_ReadControl_Delay, 4, 1, 1, 1)
        self.spinBox_ReadOFF_Delay = QtWidgets.QSpinBox(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_ReadOFF_Delay.sizePolicy().hasHeightForWidth())
        self.spinBox_ReadOFF_Delay.setSizePolicy(sizePolicy)
        self.spinBox_ReadOFF_Delay.setMinimumSize(QtCore.QSize(110, 20))
        self.spinBox_ReadOFF_Delay.setWrapping(False)
        self.spinBox_ReadOFF_Delay.setFrame(True)
        self.spinBox_ReadOFF_Delay.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.spinBox_ReadOFF_Delay.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spinBox_ReadOFF_Delay.setMaximum(65535)
        self.spinBox_ReadOFF_Delay.setProperty("value", 100)
        self.spinBox_ReadOFF_Delay.setObjectName("spinBox_ReadOFF_Delay")
        self.gridLayout_5.addWidget(self.spinBox_ReadOFF_Delay, 4, 2, 1, 1)
        self.spinBox_ReadON_Delay = QtWidgets.QSpinBox(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_ReadON_Delay.sizePolicy().hasHeightForWidth())
        self.spinBox_ReadON_Delay.setSizePolicy(sizePolicy)
        self.spinBox_ReadON_Delay.setMinimumSize(QtCore.QSize(110, 20))
        self.spinBox_ReadON_Delay.setWrapping(False)
        self.spinBox_ReadON_Delay.setFrame(True)
        self.spinBox_ReadON_Delay.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.spinBox_ReadON_Delay.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spinBox_ReadON_Delay.setMaximum(65535)
        self.spinBox_ReadON_Delay.setProperty("value", 100)
        self.spinBox_ReadON_Delay.setObjectName("spinBox_ReadON_Delay")
        self.gridLayout_5.addWidget(self.spinBox_ReadON_Delay, 4, 3, 1, 1)
        self.spinBox_LineControl_Delay = QtWidgets.QSpinBox(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_LineControl_Delay.sizePolicy().hasHeightForWidth())
        self.spinBox_LineControl_Delay.setSizePolicy(sizePolicy)
        self.spinBox_LineControl_Delay.setMinimumSize(QtCore.QSize(110, 20))
        self.spinBox_LineControl_Delay.setWrapping(False)
        self.spinBox_LineControl_Delay.setFrame(True)
        self.spinBox_LineControl_Delay.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.spinBox_LineControl_Delay.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spinBox_LineControl_Delay.setMinimum(1)
        self.spinBox_LineControl_Delay.setMaximum(65535)
        self.spinBox_LineControl_Delay.setProperty("value", 10)
        self.spinBox_LineControl_Delay.setObjectName("spinBox_LineControl_Delay")
        self.gridLayout_5.addWidget(self.spinBox_LineControl_Delay, 6, 1, 1, 1)
        self.spinBox_LineOFF_Delay = QtWidgets.QSpinBox(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_LineOFF_Delay.sizePolicy().hasHeightForWidth())
        self.spinBox_LineOFF_Delay.setSizePolicy(sizePolicy)
        self.spinBox_LineOFF_Delay.setMinimumSize(QtCore.QSize(110, 20))
        self.spinBox_LineOFF_Delay.setWrapping(False)
        self.spinBox_LineOFF_Delay.setFrame(True)
        self.spinBox_LineOFF_Delay.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.spinBox_LineOFF_Delay.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spinBox_LineOFF_Delay.setMaximum(65535)
        self.spinBox_LineOFF_Delay.setProperty("value", 1)
        self.spinBox_LineOFF_Delay.setObjectName("spinBox_LineOFF_Delay")
        self.gridLayout_5.addWidget(self.spinBox_LineOFF_Delay, 6, 2, 1, 1)
        self.spinBox_LineON_Delay = QtWidgets.QSpinBox(self.groupBox_Delay_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_LineON_Delay.sizePolicy().hasHeightForWidth())
        self.spinBox_LineON_Delay.setSizePolicy(sizePolicy)
        self.spinBox_LineON_Delay.setMinimumSize(QtCore.QSize(110, 20))
        self.spinBox_LineON_Delay.setWrapping(False)
        self.spinBox_LineON_Delay.setFrame(True)
        self.spinBox_LineON_Delay.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.spinBox_LineON_Delay.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spinBox_LineON_Delay.setMaximum(65535)
        self.spinBox_LineON_Delay.setProperty("value", 100)
        self.spinBox_LineON_Delay.setObjectName("spinBox_LineON_Delay")
        self.gridLayout_5.addWidget(self.spinBox_LineON_Delay, 6, 3, 1, 1)
        self.gridLayout_8.addWidget(self.groupBox_Delay_ScanOptions, 3, 0, 2, 2)
        self.groupBox_Feedback_ScanOptions = QtWidgets.QGroupBox(ScanOptions)
        self.groupBox_Feedback_ScanOptions.setCheckable(True)
        self.groupBox_Feedback_ScanOptions.setObjectName("groupBox_Feedback_ScanOptions")
        self.gridLayout_7 = QtWidgets.QGridLayout(self.groupBox_Feedback_ScanOptions)
        self.gridLayout_7.setObjectName("gridLayout_7")
        self.checkBox_MatchCurr_Feedback = QtWidgets.QCheckBox(self.groupBox_Feedback_ScanOptions)
        self.checkBox_MatchCurr_Feedback.setObjectName("checkBox_MatchCurr_Feedback")
        self.gridLayout_7.addWidget(self.checkBox_MatchCurr_Feedback, 0, 0, 1, 2)
        self.label_15 = QtWidgets.QLabel(self.groupBox_Feedback_ScanOptions)
        self.label_15.setObjectName("label_15")
        self.gridLayout_7.addWidget(self.label_15, 1, 0, 1, 1)
        self.spinBox_Advance_Feedabck = QtWidgets.QSpinBox(self.groupBox_Feedback_ScanOptions)
        self.spinBox_Advance_Feedabck.setMinimum(-32768)
        self.spinBox_Advance_Feedabck.setMaximum(32767)
        self.spinBox_Advance_Feedabck.setObjectName("spinBox_Advance_Feedabck")
        self.gridLayout_7.addWidget(self.spinBox_Advance_Feedabck, 1, 1, 1, 1)
        self.label_16 = QtWidgets.QLabel(self.groupBox_Feedback_ScanOptions)
        self.label_16.setObjectName("label_16")
        self.gridLayout_7.addWidget(self.label_16, 1, 2, 1, 1)
        self.scrollBar_Advance_Feedabck = QtWidgets.QScrollBar(self.groupBox_Feedback_ScanOptions)
        self.scrollBar_Advance_Feedabck.setMinimum(-32768)
        self.scrollBar_Advance_Feedabck.setMaximum(32767)
        self.scrollBar_Advance_Feedabck.setOrientation(QtCore.Qt.Horizontal)
        self.scrollBar_Advance_Feedabck.setObjectName("scrollBar_Advance_Feedabck")
        self.gridLayout_7.addWidget(self.scrollBar_Advance_Feedabck, 2, 0, 1, 3)
        self.gridLayout_8.addWidget(self.groupBox_Feedback_ScanOptions, 4, 2, 1, 1)
        self.widget_4 = QtWidgets.QWidget(ScanOptions)
        self.widget_4.setObjectName("widget_4")
        self.gridLayout_9 = QtWidgets.QGridLayout(self.widget_4)
        self.gridLayout_9.setObjectName("gridLayout_9")
        self.spinBox_Avg = QtWidgets.QSpinBox(self.widget_4)
        self.spinBox_Avg.setMinimum(1)
        self.spinBox_Avg.setMaximum(65535)
        self.spinBox_Avg.setSingleStep(100)
        self.spinBox_Avg.setProperty("value", 500)
        self.spinBox_Avg.setObjectName("spinBox_Avg")
        self.gridLayout_9.addWidget(self.spinBox_Avg, 0, 2, 1, 1)
        self.label_6 = QtWidgets.QLabel(self.widget_4)
        self.label_6.setObjectName("label_6")
        self.gridLayout_9.addWidget(self.label_6, 0, 1, 1, 1)
        self.gridLayout_8.addWidget(self.widget_4, 3, 2, 1, 1)
        self.groupBox_Tip_ScanOptions = QtWidgets.QGroupBox(ScanOptions)
        self.groupBox_Tip_ScanOptions.setCheckable(True)
        self.groupBox_Tip_ScanOptions.setObjectName("groupBox_Tip_ScanOptions")
        self.gridLayout_4 = QtWidgets.QGridLayout(self.groupBox_Tip_ScanOptions)
        self.gridLayout_4.setObjectName("gridLayout_4")
        self.label_3 = QtWidgets.QLabel(self.groupBox_Tip_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_3.sizePolicy().hasHeightForWidth())
        self.label_3.setSizePolicy(sizePolicy)
        self.label_3.setObjectName("label_3")
        self.gridLayout_4.addWidget(self.label_3, 0, 2, 1, 1)
        self.label = QtWidgets.QLabel(self.groupBox_Tip_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label.sizePolicy().hasHeightForWidth())
        self.label.setSizePolicy(sizePolicy)
        self.label.setObjectName("label")
        self.gridLayout_4.addWidget(self.label, 0, 0, 1, 1)
        self.spinBox_Retract_Tip = QtWidgets.QSpinBox(self.groupBox_Tip_ScanOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_Retract_Tip.sizePolicy().hasHeightForWidth())
        self.spinBox_Retract_Tip.setSizePolicy(sizePolicy)
        self.spinBox_Retract_Tip.setMinimumSize(QtCore.QSize(80, 20))
        self.spinBox_Retract_Tip.setMinimum(0)
        self.spinBox_Retract_Tip.setMaximum(65535)
        self.spinBox_Retract_Tip.setObjectName("spinBox_Retract_Tip")
        self.gridLayout_4.addWidget(self.spinBox_Retract_Tip, 0, 1, 1, 1)
        self.scrollBar_Retract_tip = QtWidgets.QScrollBar(self.groupBox_Tip_ScanOptions)
        self.scrollBar_Retract_tip.setMaximum(65535)
        self.scrollBar_Retract_tip.setOrientation(QtCore.Qt.Horizontal)
        self.scrollBar_Retract_tip.setObjectName("scrollBar_Retract_tip")
        self.gridLayout_4.addWidget(self.scrollBar_Retract_tip, 1, 0, 1, 3)
        self.gridLayout_8.addWidget(self.groupBox_Tip_ScanOptions, 0, 1, 3, 1)
        self.OrderandDirection = QtWidgets.QGroupBox(ScanOptions)
        self.OrderandDirection.setObjectName("OrderandDirection")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.OrderandDirection)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.widget = QtWidgets.QWidget(self.OrderandDirection)
        self.widget.setObjectName("widget")
        self.gridLayout = QtWidgets.QGridLayout(self.widget)
        self.gridLayout.setObjectName("gridLayout")
        self.radioButton_XFirst_OrderandDirection = QtWidgets.QRadioButton(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.radioButton_XFirst_OrderandDirection.sizePolicy().hasHeightForWidth())
        self.radioButton_XFirst_OrderandDirection.setSizePolicy(sizePolicy)
        self.radioButton_XFirst_OrderandDirection.setObjectName("radioButton_XFirst_OrderandDirection")
        self.gridLayout.addWidget(self.radioButton_XFirst_OrderandDirection, 0, 0, 1, 1)
        self.radioButton_YFirst_OrderandDirection = QtWidgets.QRadioButton(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.radioButton_YFirst_OrderandDirection.sizePolicy().hasHeightForWidth())
        self.radioButton_YFirst_OrderandDirection.setSizePolicy(sizePolicy)
        self.radioButton_YFirst_OrderandDirection.setChecked(True)
        self.radioButton_YFirst_OrderandDirection.setObjectName("radioButton_YFirst_OrderandDirection")
        self.gridLayout.addWidget(self.radioButton_YFirst_OrderandDirection, 1, 0, 1, 1)
        self.gridLayout_3.addWidget(self.widget, 0, 0, 1, 1)
        self.widget_2 = QtWidgets.QWidget(self.OrderandDirection)
        self.widget_2.setObjectName("widget_2")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.widget_2)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.radioButton_ReadForward_OrderandDirection = QtWidgets.QRadioButton(self.widget_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.radioButton_ReadForward_OrderandDirection.sizePolicy().hasHeightForWidth())
        self.radioButton_ReadForward_OrderandDirection.setSizePolicy(sizePolicy)
        self.radioButton_ReadForward_OrderandDirection.setChecked(True)
        self.radioButton_ReadForward_OrderandDirection.setObjectName("radioButton_ReadForward_OrderandDirection")
        self.gridLayout_2.addWidget(self.radioButton_ReadForward_OrderandDirection, 0, 0, 1, 1)
        self.radioButton_ReadBackward_OrderandDirection = QtWidgets.QRadioButton(self.widget_2)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.radioButton_ReadBackward_OrderandDirection.sizePolicy().hasHeightForWidth())
        self.radioButton_ReadBackward_OrderandDirection.setSizePolicy(sizePolicy)
        self.radioButton_ReadBackward_OrderandDirection.setObjectName("radioButton_ReadBackward_OrderandDirection")
        self.gridLayout_2.addWidget(self.radioButton_ReadBackward_OrderandDirection, 1, 0, 1, 1)
        self.gridLayout_3.addWidget(self.widget_2, 0, 1, 1, 1)
        self.gridLayout_8.addWidget(self.OrderandDirection, 0, 0, 3, 1)

        self.retranslateUi(ScanOptions)
        QtCore.QMetaObject.connectSlotsByName(ScanOptions)

    def retranslateUi(self, ScanOptions):
        _translate = QtCore.QCoreApplication.translate
        ScanOptions.setWindowTitle(_translate("ScanOptions", "Scan Options"))
        self.groupBox_Scan_ScanOptions.setTitle(_translate("ScanOptions", "Scan protection"))
        self.radioButton_Auto0_Scan.setText(_translate("ScanOptions", "Auto 0 and continue"))
        self.radioButton_AutoPre_Scan.setText(_translate("ScanOptions", "Auto to previous midpoint and continue"))
        self.radioButton_Stop_Scan.setText(_translate("ScanOptions", "Stop Scan"))
        self.label_5.setText(_translate("ScanOptions", "V"))
        self.label_22.setText(_translate("ScanOptions", "Zout limit"))
        self.groupBox_Delay_ScanOptions.setTitle(_translate("ScanOptions", "Delay"))
        self.label_11.setText(_translate("ScanOptions", "Feedback off default"))
        self.label_9.setText(_translate("ScanOptions", "Line Delay (ms)"))
        self.label_10.setText(_translate("ScanOptions", "Control"))
        self.label_8.setText(_translate("ScanOptions", "Read Delay (us)"))
        self.radioButton_Variable_Delay.setText(_translate("ScanOptions", "Variable"))
        self.label_12.setText(_translate("ScanOptions", "Feedback on default"))
        self.label_7.setText(_translate("ScanOptions", "Move Delay (us)"))
        self.radioButton_Fixed_Delay.setText(_translate("ScanOptions", "Fixed"))
        self.groupBox_Feedback_ScanOptions.setTitle(_translate("ScanOptions", "Feedback off before and on after"))
        self.checkBox_MatchCurr_Feedback.setText(_translate("ScanOptions", "Match current"))
        self.label_15.setText(_translate("ScanOptions", "Advance Z fine"))
        self.label_16.setText(_translate("ScanOptions", "bits"))
        self.label_6.setText(_translate("ScanOptions", "Average Number"))
        self.groupBox_Tip_ScanOptions.setTitle(_translate("ScanOptions", "Tip protection"))
        self.label_3.setText(_translate("ScanOptions", "bits"))
        self.label.setText(_translate("ScanOptions", "Retract Z offset fine"))
        self.OrderandDirection.setTitle(_translate("ScanOptions", "Scan order and Read direction"))
        self.radioButton_XFirst_OrderandDirection.setText(_translate("ScanOptions", "X first"))
        self.radioButton_YFirst_OrderandDirection.setText(_translate("ScanOptions", "Y first"))
        self.radioButton_ReadForward_OrderandDirection.setText(_translate("ScanOptions", "Read Forward"))
        self.radioButton_ReadBackward_OrderandDirection.setText(_translate("ScanOptions", "Read Backward"))


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    ScanOptions = QtWidgets.QWidget()
    ui = Ui_ScanOptions()
    ui.setupUi(ScanOptions)
    ScanOptions.show()
    sys.exit(app.exec_())

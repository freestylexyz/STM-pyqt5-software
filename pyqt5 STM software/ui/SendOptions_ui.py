# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'SendOptions.ui'
#
# Created by: PyQt5 UI code generator 5.12.3
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_SendOptions(object):
    def setupUi(self, SendOptions):
        SendOptions.setObjectName("SendOptions")
        SendOptions.resize(218, 178)
        self.gridLayout_2 = QtWidgets.QGridLayout(SendOptions)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.widget = QtWidgets.QWidget(SendOptions)
        self.widget.setObjectName("widget")
        self.gridLayout = QtWidgets.QGridLayout(self.widget)
        self.gridLayout.setSpacing(9)
        self.gridLayout.setObjectName("gridLayout")
        self.label_2 = QtWidgets.QLabel(self.widget)
        self.label_2.setObjectName("label_2")
        self.gridLayout.addWidget(self.label_2, 0, 0, 1, 1)
        self.spinBox_MoveDelay_SendOptions = QtWidgets.QSpinBox(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_MoveDelay_SendOptions.sizePolicy().hasHeightForWidth())
        self.spinBox_MoveDelay_SendOptions.setSizePolicy(sizePolicy)
        self.spinBox_MoveDelay_SendOptions.setMinimumSize(QtCore.QSize(100, 26))
        self.spinBox_MoveDelay_SendOptions.setMaximum(65535)
        self.spinBox_MoveDelay_SendOptions.setObjectName("spinBox_MoveDelay_SendOptions")
        self.gridLayout.addWidget(self.spinBox_MoveDelay_SendOptions, 0, 1, 1, 1)
        self.label = QtWidgets.QLabel(self.widget)
        self.label.setObjectName("label")
        self.gridLayout.addWidget(self.label, 1, 0, 1, 1)
        self.spinBox_StepSize_SendOptions = QtWidgets.QSpinBox(self.widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_StepSize_SendOptions.sizePolicy().hasHeightForWidth())
        self.spinBox_StepSize_SendOptions.setSizePolicy(sizePolicy)
        self.spinBox_StepSize_SendOptions.setMinimumSize(QtCore.QSize(100, 26))
        self.spinBox_StepSize_SendOptions.setMinimum(1)
        self.spinBox_StepSize_SendOptions.setMaximum(65535)
        self.spinBox_StepSize_SendOptions.setObjectName("spinBox_StepSize_SendOptions")
        self.gridLayout.addWidget(self.spinBox_StepSize_SendOptions, 1, 1, 1, 1)
        self.gridLayout_2.addWidget(self.widget, 1, 0, 1, 1)
        self.groupBox_Crash_SendOptions = QtWidgets.QGroupBox(SendOptions)
        self.groupBox_Crash_SendOptions.setCheckable(True)
        self.groupBox_Crash_SendOptions.setObjectName("groupBox_Crash_SendOptions")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.groupBox_Crash_SendOptions)
        self.gridLayout_3.setSpacing(9)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.label_5 = QtWidgets.QLabel(self.groupBox_Crash_SendOptions)
        self.label_5.setObjectName("label_5")
        self.gridLayout_3.addWidget(self.label_5, 0, 0, 1, 1)
        self.spinBox_Limit_Crash = QtWidgets.QDoubleSpinBox(self.groupBox_Crash_SendOptions)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.spinBox_Limit_Crash.sizePolicy().hasHeightForWidth())
        self.spinBox_Limit_Crash.setSizePolicy(sizePolicy)
        self.spinBox_Limit_Crash.setMinimumSize(QtCore.QSize(100, 26))
        self.spinBox_Limit_Crash.setDecimals(5)
        self.spinBox_Limit_Crash.setMaximum(10.24)
        self.spinBox_Limit_Crash.setProperty("value", 5.12)
        self.spinBox_Limit_Crash.setObjectName("spinBox_Limit_Crash")
        self.gridLayout_3.addWidget(self.spinBox_Limit_Crash, 0, 1, 1, 1)
        self.gridLayout_2.addWidget(self.groupBox_Crash_SendOptions, 2, 0, 1, 1)

        self.retranslateUi(SendOptions)
        QtCore.QMetaObject.connectSlotsByName(SendOptions)

    def retranslateUi(self, SendOptions):
        _translate = QtCore.QCoreApplication.translate
        SendOptions.setWindowTitle(_translate("SendOptions", "Send Options"))
        self.label_2.setText(_translate("SendOptions", "Move Delay"))
        self.label.setText(_translate("SendOptions", "Step Size"))
        self.groupBox_Crash_SendOptions.setTitle(_translate("SendOptions", "Crash protection"))
        self.label_5.setText(_translate("SendOptions", "Limit"))


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    SendOptions = QtWidgets.QWidget()
    ui = Ui_SendOptions()
    ui.setupUi(SendOptions)
    SendOptions.show()
    sys.exit(app.exec_())

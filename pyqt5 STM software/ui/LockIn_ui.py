# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'LockIn.ui'
#
# Created by: PyQt5 UI code generator 5.12.3
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_LockIn(object):
    def setupUi(self, LockIn):
        LockIn.setObjectName("LockIn")
        LockIn.resize(769, 143)
        self.gridLayout = QtWidgets.QGridLayout(LockIn)
        self.gridLayout.setObjectName("gridLayout")
        self.spinBox_Phase1 = QtWidgets.QDoubleSpinBox(LockIn)
        self.spinBox_Phase1.setMaximum(360.0)
        self.spinBox_Phase1.setObjectName("spinBox_Phase1")
        self.gridLayout.addWidget(self.spinBox_Phase1, 1, 3, 1, 1)
        self.spinBox_OSC_Freq = QtWidgets.QDoubleSpinBox(LockIn)
        self.spinBox_OSC_Freq.setMaximum(1000000000.0)
        self.spinBox_OSC_Freq.setObjectName("spinBox_OSC_Freq")
        self.gridLayout.addWidget(self.spinBox_OSC_Freq, 0, 3, 1, 1)
        self.label_103 = QtWidgets.QLabel(LockIn)
        self.label_103.setObjectName("label_103")
        self.gridLayout.addWidget(self.label_103, 3, 4, 1, 1)
        self.spinBox_Sen1 = QtWidgets.QDoubleSpinBox(LockIn)
        self.spinBox_Sen1.setMaximum(10000.0)
        self.spinBox_Sen1.setObjectName("spinBox_Sen1")
        self.gridLayout.addWidget(self.spinBox_Sen1, 2, 1, 1, 1)
        self.label_90 = QtWidgets.QLabel(LockIn)
        self.label_90.setObjectName("label_90")
        self.gridLayout.addWidget(self.label_90, 0, 4, 1, 1)
        self.spinBox_Sen2 = QtWidgets.QDoubleSpinBox(LockIn)
        self.spinBox_Sen2.setMaximum(10000.0)
        self.spinBox_Sen2.setObjectName("spinBox_Sen2")
        self.gridLayout.addWidget(self.spinBox_Sen2, 3, 1, 1, 1)
        self.spinBox_OSC_RMS = QtWidgets.QDoubleSpinBox(LockIn)
        self.spinBox_OSC_RMS.setMaximum(10000.0)
        self.spinBox_OSC_RMS.setObjectName("spinBox_OSC_RMS")
        self.gridLayout.addWidget(self.spinBox_OSC_RMS, 0, 1, 1, 1)
        self.spinBox_Offset2 = QtWidgets.QDoubleSpinBox(LockIn)
        self.spinBox_Offset2.setMinimum(-300.0)
        self.spinBox_Offset2.setMaximum(300.0)
        self.spinBox_Offset2.setObjectName("spinBox_Offset2")
        self.gridLayout.addWidget(self.spinBox_Offset2, 3, 3, 1, 1)
        self.spinBox_LockIn_Freq = QtWidgets.QDoubleSpinBox(LockIn)
        self.spinBox_LockIn_Freq.setMaximum(10000.0)
        self.spinBox_LockIn_Freq.setObjectName("spinBox_LockIn_Freq")
        self.gridLayout.addWidget(self.spinBox_LockIn_Freq, 1, 1, 1, 1)
        self.spinBox_Offset1 = QtWidgets.QDoubleSpinBox(LockIn)
        self.spinBox_Offset1.setMinimum(-300.0)
        self.spinBox_Offset1.setMaximum(300.0)
        self.spinBox_Offset1.setObjectName("spinBox_Offset1")
        self.gridLayout.addWidget(self.spinBox_Offset1, 2, 3, 1, 1)
        self.label_108 = QtWidgets.QLabel(LockIn)
        self.label_108.setObjectName("label_108")
        self.gridLayout.addWidget(self.label_108, 1, 0, 1, 1)
        self.label_100 = QtWidgets.QLabel(LockIn)
        self.label_100.setObjectName("label_100")
        self.gridLayout.addWidget(self.label_100, 3, 2, 1, 1)
        self.spinBox_Phase2 = QtWidgets.QDoubleSpinBox(LockIn)
        self.spinBox_Phase2.setMaximum(360.0)
        self.spinBox_Phase2.setObjectName("spinBox_Phase2")
        self.gridLayout.addWidget(self.spinBox_Phase2, 1, 5, 1, 1)
        self.label_101 = QtWidgets.QLabel(LockIn)
        self.label_101.setObjectName("label_101")
        self.gridLayout.addWidget(self.label_101, 0, 0, 1, 1)
        self.label_99 = QtWidgets.QLabel(LockIn)
        self.label_99.setObjectName("label_99")
        self.gridLayout.addWidget(self.label_99, 3, 0, 1, 1)
        self.label_95 = QtWidgets.QLabel(LockIn)
        self.label_95.setObjectName("label_95")
        self.gridLayout.addWidget(self.label_95, 2, 4, 1, 1)
        self.label_105 = QtWidgets.QLabel(LockIn)
        self.label_105.setObjectName("label_105")
        self.gridLayout.addWidget(self.label_105, 1, 4, 1, 1)
        self.label_98 = QtWidgets.QLabel(LockIn)
        self.label_98.setObjectName("label_98")
        self.gridLayout.addWidget(self.label_98, 2, 2, 1, 1)
        self.label_97 = QtWidgets.QLabel(LockIn)
        self.label_97.setObjectName("label_97")
        self.gridLayout.addWidget(self.label_97, 0, 2, 1, 1)
        self.label_107 = QtWidgets.QLabel(LockIn)
        self.label_107.setObjectName("label_107")
        self.gridLayout.addWidget(self.label_107, 1, 2, 1, 1)
        self.spinBox_TC2 = QtWidgets.QSpinBox(LockIn)
        self.spinBox_TC2.setMaximum(10000)
        self.spinBox_TC2.setObjectName("spinBox_TC2")
        self.gridLayout.addWidget(self.spinBox_TC2, 3, 5, 1, 1)
        self.spinBox_TC1 = QtWidgets.QSpinBox(LockIn)
        self.spinBox_TC1.setMaximum(10000)
        self.spinBox_TC1.setObjectName("spinBox_TC1")
        self.gridLayout.addWidget(self.spinBox_TC1, 2, 5, 1, 1)
        self.label_104 = QtWidgets.QLabel(LockIn)
        self.label_104.setObjectName("label_104")
        self.gridLayout.addWidget(self.label_104, 2, 0, 1, 1)
        self.lineEdit_Type = QtWidgets.QLineEdit(LockIn)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.lineEdit_Type.sizePolicy().hasHeightForWidth())
        self.lineEdit_Type.setSizePolicy(sizePolicy)
        self.lineEdit_Type.setObjectName("lineEdit_Type")
        self.gridLayout.addWidget(self.lineEdit_Type, 0, 5, 1, 1)

        self.retranslateUi(LockIn)
        QtCore.QMetaObject.connectSlotsByName(LockIn)

    def retranslateUi(self, LockIn):
        _translate = QtCore.QCoreApplication.translate
        LockIn.setWindowTitle(_translate("LockIn", "Lock in parameters"))
        self.label_103.setText(_translate("LockIn", "Time constant 2 (ms)"))
        self.label_90.setText(_translate("LockIn", "Modulation type"))
        self.label_108.setText(_translate("LockIn", "Lock-in frequency (Hz)"))
        self.label_100.setText(_translate("LockIn", "Offset 2 (%)"))
        self.label_101.setText(_translate("LockIn", "OSC amplitude (mVrms)"))
        self.label_99.setText(_translate("LockIn", "Sensivity 2 (mV)"))
        self.label_95.setText(_translate("LockIn", "Time constant 1 (ms)"))
        self.label_105.setText(_translate("LockIn", "Phase 2"))
        self.label_98.setText(_translate("LockIn", "Offset 1 (%)"))
        self.label_97.setText(_translate("LockIn", "OSC frequency (kHz)"))
        self.label_107.setText(_translate("LockIn", "Phase 1"))
        self.label_104.setText(_translate("LockIn", "Sensivity 1 (mV)"))
        self.lineEdit_Type.setText(_translate("LockIn", "Sine"))


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    LockIn = QtWidgets.QWidget()
    ui = Ui_LockIn()
    ui.setupUi(LockIn)
    LockIn.show()
    sys.exit(app.exec_())

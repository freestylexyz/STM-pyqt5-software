# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'I_O.ui'
#
# Created by: PyQt5 UI code generator 5.12.3
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets
from DSP import dsp

class Ui_MainWindow(object):
    def setupUi(self, MainWindow, DSP):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(710, 520)
        font = QtGui.QFont()
        font.setPointSize(10)
        MainWindow.setFont(font)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        
        # 20bit DAC group
        self.bit20 = QtWidgets.QGroupBox(self.centralwidget)
        self.bit20.setGeometry(QtCore.QRect(320, 140, 381, 90))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.bit20.setFont(font)
        self.bit20.setObjectName("bit20")
        self.bit20_val = QtWidgets.QDoubleSpinBox(self.bit20)
        self.bit20_val.setGeometry(QtCore.QRect(10, 25, 140, 28))
        self.bit20_val.setDecimals(8)
        self.bit20_val.setMinimum(-5.0)
        self.bit20_val.setMaximum(5.0)
        self.bit20_val.setSingleStep(0.1)
        self.bit20_val.setObjectName("bit20_val")
        self.bit20_bit = QtWidgets.QScrollBar(self.bit20)
        self.bit20_bit.setGeometry(QtCore.QRect(10, 60, 361, 20))
        self.bit20_bit.setMaximum(1048575)
        self.bit20_bit.setPageStep(10000)
        self.bit20_bit.setOrientation(QtCore.Qt.Horizontal)
        self.bit20_bit.setObjectName("bit20_bit")
        self.bit20_out = QtWidgets.QPushButton(self.bit20)
        self.bit20_out.setGeometry(QtCore.QRect(250, 25, 120, 28))
        self.bit20_out.setObjectName("bit20_out")
        self.bit20_0 = QtWidgets.QPushButton(self.bit20)
        self.bit20_0.setGeometry(QtCore.QRect(170, 25, 60, 28))
        self.bit20_0.setObjectName("bit20_0")
        
        # DAC group
        self.dac_G = QtWidgets.QGroupBox(self.centralwidget)
        self.dac_G.setGeometry(QtCore.QRect(10, 10, 300, 220))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.dac_G.setFont(font)
        self.dac_G.setObjectName("dac_G")
        self.dac_ch = QtWidgets.QComboBox(self.dac_G)
        self.dac_ch.setGeometry(QtCore.QRect(10, 25, 280, 25))
        self.dac_ch.setObjectName("dac_ch")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_ch.addItem("")
        self.dac_val = QtWidgets.QDoubleSpinBox(self.dac_G)
        self.dac_val.setGeometry(QtCore.QRect(10, 60, 110, 25))
        self.dac_val.setDecimals(6)
        self.dac_val.setMinimum(-10.0)
        self.dac_val.setMaximum(10.0)
        self.dac_val.setSingleStep(0.1)
        self.dac_val.setObjectName("dac_val")
        self.dac_bit = QtWidgets.QScrollBar(self.dac_G)
        self.dac_bit.setGeometry(QtCore.QRect(10, 95, 280, 20))
        self.dac_bit.setMaximum(65535)
        self.dac_bit.setPageStep(1000)
        self.dac_bit.setOrientation(QtCore.Qt.Horizontal)
        self.dac_bit.setObjectName("dac_bit")
        self.dac_out = QtWidgets.QPushButton(self.dac_G)
        self.dac_out.setGeometry(QtCore.QRect(200, 60, 90, 28))
        self.dac_out.setObjectName("dac_out")
        self.dac_Iset_val = QtWidgets.QDoubleSpinBox(self.dac_G)
        self.dac_Iset_val.setEnabled(False)
        self.dac_Iset_val.setGeometry(QtCore.QRect(60, 185, 110, 25))
        self.dac_Iset_val.setDecimals(6)
        self.dac_Iset_val.setMinimum(-10.0)
        self.dac_Iset_val.setMaximum(10.0)
        self.dac_Iset_val.setSingleStep(0.01)
        self.dac_Iset_val.setObjectName("dac_Iset_val")
        self.dac_Iset_l = QtWidgets.QLabel(self.dac_G)
        self.dac_Iset_l.setGeometry(QtCore.QRect(10, 185, 40, 25))
        self.dac_Iset_l.setObjectName("dac_Iset_l")
        self.dac_b20 = QtWidgets.QRadioButton(self.dac_G)
        self.dac_b20.setGeometry(QtCore.QRect(10, 120, 80, 30))
        self.dac_b20.setObjectName("dac_b20")
        self.dac_b10 = QtWidgets.QRadioButton(self.dac_G)
        self.dac_b10.setGeometry(QtCore.QRect(105, 120, 80, 30))
        self.dac_b10.setObjectName("dac_b10")
        self.dac_b5 = QtWidgets.QRadioButton(self.dac_G)
        self.dac_b5.setGeometry(QtCore.QRect(200, 120, 80, 30))
        self.dac_b5.setObjectName("dac_b5")
        self.dac_u5 = QtWidgets.QRadioButton(self.dac_G)
        self.dac_u5.setGeometry(QtCore.QRect(10, 150, 95, 30))
        self.dac_u5.setObjectName("dac_u5")
        self.dac_u10 = QtWidgets.QRadioButton(self.dac_G)
        self.dac_u10.setGeometry(QtCore.QRect(105, 150, 95, 30))
        self.dac_u10.setObjectName("dac_u10")
        self.dac_Iset_unit = QtWidgets.QLabel(self.dac_G)
        self.dac_Iset_unit.setGeometry(QtCore.QRect(175, 185, 20, 25))
        self.dac_Iset_unit.setObjectName("dac_Iset_unit")
        self.dac_0 = QtWidgets.QPushButton(self.dac_G)
        self.dac_0.setGeometry(QtCore.QRect(130, 60, 60, 28))
        self.dac_0.setObjectName("dac_0")
        
        # ADC group
        self.adc_G = QtWidgets.QGroupBox(self.centralwidget)
        self.adc_G.setGeometry(QtCore.QRect(320, 10, 380, 130))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.adc_G.setFont(font)
        self.adc_G.setObjectName("adc_G")
        self.adc_in = QtWidgets.QPushButton(self.adc_G)
        self.adc_in.setGeometry(QtCore.QRect(299, 25, 71, 28))
        self.adc_in.setObjectName("adc_in")
        self.adc_ch = QtWidgets.QComboBox(self.adc_G)
        self.adc_ch.setGeometry(QtCore.QRect(10, 25, 150, 25))
        self.adc_ch.setObjectName("adc_ch")
        self.adc_ch.addItem("")
        self.adc_ch.addItem("")
        self.adc_ch.addItem("")
        self.adc_ch.addItem("")
        self.adc_ch.addItem("")
        self.adc_ch.addItem("")
        self.adc_ch.addItem("")
        self.adc_ch.addItem("")
        self.adc_val = QtWidgets.QDoubleSpinBox(self.adc_G)
        self.adc_val.setEnabled(False)
        self.adc_val.setGeometry(QtCore.QRect(170, 25, 110, 25))
        self.adc_val.setDecimals(6)
        self.adc_val.setMinimum(-10.24)
        self.adc_val.setMaximum(10.24)
        self.adc_val.setSingleStep(0.01)
        self.adc_val.setObjectName("adc_val")
        self.adc_b20 = QtWidgets.QRadioButton(self.adc_G)
        self.adc_b20.setGeometry(QtCore.QRect(10, 60, 100, 30))
        self.adc_b20.setObjectName("adc_b20")
        self.adc_b10 = QtWidgets.QRadioButton(self.adc_G)
        self.adc_b10.setGeometry(QtCore.QRect(140, 60, 100, 30))
        self.adc_b10.setObjectName("adc_b10")
        self.adc_b5 = QtWidgets.QRadioButton(self.adc_G)
        self.adc_b5.setGeometry(QtCore.QRect(270, 60, 100, 30))
        self.adc_b5.setObjectName("adc_b5")
        self.adc_u10 = QtWidgets.QRadioButton(self.adc_G)
        self.adc_u10.setGeometry(QtCore.QRect(10, 90, 120, 30))
        self.adc_u10.setObjectName("adc_u10")
        self.adc_u5 = QtWidgets.QRadioButton(self.adc_G)
        self.adc_u5.setGeometry(QtCore.QRect(140, 90, 120, 30))
        self.adc_u5.setObjectName("adc_u5")
        
        # x gain group
        self.xgain_G = QtWidgets.QGroupBox(self.centralwidget)
        self.xgain_G.setGeometry(QtCore.QRect(10, 230, 210, 55))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.xgain_G.setFont(font)
        self.xgain_G.setObjectName("xgain_G")
        self.xgain1 = QtWidgets.QRadioButton(self.xgain_G)
        self.xgain1.setGeometry(QtCore.QRect(80, 25, 50, 20))
        self.xgain1.setObjectName("xgain1")
        self.xgain0_1 = QtWidgets.QRadioButton(self.xgain_G)
        self.xgain0_1.setGeometry(QtCore.QRect(10, 25, 50, 20))
        self.xgain0_1.setObjectName("xgain0_1")
        self.xgain10 = QtWidgets.QRadioButton(self.xgain_G)
        self.xgain10.setGeometry(QtCore.QRect(150, 25, 50, 20))
        self.xgain10.setObjectName("xgain10")
        
        # y gain group
        self.ygain_G = QtWidgets.QGroupBox(self.centralwidget)
        self.ygain_G.setGeometry(QtCore.QRect(10, 290, 210, 55))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.ygain_G.setFont(font)
        self.ygain_G.setObjectName("ygain_G")
        self.ygain1 = QtWidgets.QRadioButton(self.ygain_G)
        self.ygain1.setGeometry(QtCore.QRect(80, 25, 50, 20))
        self.ygain1.setObjectName("ygain1")
        self.ygain0_1 = QtWidgets.QRadioButton(self.ygain_G)
        self.ygain0_1.setGeometry(QtCore.QRect(10, 25, 50, 20))
        self.ygain0_1.setObjectName("ygain0_1")
        self.ygain10 = QtWidgets.QRadioButton(self.ygain_G)
        self.ygain10.setGeometry(QtCore.QRect(150, 25, 50, 20))
        self.ygain10.setObjectName("ygain10")
        
        # z1 gain group
        self.z1gain_G = QtWidgets.QGroupBox(self.centralwidget)
        self.z1gain_G.setGeometry(QtCore.QRect(10, 350, 210, 55))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.z1gain_G.setFont(font)
        self.z1gain_G.setObjectName("z1gain_G")
        self.z1gain1 = QtWidgets.QRadioButton(self.z1gain_G)
        self.z1gain1.setGeometry(QtCore.QRect(80, 25, 50, 20))
        self.z1gain1.setObjectName("z1gain1")
        self.z1gain0_1 = QtWidgets.QRadioButton(self.z1gain_G)
        self.z1gain0_1.setGeometry(QtCore.QRect(10, 25, 50, 20))
        self.z1gain0_1.setObjectName("z1gain0_1")
        self.z1gain10 = QtWidgets.QRadioButton(self.z1gain_G)
        self.z1gain10.setGeometry(QtCore.QRect(150, 25, 50, 20))
        self.z1gain10.setObjectName("z1gain10")
        
        # z2 gain group
        self.z2gain_G = QtWidgets.QGroupBox(self.centralwidget)
        self.z2gain_G.setGeometry(QtCore.QRect(10, 410, 210, 55))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.z2gain_G.setFont(font)
        self.z2gain_G.setObjectName("z2gain_G")
        self.z2gain1 = QtWidgets.QRadioButton(self.z2gain_G)
        self.z2gain1.setGeometry(QtCore.QRect(80, 25, 50, 20))
        self.z2gain1.setObjectName("z2gain1")
        self.z2gain10 = QtWidgets.QRadioButton(self.z2gain_G)
        self.z2gain10.setGeometry(QtCore.QRect(150, 25, 50, 20))
        self.z2gain10.setObjectName("z2gain10")
        self.z2gain0_1 = QtWidgets.QRadioButton(self.z2gain_G)
        self.z2gain0_1.setGeometry(QtCore.QRect(10, 25, 50, 20))
        self.z2gain0_1.setObjectName("z2gain0_1")
        
        # dither1 group
        self.dither1_G = QtWidgets.QGroupBox(self.centralwidget)
        self.dither1_G.setGeometry(QtCore.QRect(230, 290, 200, 55))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.dither1_G.setFont(font)
        self.dither1_G.setObjectName("dither1_G")
        self.dither1_on = QtWidgets.QRadioButton(self.dither1_G)
        self.dither1_on.setGeometry(QtCore.QRect(10, 25, 60, 20))
        self.dither1_on.setObjectName("dither1_on")
        self.dither1_off = QtWidgets.QRadioButton(self.dither1_G)
        self.dither1_off.setGeometry(QtCore.QRect(110, 25, 60, 20))
        self.dither1_off.setObjectName("dither1_off")
        
        # dither0 group
        self.dither0_G = QtWidgets.QGroupBox(self.centralwidget)
        self.dither0_G.setGeometry(QtCore.QRect(230, 230, 200, 55))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.dither0_G.setFont(font)
        self.dither0_G.setObjectName("dither0_G")
        self.dither0_on = QtWidgets.QRadioButton(self.dither0_G)
        self.dither0_on.setGeometry(QtCore.QRect(10, 25, 60, 20))
        self.dither0_on.setObjectName("dither0_on")
        self.dither0_off = QtWidgets.QRadioButton(self.dither0_G)
        self.dither0_off.setGeometry(QtCore.QRect(110, 25, 60, 20))
        self.dither0_off.setObjectName("dither0_off")
        
        # feedback group
        self.feedback_G = QtWidgets.QGroupBox(self.centralwidget)
        self.feedback_G.setGeometry(QtCore.QRect(230, 350, 200, 55))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.feedback_G.setFont(font)
        self.feedback_G.setObjectName("feedback_G")
        self.feedback_on = QtWidgets.QRadioButton(self.feedback_G)
        self.feedback_on.setGeometry(QtCore.QRect(10, 25, 60, 20))
        self.feedback_on.setObjectName("feedback_on")
        self.feedback_off = QtWidgets.QRadioButton(self.feedback_G)
        self.feedback_off.setGeometry(QtCore.QRect(110, 25, 60, 20))
        self.feedback_off.setObjectName("feedback_off")
        
        # retract group
        self.retract_G = QtWidgets.QGroupBox(self.centralwidget)
        self.retract_G.setGeometry(QtCore.QRect(230, 410, 200, 55))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.retract_G.setFont(font)
        self.retract_G.setObjectName("retract_G")
        self.retract_on = QtWidgets.QRadioButton(self.retract_G)
        self.retract_on.setGeometry(QtCore.QRect(10, 25, 60, 20))
        self.retract_on.setObjectName("retract_on")
        self.retract_off = QtWidgets.QRadioButton(self.retract_G)
        self.retract_off.setGeometry(QtCore.QRect(110, 25, 60, 20))
        self.retract_off.setObjectName("retract_off")
        
        # coarse group
        self.coarse_G = QtWidgets.QGroupBox(self.centralwidget)
        self.coarse_G.setGeometry(QtCore.QRect(440, 230, 260, 55))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.coarse_G.setFont(font)
        self.coarse_G.setObjectName("coarse_G")
        self.fine = QtWidgets.QRadioButton(self.coarse_G)
        self.fine.setGeometry(QtCore.QRect(10, 25, 80, 20))
        self.fine.setObjectName("fine")
        self.coarse = QtWidgets.QRadioButton(self.coarse_G)
        self.coarse.setGeometry(QtCore.QRect(130, 25, 80, 20))
        self.coarse.setObjectName("coarse")
        
        # rot group
        self.rot_G = QtWidgets.QGroupBox(self.centralwidget)
        self.rot_G.setGeometry(QtCore.QRect(440, 290, 260, 55))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.rot_G.setFont(font)
        self.rot_G.setObjectName("rot_G")
        self.rotation = QtWidgets.QRadioButton(self.rot_G)
        self.rotation.setGeometry(QtCore.QRect(10, 25, 105, 20))
        self.rotation.setObjectName("rotation")
        self.translation = QtWidgets.QRadioButton(self.rot_G)
        self.translation.setGeometry(QtCore.QRect(130, 25, 105, 20))
        self.translation.setObjectName("translation")
        
        # ramp button
        self.ramp = QtWidgets.QPushButton(self.centralwidget)
        self.ramp.setGeometry(QtCore.QRect(440, 350, 260, 110))
        font = QtGui.QFont()
        font.setPointSize(24)
        self.ramp.setFont(font)
        self.ramp.setObjectName("ramp")
        
        # other features
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 710, 26))
        self.menubar.setObjectName("menubar")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        self.reinitial(MainWindow, DSP)
        self.connectfunction(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow"))
        self.bit20.setTitle(_translate("MainWindow", "20bit DAC"))
        self.bit20_out.setText(_translate("MainWindow", "20bit output"))
        self.bit20_0.setText(_translate("MainWindow", "Zero"))
        self.dac_G.setTitle(_translate("MainWindow", "DAC"))
        self.dac_ch.setItemText(0, _translate("MainWindow", "DAC0 (XIN)"))
        self.dac_ch.setItemText(1, _translate("MainWindow", "DAC1 (X OFFSET)"))
        self.dac_ch.setItemText(2, _translate("MainWindow", "DAC2 (Z OFFSET FINE)"))
        self.dac_ch.setItemText(3, _translate("MainWindow", "DAC3 (Z OFFSET)"))
        self.dac_ch.setItemText(4, _translate("MainWindow", "DAC4 (ISET OFFSET)"))
        self.dac_ch.setItemText(5, _translate("MainWindow", "DAC5 (ISET)"))
        self.dac_ch.setItemText(6, _translate("MainWindow", "DAC6"))
        self.dac_ch.setItemText(7, _translate("MainWindow", "DAC7"))
        self.dac_ch.setItemText(8, _translate("MainWindow", "DAC8"))
        self.dac_ch.setItemText(9, _translate("MainWindow", "DAC9"))
        self.dac_ch.setItemText(10, _translate("MainWindow", "DAC10 (ZOUTER)"))
        self.dac_ch.setItemText(11, _translate("MainWindow", "DAC11"))
        self.dac_ch.setItemText(12, _translate("MainWindow", "DAC12 (BIAS OFFSET)"))
        self.dac_ch.setItemText(13, _translate("MainWindow", "DAC13 (BIAS)"))
        self.dac_ch.setItemText(14, _translate("MainWindow", "DAC14 (YOFFSET)"))
        self.dac_ch.setItemText(15, _translate("MainWindow", "DAC15 (YIN)"))
        self.dac_out.setText(_translate("MainWindow", "Output"))
        self.dac_Iset_l.setText(_translate("MainWindow", "ISET"))
        self.dac_b20.setText(_translate("MainWindow", "+/-10V"))
        self.dac_b10.setText(_translate("MainWindow", "+/-5V"))
        self.dac_b5.setText(_translate("MainWindow", "+/-2.5V"))
        self.dac_u5.setText(_translate("MainWindow", "0V - 5V"))
        self.dac_u10.setText(_translate("MainWindow", "0V - 10V"))
        self.dac_Iset_unit.setText(_translate("MainWindow", "V"))
        self.dac_0.setText(_translate("MainWindow", "Zero"))
        self.adc_G.setTitle(_translate("MainWindow", "ADC"))
        self.adc_in.setText(_translate("MainWindow", "Input"))
        self.adc_ch.setItemText(0, _translate("MainWindow", "AIN0"))
        self.adc_ch.setItemText(1, _translate("MainWindow", "AIN1"))
        self.adc_ch.setItemText(2, _translate("MainWindow", "AIN2"))
        self.adc_ch.setItemText(3, _translate("MainWindow", "AIN3"))
        self.adc_ch.setItemText(4, _translate("MainWindow", "AIN4"))
        self.adc_ch.setItemText(5, _translate("MainWindow", "AIN5"))
        self.adc_ch.setItemText(6, _translate("MainWindow", "AIN6 (ZOUT)"))
        self.adc_ch.setItemText(7, _translate("MainWindow", "AIN7 (PREAMP)"))
        self.adc_b20.setText(_translate("MainWindow", "+/- 10.24"))
        self.adc_b10.setText(_translate("MainWindow", "+/- 5.12"))
        self.adc_b5.setText(_translate("MainWindow", "+/- 2.56"))
        self.adc_u10.setText(_translate("MainWindow", "0V - 10.24V"))
        self.adc_u5.setText(_translate("MainWindow", "0V - 5.12V"))
        self.xgain_G.setTitle(_translate("MainWindow", "XIN gain"))
        self.xgain1.setText(_translate("MainWindow", "1.0"))
        self.xgain0_1.setText(_translate("MainWindow", "0.1"))
        self.xgain10.setText(_translate("MainWindow", "10"))
        self.ygain_G.setTitle(_translate("MainWindow", "YIN gain"))
        self.ygain1.setText(_translate("MainWindow", "1.0"))
        self.ygain0_1.setText(_translate("MainWindow", "0.1"))
        self.ygain10.setText(_translate("MainWindow", "10"))
        self.z1gain_G.setTitle(_translate("MainWindow", "Z1 gain"))
        self.z1gain1.setText(_translate("MainWindow", "1.0"))
        self.z1gain0_1.setText(_translate("MainWindow", "0.1"))
        self.z1gain10.setText(_translate("MainWindow", "10"))
        self.z2gain_G.setTitle(_translate("MainWindow", "Z2 gain"))
        self.z2gain1.setText(_translate("MainWindow", "1.0"))
        self.z2gain10.setText(_translate("MainWindow", "10"))
        self.z2gain0_1.setText(_translate("MainWindow", "0.1"))
        self.dither1_G.setTitle(_translate("MainWindow", "Dither1"))
        self.dither1_on.setText(_translate("MainWindow", "ON"))
        self.dither1_off.setText(_translate("MainWindow", "OFF"))
        self.dither0_G.setTitle(_translate("MainWindow", "Dither0"))
        self.dither0_on.setText(_translate("MainWindow", "ON"))
        self.dither0_off.setText(_translate("MainWindow", "OFF"))
        self.feedback_G.setTitle(_translate("MainWindow", "Feedback"))
        self.feedback_on.setText(_translate("MainWindow", "ON"))
        self.feedback_off.setText(_translate("MainWindow", "OFF"))
        self.retract_G.setTitle(_translate("MainWindow", "Retract"))
        self.retract_on.setText(_translate("MainWindow", "ON"))
        self.retract_off.setText(_translate("MainWindow", "OFF"))
        self.coarse_G.setTitle(_translate("MainWindow", "Coarse"))
        self.fine.setText(_translate("MainWindow", "Coarse"))
        self.coarse.setText(_translate("MainWindow", "Fine"))
        self.rot_G.setTitle(_translate("MainWindow", "ROT"))
        self.rotation.setText(_translate("MainWindow", "Rotation"))
        self.translation.setText(_translate("MainWindow", "Translation"))
        self.ramp.setText(_translate("MainWindow", "Ramp"))
        
    def reinitial(self, MainWindow, DSP):
        # flag and last ouput
        self.lastdac = [0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000]
        self.dacrange = [0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
        self.adcrange = [0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
        self.lastbit20 = 0x80000
        self.g_xgain = 2
        self.g_ygain = 2
        self.g_z1gain = 2
        self.g_z2gain = 2
        self.g_coarse = 1
        self.g_rot = 1
        self.g_dither0 = 0
        self.g_dither1 = 0
        self.g_retract = 0
        self.g_feedback = 1
        self.Dsp = DSP
        
        self.dac_i = 0
        self.adc_i = 0
        self.dac_ll_list = [-10.0, -5.0, -2.5, 0.0, 0.0]
        self.dac_ul_list = [10.0, 5.0, 2.5, 5.0, 10.0]
        self.adc_ll_list = [-10.24, -5.12, -2.56, 0.0, 0.0]
        self.adc_ul_list = [10.24, 5.12, 2.56, 5.12, 10.24]
        
        self.xgain1.setChecked(True)
        self.ygain1.setChecked(True)
        self.z1gain1.setChecked(True)
        self.z2gain1.setChecked(True)
        
        self.dither0_off.setChecked(True)
        self.dither1_off.setChecked(True)
        self.retract_off.setChecked(True)
        self.feedback_on.setChecked(True)
        self.coarse.setChecked(True)
        self.translation.setChecked(True)
        
        self.dac_b20.setChecked(True)
        self.adc_b20.setChecked(True)
        
        self.dac_bit.setValue(0x8000)
        self.bit20_bit.setValue(0x80000)
        self.dac_Iset_val.setValue(1.0)
        
    def connectfunction(self, Mainwindow):
        self.dac_ch.currentIndexChanged.connect(self.dac_channel_change)
        self.adc_ch.currentIndexChanged.connect(self.adc_channel_change)
        self.dac_bit.valueChanged.connect(self.dac_b2v)
        self.dac_val.editingFinished.connect(self.dac_v2b)
        self.dac_0.clicked.connect(self.dac_0_click)
        self.dac_out.clicked.connect(self.dac_out_click)
        self.dac_b20.clicked.connect(self.dac_toggle)
        self.dac_b10.clicked.connect(self.dac_toggle)
        self.dac_b5.clicked.connect(self.dac_toggle)
        self.dac_u5.clicked.connect(self.dac_toggle)
        self.dac_u10.clicked.connect(self.dac_toggle)
        
        self.bit20_bit.valueChanged.connect(self.bit20_b2v)
        self.bit20_val.editingFinished.connect(self.bit20_v2b)
        self.bit20_0.clicked.connect(self.bit20_0_click)
        self.bit20_out.clicked.connect(self.bit20_out_click)
        
        self.adc_b20.clicked.connect(self.adc_toggle)
        self.adc_b10.clicked.connect(self.adc_toggle)
        self.adc_b5.clicked.connect(self.adc_toggle)
        self.adc_u5.clicked.connect(self.adc_toggle)
        self.adc_u10.clicked.connect(self.adc_toggle)
        self.adc_in.clicked.connect(self.adc_in_click)
        
        self.xgain0_1.clicked.connect(self.xgain_toggle)
        self.xgain1.clicked.connect(self.xgain_toggle)
        self.xgain10.clicked.connect(self.xgain_toggle)
        
        self.ygain0_1.clicked.connect(self.ygain_toggle)
        self.ygain1.clicked.connect(self.ygain_toggle)
        self.ygain10.clicked.connect(self.ygain_toggle)
        
        self.z1gain0_1.clicked.connect(self.z1gain_toggle)
        self.z1gain1.clicked.connect(self.z1gain_toggle)
        self.z1gain10.clicked.connect(self.z1gain_toggle)
        
        self.z2gain0_1.clicked.connect(self.z2gain_toggle)
        self.z2gain1.clicked.connect(self.z2gain_toggle)
        self.z2gain10.clicked.connect(self.z2gain_toggle)
        
        self.feedback_on.toggled.connect(self.feedback_toggle)
        # self.feedback_off.toggled.connect(self.feedback_toggle)
        self.dither0_on.toggled.connect(self.dither0_toggle)
        # self.dither0_off.toggled.connect(self.dither0_toggle)
        self.dither1_on.toggled.connect(self.dither1_toggle)
        # self.dither1_off.toggled.connect(self.dither1_toggle)
        self.retract_on.toggled.connect(self.retract_toggle)
        # self.retract_off.toggled.connect(self.retract_toggle)
        # self.coarse.toggled.connect(self.coarse_toggle)
        self.fine.toggled.connect(self.coarse_toggle)
        # self.rotation.toggled.connect(self.rot_toggle)
        self.translation.toggled.connect(self.rot_toggle)
        
    def adc_in_click(self):
        b = self.Dsp.adc(192 + (4 * self.adc_i))
        ll = self.adc_ll_list[self.adcrange[self.adc_i]]        # obtain current dac channel lower limit
        ul = self.adc_ul_list[self.adcrange[self.adc_i]]        # obtain current dac channel upper limit
        val = b * (ul - ll) / (2**16) + ll
        self.adc_val.setValue(val)
        
    def dac_out_click(self):
        self.Dsp.dac_W(self.dac_i + 16, self.dac_bit.value())
        self.lastdac[self.dac_i] = self.dac_bit.value()
    
    def bit20_out_click(self):
        self.Dsp.bit20_W(0x10, self.bit20_bit.value())
        self.lastbit20 = self.bit20_bit.value()
        
    def dac_0_click(self):
        if self.dacrange[self.dac_i] < 3:
            self.dac_bit.setValue(0x8000)
        else:
            self.dac_bit.setValue(0x0000)
        self.dac_out_click()
            
    def bit20_0_click(self):
        self.bit20_bit.setValue(0x80000)
        self.bit20_out_click()
        
    def dac_channel_change(self, i):
        self.dac_i = i
        
    def adc_channel_change(self, i):
        self.adc_i = i
        
    def dac_b2v(self, b):
        ll = self.dac_ll_list[self.dacrange[self.dac_i]]        # obtain current dac channel lower limit
        ul = self.dac_ul_list[self.dacrange[self.dac_i]]        # obtain current dac channel upper limit
        val = b * (ul - ll) / (2**16) + ll
        self.dac_val.setValue(val)
        if self.dac_i == 5:
            self.dac_Iset_val.setValue(10**(-val/10))            

    def dac_v2b(self):
        val = self.dac_val.value()
        ll = self.dac_ll_list[self.dacrange[self.dac_i]]        # obtain current dac channel lower limit
        ul = self.dac_ul_list[self.dacrange[self.dac_i]]        # obtain current dac channel upper limit
        b = int((val - ll) * (2**16) / (ul - ll))
        self.dac_bit.setValue(b)
        
    def bit20_b2v(self, b):
        val = b * 10.0 / (2**20) -5.0
        self.bit20_val.setValue(val)

    def bit20_v2b(self):
        val = self.bit20_val.value()
        b = int((val + 5.0) * (2**20) / 10.0)
        self.bit20_bit.setValue(b)
        
    def dac_toggle(self):
        if self.dac_b20.isChecked():
            self.Dsp.dac_range(self.dac_i, self.Dsp.DAC_B20)
            self.dacrange[self.dac_i] = 0
        elif self.dac_b10.isChecked():
            self.Dsp.dac_range(self.dac_i, self.Dsp.DAC_B10)
            self.dacrange[self.dac_i] = 1
        elif self.dac_b5.isChecked():
            self.Dsp.dac_range(self.dac_i, self.Dsp.DAC_B5)
            self.dacrange[self.dac_i] = 2
        elif self.dac_u5.isChecked():
            self.Dsp.dac_range(self.dac_i, self.Dsp.DAC_U5)
            self.dacrange[self.dac_i] = 3
        elif self.dac_u10.isChecked():
            self.Dsp.dac_range(self.dac_i, self.Dsp.DAC_U10)
            self.dacrange[self.dac_i] = 4
        b = self.dac_bit.value()
        self.dac_b2v(b)
        
    def adc_toggle(self):
        if self.adc_b20.isChecked():
            self.Dsp.adc_W(self.Dsp.ADC_RANGE0, self.Dsp.ADC_B20)
            self.adcrange[self.adc_i] = 0
        elif self.adc_b10.isChecked():
            self.Dsp.adc_W(self.Dsp.ADC_RANGE0, self.Dsp.ADC_B10)
            self.adcrange[self.adc_i] = 1
        elif self.adc_b5.isChecked():
            self.Dsp.adc_W(self.Dsp.ADC_RANGE0, self.Dsp.ADC_B5)
            self.adcrange[self.adc_i] = 2
        elif self.adc_u5.isChecked():
            self.Dsp.adc_W(self.Dsp.ADC_RANGE0, self.Dsp.ADC_U10)
            self.adcrange[self.adc_i] = 4
        elif self.adc_u10.isChecked():
            self.Dsp.adc_W(self.Dsp.ADC_RANGE0, self.Dsp.ADC_U5)
            self.adcrange[self.adc_i] = 3
            
    def xgain_toggle(self):
        if self.xgain0_1.isChecked():
            self.g_xgain = 0
        elif self.xgain1.isChecked():
            self.g_xgain = 2
        elif self.xgain10.isChecked():
            self.g_xgain = 3
        self.Dsp.gain(self.Dsp.XGAIN, self.g_xgain)
    
    def ygain_toggle(self):
        if self.ygain0_1.isChecked():
            self.g_ygain = 0
        elif self.ygain1.isChecked():
            self.g_ygain = 2
        elif self.ygain10.isChecked():
            self.g_ygain = 3
        self.Dsp.gain(self.Dsp.YGAIN, self.g_ygain)
    
    def z1gain_toggle(self):
        if self.z1gain0_1.isChecked():
            self.g_z1gain = 0
        elif self.z1gain1.isChecked():
            self.g_z1gain = 2
        elif self.z1gain10.isChecked():
            self.g_z1gain = 3
        self.Dsp.gain(self.Dsp.Z1GAIN, self.g_z1gain)
    
    def z2gain_toggle(self):
        if self.z2gain0_1.isChecked():
            self.g_z2gain = 0
        elif self.z2gain1.isChecked():
            self.g_z2gain = 2
        elif self.xgain10.isChecked():
            self.g_z2gain = 3
        self.Dsp.gain(self.Dsp.Z2GAIN, self.g_z2gain)
            
    def feedback_toggle(self, i):
        self.g_feedback = i
        self.Dsp.digitalIO(self.Dsp.FEEDBACK, self.g_feedback)
        
    def dither0_toggle(self, i):
        self.g_dither0 = i
        self.Dsp.digitalIO(self.Dsp.DITHER_0, self.g_dither0)
            
    def dither1_toggle(self, i):
        self.g_dither1 = i
        self.Dsp.digitalIO(self.Dsp.DITHER_1, self.g_dither1)
            
    def retract_toggle(self, i):
        self.g_retract = i
        self.Dsp.digitalIO(self.Dsp.RETRACT, self.g_retract)
            
    def coarse_toggle(self, i):
        self.g_coarse = i
        self.Dsp.digitalIO(self.Dsp.COARSE, self.g_coarse)
    
    def rot_toggle(self, i):
        self.g_rot = i
        self.Dsp.digitalIO(self.Dsp.TRANSLATION, self.g_rot)


if __name__ == "__main__":
    import sys
    with dsp('com4', 38400) as DSP:
        app = QtWidgets.QApplication(sys.argv)
        MainWindow = QtWidgets.QMainWindow()
        ui = Ui_MainWindow()
        ui.setupUi(MainWindow, DSP)
        MainWindow.show()
        sys.exit(app.exec_())

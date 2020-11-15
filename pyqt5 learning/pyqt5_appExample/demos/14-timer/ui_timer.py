# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'timer.ui'
#
# Created by: PyQt5 UI code generator 5.13.0
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_Form(object):
    def setupUi(self, Form):
        Form.setObjectName("Form")
        Form.resize(400, 300)
        self.label = QtWidgets.QLabel(Form)
        self.label.setGeometry(QtCore.QRect(180, 60, 91, 31))
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(12)
        self.label.setFont(font)
        self.label.setObjectName("label")
        self.lcdNumber = QtWidgets.QLCDNumber(Form)
        self.lcdNumber.setGeometry(QtCore.QRect(170, 120, 131, 31))
        self.lcdNumber.setObjectName("lcdNumber")
        self.label_2 = QtWidgets.QLabel(Form)
        self.label_2.setGeometry(QtCore.QRect(90, 70, 81, 16))
        self.label_2.setObjectName("label_2")
        self.label_3 = QtWidgets.QLabel(Form)
        self.label_3.setGeometry(QtCore.QRect(90, 130, 72, 15))
        self.label_3.setObjectName("label_3")
        self.btn_start = QtWidgets.QPushButton(Form)
        self.btn_start.setGeometry(QtCore.QRect(40, 210, 93, 28))
        self.btn_start.setObjectName("btn_start")
        self.btn_once = QtWidgets.QPushButton(Form)
        self.btn_once.setGeometry(QtCore.QRect(150, 210, 93, 28))
        self.btn_once.setObjectName("btn_once")
        self.btn_stop = QtWidgets.QPushButton(Form)
        self.btn_stop.setGeometry(QtCore.QRect(260, 210, 93, 28))
        self.btn_stop.setObjectName("btn_stop")

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        _translate = QtCore.QCoreApplication.translate
        Form.setWindowTitle(_translate("Form", "Form"))
        self.label.setText(_translate("Form", "TextLabel"))
        self.label_2.setText(_translate("Form", "Label显示:"))
        self.label_3.setText(_translate("Form", "LCD显示:"))
        self.btn_start.setText(_translate("Form", "开始"))
        self.btn_once.setText(_translate("Form", "一次"))
        self.btn_stop.setText(_translate("Form", "停止"))

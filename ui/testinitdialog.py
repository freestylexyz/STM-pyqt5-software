# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'testinitdialog.ui'
#
# Created by: PyQt5 UI code generator 5.12.3
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_TestInitDialog(object):
    def setupUi(self, TestInitDialog):
        TestInitDialog.setObjectName("TestInitDialog")
        TestInitDialog.resize(237, 102)
        self.gridLayout = QtWidgets.QGridLayout(TestInitDialog)
        self.gridLayout.setObjectName("gridLayout")
        self.label = QtWidgets.QLabel(TestInitDialog)
        self.label.setObjectName("label")
        self.gridLayout.addWidget(self.label, 0, 0, 1, 1)
        self.buttonBox = QtWidgets.QDialogButtonBox(TestInitDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Cancel|QtWidgets.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.gridLayout.addWidget(self.buttonBox, 1, 0, 1, 1)

        self.retranslateUi(TestInitDialog)
        self.buttonBox.rejected.connect(TestInitDialog.reject)
        self.buttonBox.accepted.connect(TestInitDialog.accept)
        QtCore.QMetaObject.connectSlotsByName(TestInitDialog)

    def retranslateUi(self, TestInitDialog):
        _translate = QtCore.QCoreApplication.translate
        TestInitDialog.setWindowTitle(_translate("TestInitDialog", "Dialog"))
        self.label.setText(_translate("TestInitDialog", "Conform to initialize all settings?"))

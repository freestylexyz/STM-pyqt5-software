# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'settinginitdialog.ui'
#
# Created by: PyQt5 UI code generator 5.12.3
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_SettingInitDialog(object):
    def setupUi(self, SettingInitDialog):
        SettingInitDialog.setObjectName("SettingInitDialog")
        SettingInitDialog.resize(237, 102)
        self.gridLayout = QtWidgets.QGridLayout(SettingInitDialog)
        self.gridLayout.setObjectName("gridLayout")
        self.label = QtWidgets.QLabel(SettingInitDialog)
        self.label.setObjectName("label")
        self.gridLayout.addWidget(self.label, 0, 0, 1, 1)
        self.buttonBox = QtWidgets.QDialogButtonBox(SettingInitDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.gridLayout.addWidget(self.buttonBox, 1, 0, 1, 1)

        self.retranslateUi(SettingInitDialog)
        self.buttonBox.rejected.connect(SettingInitDialog.reject)
        self.buttonBox.accepted.connect(SettingInitDialog.accept)
        QtCore.QMetaObject.connectSlotsByName(SettingInitDialog)

    def retranslateUi(self, SettingInitDialog):
        _translate = QtCore.QCoreApplication.translate
        SettingInitDialog.setWindowTitle(_translate("SettingInitDialog", "Dialog"))
        self.label.setText(_translate("SettingInitDialog", "Setting initialization completed."))

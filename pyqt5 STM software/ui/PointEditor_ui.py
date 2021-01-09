# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'PointEditor.ui'
#
# Created by: PyQt5 UI code generator 5.12.3
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_PointEditor(object):
    def setupUi(self, PointEditor):
        PointEditor.setObjectName("PointEditor")
        PointEditor.resize(521, 275)
        self.gridLayout = QtWidgets.QGridLayout(PointEditor)
        self.gridLayout.setObjectName("gridLayout")
        self.tableWidget = QtWidgets.QTableWidget(PointEditor)
        self.tableWidget.setObjectName("tableWidget")
        self.tableWidget.setColumnCount(2)
        self.tableWidget.setRowCount(0)
        item = QtWidgets.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(0, item)
        item = QtWidgets.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(1, item)
        self.gridLayout.addWidget(self.tableWidget, 0, 0, 1, 1)
        self.widget = QtWidgets.QWidget(PointEditor)
        self.widget.setObjectName("widget")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.widget)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.pushButton_4 = QtWidgets.QPushButton(self.widget)
        self.pushButton_4.setObjectName("pushButton_4")
        self.gridLayout_2.addWidget(self.pushButton_4, 2, 1, 1, 1)
        self.pushButton_2 = QtWidgets.QPushButton(self.widget)
        self.pushButton_2.setObjectName("pushButton_2")
        self.gridLayout_2.addWidget(self.pushButton_2, 7, 0, 1, 2)
        self.pushButton_6 = QtWidgets.QPushButton(self.widget)
        self.pushButton_6.setObjectName("pushButton_6")
        self.gridLayout_2.addWidget(self.pushButton_6, 0, 0, 1, 1)
        self.pushButton_7 = QtWidgets.QPushButton(self.widget)
        self.pushButton_7.setObjectName("pushButton_7")
        self.gridLayout_2.addWidget(self.pushButton_7, 2, 0, 1, 1)
        self.pushButton = QtWidgets.QPushButton(self.widget)
        self.pushButton.setObjectName("pushButton")
        self.gridLayout_2.addWidget(self.pushButton, 0, 1, 1, 1)
        self.groupBox = QtWidgets.QGroupBox(self.widget)
        self.groupBox.setObjectName("groupBox")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.groupBox)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.spinBox_2 = QtWidgets.QSpinBox(self.groupBox)
        self.spinBox_2.setMinimum(-32768)
        self.spinBox_2.setMaximum(32767)
        self.spinBox_2.setObjectName("spinBox_2")
        self.gridLayout_3.addWidget(self.spinBox_2, 1, 1, 1, 1)
        self.spinBox = QtWidgets.QSpinBox(self.groupBox)
        self.spinBox.setMinimum(-32768)
        self.spinBox.setMaximum(32767)
        self.spinBox.setObjectName("spinBox")
        self.gridLayout_3.addWidget(self.spinBox, 0, 1, 1, 1)
        self.label = QtWidgets.QLabel(self.groupBox)
        self.label.setObjectName("label")
        self.gridLayout_3.addWidget(self.label, 0, 0, 1, 1)
        self.label_2 = QtWidgets.QLabel(self.groupBox)
        self.label_2.setObjectName("label_2")
        self.gridLayout_3.addWidget(self.label_2, 1, 0, 1, 1)
        self.gridLayout_2.addWidget(self.groupBox, 4, 0, 1, 2)
        self.pushButton_5 = QtWidgets.QPushButton(self.widget)
        self.pushButton_5.setObjectName("pushButton_5")
        self.gridLayout_2.addWidget(self.pushButton_5, 3, 0, 1, 2)
        self.gridLayout.addWidget(self.widget, 0, 1, 1, 1)

        self.retranslateUi(PointEditor)
        QtCore.QMetaObject.connectSlotsByName(PointEditor)

    def retranslateUi(self, PointEditor):
        _translate = QtCore.QCoreApplication.translate
        PointEditor.setWindowTitle(_translate("PointEditor", "Point Editor"))
        item = self.tableWidget.horizontalHeaderItem(0)
        item.setText(_translate("PointEditor", "X"))
        item = self.tableWidget.horizontalHeaderItem(1)
        item.setText(_translate("PointEditor", "Y"))
        self.pushButton_4.setText(_translate("PointEditor", "Clear all"))
        self.pushButton_2.setText(_translate("PointEditor", "Done"))
        self.pushButton_6.setText(_translate("PointEditor", "Add point"))
        self.pushButton_7.setText(_translate("PointEditor", "Insert point"))
        self.pushButton.setText(_translate("PointEditor", "Remove selected"))
        self.groupBox.setTitle(_translate("PointEditor", "Reference"))
        self.label.setText(_translate("PointEditor", "X"))
        self.label_2.setText(_translate("PointEditor", "Y"))
        self.pushButton_5.setText(_translate("PointEditor", "Load points from .spc"))


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    PointEditor = QtWidgets.QWidget()
    ui = Ui_PointEditor()
    ui.setupUi(PointEditor)
    PointEditor.show()
    sys.exit(app.exec_())

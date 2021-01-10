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
        PointEditor.resize(424, 282)
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
        self.pushButton_Insert = QtWidgets.QPushButton(self.widget)
        self.pushButton_Insert.setObjectName("pushButton_Insert")
        self.gridLayout_2.addWidget(self.pushButton_Insert, 2, 0, 1, 1)
        self.pushButton_Load = QtWidgets.QPushButton(self.widget)
        self.pushButton_Load.setObjectName("pushButton_Load")
        self.gridLayout_2.addWidget(self.pushButton_Load, 5, 0, 1, 1)
        self.pushButton_Add = QtWidgets.QPushButton(self.widget)
        self.pushButton_Add.setObjectName("pushButton_Add")
        self.gridLayout_2.addWidget(self.pushButton_Add, 0, 0, 1, 1)
        self.pushButton_Clear = QtWidgets.QPushButton(self.widget)
        self.pushButton_Clear.setObjectName("pushButton_Clear")
        self.gridLayout_2.addWidget(self.pushButton_Clear, 3, 0, 1, 1)
        self.pushButton_Done = QtWidgets.QPushButton(self.widget)
        self.pushButton_Done.setObjectName("pushButton_Done")
        self.gridLayout_2.addWidget(self.pushButton_Done, 6, 0, 1, 1)
        self.pushButton_Remove = QtWidgets.QPushButton(self.widget)
        self.pushButton_Remove.setObjectName("pushButton_Remove")
        self.gridLayout_2.addWidget(self.pushButton_Remove, 1, 0, 1, 1)
        self.pushButton_Save = QtWidgets.QPushButton(self.widget)
        self.pushButton_Save.setObjectName("pushButton_Save")
        self.gridLayout_2.addWidget(self.pushButton_Save, 4, 0, 1, 1)
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
        self.pushButton_Insert.setText(_translate("PointEditor", "Insert point"))
        self.pushButton_Load.setText(_translate("PointEditor", "Load points"))
        self.pushButton_Add.setText(_translate("PointEditor", "Add point"))
        self.pushButton_Clear.setText(_translate("PointEditor", "Clear all"))
        self.pushButton_Done.setText(_translate("PointEditor", "Done"))
        self.pushButton_Remove.setText(_translate("PointEditor", "Remove selected"))
        self.pushButton_Save.setText(_translate("PointEditor", "Save as .pts"))


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    PointEditor = QtWidgets.QWidget()
    ui = Ui_PointEditor()
    ui.setupUi(PointEditor)
    PointEditor.show()
    sys.exit(app.exec_())

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'SequenceList.ui'
#
# Created by: PyQt5 UI code generator 5.12.3
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_SequenceList(object):
    def setupUi(self, SequenceList):
        SequenceList.setObjectName("SequenceList")
        SequenceList.setWindowModality(QtCore.Qt.ApplicationModal)
        SequenceList.resize(479, 322)
        self.gridLayout_2 = QtWidgets.QGridLayout(SequenceList)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.widget_2 = QtWidgets.QWidget(SequenceList)
        self.widget_2.setObjectName("widget_2")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.widget_2)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.listWidget_SeqList = QtWidgets.QListWidget(self.widget_2)
        self.listWidget_SeqList.setObjectName("listWidget_SeqList")
        self.gridLayout_3.addWidget(self.listWidget_SeqList, 0, 0, 1, 1)
        self.gridLayout_2.addWidget(self.widget_2, 0, 0, 1, 1)
        self.widget = QtWidgets.QWidget(SequenceList)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.widget.sizePolicy().hasHeightForWidth())
        self.widget.setSizePolicy(sizePolicy)
        self.widget.setMinimumSize(QtCore.QSize(180, 300))
        self.widget.setObjectName("widget")
        self.gridLayout = QtWidgets.QGridLayout(self.widget)
        self.gridLayout.setVerticalSpacing(14)
        self.gridLayout.setObjectName("gridLayout")
        self.pushButton_Load_SeqList = QtWidgets.QPushButton(self.widget)
        self.pushButton_Load_SeqList.setMinimumSize(QtCore.QSize(160, 36))
        self.pushButton_Load_SeqList.setObjectName("pushButton_Load_SeqList")
        self.gridLayout.addWidget(self.pushButton_Load_SeqList, 0, 0, 1, 1)
        self.pushButton_Build_SeqList = QtWidgets.QPushButton(self.widget)
        self.pushButton_Build_SeqList.setMinimumSize(QtCore.QSize(160, 36))
        self.pushButton_Build_SeqList.setObjectName("pushButton_Build_SeqList")
        self.gridLayout.addWidget(self.pushButton_Build_SeqList, 1, 0, 1, 1)
        self.pushButton_Edit_SeqList = QtWidgets.QPushButton(self.widget)
        self.pushButton_Edit_SeqList.setMinimumSize(QtCore.QSize(160, 36))
        self.pushButton_Edit_SeqList.setObjectName("pushButton_Edit_SeqList")
        self.gridLayout.addWidget(self.pushButton_Edit_SeqList, 2, 0, 1, 1)
        self.pushButton_Remove_SeqList = QtWidgets.QPushButton(self.widget)
        self.pushButton_Remove_SeqList.setMinimumSize(QtCore.QSize(160, 36))
        self.pushButton_Remove_SeqList.setObjectName("pushButton_Remove_SeqList")
        self.gridLayout.addWidget(self.pushButton_Remove_SeqList, 3, 0, 1, 1)
        self.pushButton_Select_SeqList = QtWidgets.QPushButton(self.widget)
        self.pushButton_Select_SeqList.setMinimumSize(QtCore.QSize(160, 36))
        self.pushButton_Select_SeqList.setObjectName("pushButton_Select_SeqList")
        self.gridLayout.addWidget(self.pushButton_Select_SeqList, 4, 0, 1, 1)
        self.gridLayout_2.addWidget(self.widget, 0, 1, 1, 1)

        self.retranslateUi(SequenceList)
        QtCore.QMetaObject.connectSlotsByName(SequenceList)

    def retranslateUi(self, SequenceList):
        _translate = QtCore.QCoreApplication.translate
        SequenceList.setWindowTitle(_translate("SequenceList", "Sequence List"))
        self.pushButton_Load_SeqList.setText(_translate("SequenceList", "Load Sequence to list"))
        self.pushButton_Build_SeqList.setText(_translate("SequenceList", "Build New Sequence"))
        self.pushButton_Edit_SeqList.setText(_translate("SequenceList", "Edit Current Sequence"))
        self.pushButton_Remove_SeqList.setText(_translate("SequenceList", "Remove Current Sequence"))
        self.pushButton_Select_SeqList.setText(_translate("SequenceList", "Select Current Sequence"))


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    SequenceList = QtWidgets.QWidget()
    ui = Ui_SequenceList()
    ui.setupUi(SequenceList)
    SequenceList.show()
    sys.exit(app.exec_())

# encoding=utf-8

import sys
import PyQt5.QtWidgets as qw
import ui_spinbox

# sys.setrecursionlimit(1000000)

class myForm(qw.QMainWindow, ui_spinbox.Ui_MainWindow):
    def __init__(self):
        super().__init__()
        self.setupUi(self)

        self.statusbar.showMessage("status:ok")
        # spinbox1
        self.spinBox.setRange(-10, 100)
        self.spinBox.setSingleStep(10)
        self.spinBox.setValue(10)
        self.spinBox.setWrapping(True)
        # spinbox2
        self.spinBox_2.setRange(-10, 100)
        self.spinBox_2.setSingleStep(10)
        self.spinBox_2.setValue(10)
        self.spinBox_2.setWrapping(True)
        self.spinBox_2.setPrefix("前缀君")
        self.spinBox_2.setSuffix("后缀君")

        self.spinBox.valueChanged.connect(self.spinBox_cb)
        self.spinBox_2.valueChanged.connect(self.spinBox2_cb)

    def spinBox_cb(self, value):
        print("spinbox1's current value is ", value)

    def spinBox2_cb(self, value):
        print("spinbox2's current value is ", value)

if __name__ == '__main__':
    app = qw.QApplication(sys.argv)
    w1 = myForm()
    w1.show()

    app.exec_()
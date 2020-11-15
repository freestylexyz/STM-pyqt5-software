# encoding=utf-8

import sys
import PyQt5.QtWidgets as qw
from PyQt5.QtCore import QTimer
import ui_timer
import datetime

class myForm(qw.QWidget, ui_timer.Ui_Form):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        # self.lcdNumber.setDecMode()  #设置显示模式
        self.lcdNumber.setSegmentStyle(self.lcdNumber.Flat)
        self.lcdNumber.setDigitCount(10)
        self.label.setText("0")
        # self.lcdNumber.setProperty()

        self.btn_start.clicked.connect(self.btn_start_cb)
        self.btn_once.clicked.connect(self.btn_once_cb)
        self.btn_stop.clicked.connect(self.btn_stop_cb)

        self.my_timer = QTimer(self)
        self.my_timer.timeout.connect(self.my_timer_cb)
        self.index = 0

    def text_changed_cb(self):
        value = "hello world"
        self.lcdNumber.display(str(value))
        self.label.setText(value)

    def my_timer_cb(self):
        self.index += 1
        current_timer = datetime.datetime.now().strftime('%Y_%m_%d-%H-%M-%S-%f')
        print("[%s] self.index is %d" % (current_timer, self.index) )
        self.lcdNumber.display(str(self.index))
        self.label.setText(str(self.index))

    def btn_start_cb(self):
        print("you pressed btn_start")
        self.my_timer.start(1000)

    def btn_once_cb(self):
        self.my_timer.setSingleShot(True)
        print("setSingleShot is True")

    def btn_stop_cb(self):
        print("you pressed btn_stop")
        self.my_timer.stop()

if __name__ == '__main__':
    app = qw.QApplication(sys.argv)
    w = myForm()
    w.show()

    sys.exit(app.exec_())
# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:49:07
@Author   : milier00
@FileName : TipApproach.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication , QWidget , QDesktopWidget
from PyQt5.QtCore import pyqtSignal , Qt
from TipApproach_ui import Ui_TipApproach



class myTipApproach(QWidget, Ui_TipApproach):
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        screen = QDesktopWidget().screenGeometry()
        size = self.frameGeometry()
        self.move(int((screen.width()-size.width())/2), int((screen.height()-size.height())/2))
        self.setFixedSize(self.width(), self.height())
    
    def init_tipAppr(self):
        pass
    
    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit()
        event.accept()
    
    # Enable serial
    def enable_serial(self, enable):
        self.pushButton_Up_Steps.setEnabled(enable)
        self.pushButton_Down_Steps.setEnabled(enable)
        self.pushButton_TipAppr_Steps.setEnabled(enable)
        self.groupBox_Trans_TipAppr.setEnabled(enable)
        self.pushButton_STOP_Steps.setEnabled(enable)



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myTipApproach()
    window.show()
    sys.exit(app.exec_())
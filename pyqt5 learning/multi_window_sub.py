# -*- coding: utf-8 -*-
"""
Created on Mon Nov  2 12:42:07 2020

@author: yaoji
"""

import sys
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

class SWinForm(QWidget):
    signal1 = pyqtSignal()
    signal2 = pyqtSignal()
    
    def __init__(self, parent=None):
        super(SWinForm, self).__init__(parent)
        self.resize(400, 90)
        self.setWindowTitle('test multi windows sub')

        self.label = QLabel(self)
        self.label.setText("Hello World")
        self.button1 = QPushButton('sbutton1')
        self.button1.clicked.connect(self.signal1)

        self.button2 = QPushButton('sbutton2')
        self.button2.clicked.connect(self.signal2)

        gridLayout = QGridLayout()
        gridLayout.addWidget(self.label)
        gridLayout.addWidget(self.button1)
        gridLayout.addWidget(self.button2)
        self.setLayout(gridLayout)

    # def onButton1Click(self):
    #     print('click button1')


    # def onButton2Click(self):
    #     print('click button2')



if __name__ == "__main__":
    app = QApplication(sys.argv)
    form = SWinForm()
    form.show()
    sys.exit(app.exec_())
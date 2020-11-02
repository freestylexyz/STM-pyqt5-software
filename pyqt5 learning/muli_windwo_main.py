# -*- coding: utf-8 -*-
"""
Created on Mon Nov  2 12:33:54 2020

@author: yaoji
"""

import sys
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from multi_window_sub import SWinForm


class WinForm(QWidget):
    def __init__(self, parent=None):
        super(WinForm, self).__init__(parent)
        self.resize(400, 90)
        self.setWindowTitle('test multi windows main')

        self.label = QLabel(self)
        self.label.setText("Hello World")
        self.buttons = QPushButton('subwindow')
        self.buttons.clicked.connect(self.onButtonsClick)

        self.button1 = QPushButton('button1')
        self.button1.clicked.connect(self.onButton1Click)
        
        self.button2 = QPushButton('button2')
        self.button2.clicked.connect(self.onButton2Click)

        gridLayout = QGridLayout()
        gridLayout.addWidget(self.label)
        gridLayout.addWidget(self.buttons)
        gridLayout.addWidget(self.button1)
        gridLayout.addWidget(self.button2)
        self.setLayout(gridLayout)
        
        self.swin = SWinForm()
        self.swin.signal1.connect(self.onSignal1)
        self.swin.signal2.connect(self.onSignal2)
        
    
    def onButtonsClick(self):
        self.swin.show()
        

    def onButton1Click(self):
        self.swin.label.setText('clicked main window button1')


    def onButton2Click(self):
        self.swin.label.setText('clicked main window button2')
        
    def onSignal1(self):
        self.label.setText('clicked main window sbutton1')
    
    def onSignal2(self):
        self.label.setText('clicked main window sbutton2')



if __name__ == "__main__":
    app = QApplication(sys.argv)
    form = WinForm()
    form.show()
    sys.exit(app.exec_())

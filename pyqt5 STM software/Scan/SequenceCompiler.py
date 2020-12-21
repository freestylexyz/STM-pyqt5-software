# -*- coding: utf-8 -*-
"""
Created on Fri Dec 18 11:04:23 2020

@author: yaoji
"""

import sys
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QWidget, QTableWidget, QHBoxLayout, QApplication, QTableWidgetItem, QAbstractItemView ,QComboBox, QPushButton

class mySequenceComplier(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()
        
    def initUI(self):
        self.cl = []
        self.i = 0
        self.setWindowTitle("Sequence Compiler")
        self.resize(670, 400)
        self.conLayout = QHBoxLayout()
        self.conLayout = QHBoxLayout()
        self.table = QTableWidget()
        self.table.setRowCount(self.i)
        self.table.setColumnCount(5)
        
        self.button_addrow = QPushButton('Add Row')
        self.button_drow = QPushButton('Delete Row')
        self.button_fetch = QPushButton('Print value')
        
        
        self.conLayout.addWidget(self.table)
        self.conLayout.addWidget(self.button_addrow)
        self.conLayout.addWidget(self.button_drow)
        self.conLayout.addWidget(self.button_fetch)
        
        self.button_addrow.clicked.connect(self.addrow)
        self.button_drow.clicked.connect(self.drow)
        self.button_fetch.clicked.connect(self.printvalue)
        
        self.table.setHorizontalHeaderLabels(['Command','Channel','Option 1', 'Option 2', 'Units'])  
        self.setLayout(self.conLayout)
    
    def drow(self):
        if self.i > 0:
            self.i -= 1
            self.table.setRowCount(self.i)
    
    def addrow(self):
        self.i += 1
        self.table.setRowCount(self.i)
        command = QComboBox()
        command.addItem('a')
        command.addItem('b')
        command.setStyleSheet("QComboBox{margin:3px};")
        self.cl += [command]
        self.table.setCellWidget(self.i - 1, 0, self.cl[self.i - 1])
        
    def printvalue(self):
        for i in range(self.i):
            # print(self.cl[i].currentIndex())
            print(self.table.item(i, 1).text())
        
    
if __name__ == '__main__':
    app = QApplication(sys.argv)
    sc = mySequenceComplier()
    sc.show()
    sys.exit(app.exec_())
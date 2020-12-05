# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 16:04:05 2020

@author: yaoji
"""

import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication, QMainWindow, QDesktopWidget
from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSignal, Qt, QMetaObject, QSettings
from Setting import mySetting
from TipApproach import myTipApproach
from Etest import myEtest
from MainMenu import myMainMenu
import conversion as cnv
import threading

class myZcontroller(myMainMenu):
    # Init Zcontroller dock
    def init_Zcontroller_dock(self):
        self.init_Zcontroller()
    
    # Show Zcontroller dock
    def Zcontroller_show(self):
        self.init_Zcontroller()
        self.Zcontrol.show()
    
    def init_Zcontroller(self):
        pass

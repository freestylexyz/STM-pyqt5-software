# -*- coding: utf-8 -*-
"""
Created on Thu Dec  3 11:11:48 2020

@author: yaoji
"""

import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSignal, Qt, QMetaObject, QSettings
from Setting import mySetting
from TipApproach import myTipApproach
from Etest import myEtest
from MainMenu import myMainMenu
import conversion as cnv
import threading

class myEtestControl(myMainMenu):
    
    def etest_ramp():
        pass
    
    def etest_ramp_start(self):
        # lambda
        threading.Thread(self.etest_ramp).start()

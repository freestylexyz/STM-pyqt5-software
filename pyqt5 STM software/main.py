# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:46:49
@Author   : milier00
@FileName : main.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication
from MainMenu import myMainMenu



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myMainMenu()
    window.show()
    sys.exit(app.exec_())


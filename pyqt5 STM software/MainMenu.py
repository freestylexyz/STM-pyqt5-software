import logging
import sys
sys.path.append("./ui/")
from PyQt5 import QtCore, QtGui, QtWidgets
# make the example runnable without the need to install
from os.path import abspath, dirname
sys.path.insert(0, abspath(dirname(abspath(__file__)) + '/..'))

import qdarkstyle
from MainMenu_ui import Ui_HoGroupSTM
from Setting import mySetting
from TipApproach_ui import Ui_TipApproach
from Etest_ui import Ui_ElectronicTest

class myMainMenu(QtWidgets.QMainWindow, Ui_HoGroupSTM):
    def __init__(self):
        super().__init__()
        self.setupUi(self)


        self.actionSetting.triggered['bool'].connect(self.opensettingwin)
        self.actionTipAppr.triggered['bool'].connect(self.opentipapprwin)
        self.actionEtest.triggered['bool'].connect(self.openelectronictest)
        QtCore.QMetaObject.connectSlotsByName(self)


        # call out sub windows from menu

        # Setting subwindow
        self.settingwidget = mySetting()
        # Tip approach subwindow
        self.tipapprwidget = QtWidgets.QWidget()
        self.tipapprwin = Ui_TipApproach()
        self.tipapprwin.setupUi(self.tipapprwidget)
        
        # Electronic test subwindow
        self.electrotestwidget = QtWidgets.QWidget()
        self.electrotestwin = Ui_ElectronicTest()
        self.electrotestwin.setupUi(self.electrotestwidget)

    # Open dialog from menubar
    def opensettingwin(self):
        self.settingwidget.show()
    def opentipapprwin(self):
        self.tipapprwidget.show()
    def openelectronictest(self):
        self.electrotestwidget.show()



def main():
    """
    Application entry point
    """
    logging.basicConfig(level=logging.DEBUG)
    # create the application and the main window
    app = QtWidgets.QApplication(sys.argv)
    window = myMainMenu()

    # setup stylesheet
    # app.setStyleSheet(qdarkstyle.load_stylesheet_pyqt5())

    # auto quit after 2s when testing on travis-ci
    if "--travis" in sys.argv:
        QtCore.QTimer.singleShot(2000, app.exit)

    # run
    window.show()
    app.exec_()


if __name__ == "__main__":
    main()


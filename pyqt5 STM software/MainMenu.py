import logging
import sys
from PyQt5 import QtWidgets, QtCore
# make the example runnable without the need to install
from os.path import abspath, dirname
sys.path.insert(0, abspath(dirname(abspath(__file__)) + '/..'))

import qdarkstyle
import MainMenu_ui as mainwin


def main():
    """
    Application entry point
    """
    logging.basicConfig(level=logging.DEBUG)
    # create the application and the main window
    app = QtWidgets.QApplication(sys.argv)
    window = QtWidgets.QMainWindow()

    # setup ui
    ui = mainwin.Ui_HoGroupSTM()
    ui.setupUi(window)


    # ui.bt_delay_popup.addActions([
    #     ui.actionAction,
    #     ui.actionAction_C
    # ])
    # ui.bt_instant_popup.addActions([
    #     ui.actionAction,
    #     ui.actionAction_C
    # ])
    # ui.bt_menu_button_popup.addActions([
    #     ui.actionAction,
    #     ui.actionAction_C
    # ])
    #item = QtWidgets.QTableWidgetItem("Test")
    #item.setCheckState(QtCore.Qt.Checked)
    #ui.tableWidget.setItem(0, 0, item)
    #window.setWindowTitle("QDarkStyle example")

    # tabify dock widgets to show bug #6
   # window.tabifyDockWidget(ui.Bias, ui.Current,ui.Zcontrol)

    # setup stylesheet
#    app.setStyleSheet(qdarkstyle.load_stylesheet_pyqt5())

    # auto quit after 2s when testing on travis-ci
    if "--travis" in sys.argv:
        QtCore.QTimer.singleShot(2000, app.exit)

    # run
    window.show()
    app.exec_()


if __name__ == "__main__":
    main()

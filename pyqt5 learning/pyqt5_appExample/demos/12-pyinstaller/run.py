# encoding=utf-8

import sys, os
if hasattr(sys, 'frozen'):
    os.environ['PATH'] = sys._MEIPASS + ";" + os.environ['PATH']

import PyQt5.QtWidgets as qw
import ui_test

if __name__ == "__main__":
    app = qw.QApplication(sys.argv)
    w = qw.QMainWindow()
    print("runing......")
    ui = ui_test.Ui_MainWindow()
    ui.setupUi(w)
    w.show()

    sys.exit(app.exec_())


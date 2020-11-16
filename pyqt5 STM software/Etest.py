import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication , QWidget
from PyQt5.QtCore import pyqtSignal , Qt
from Etest_ui import Ui_ElectronicTest



class myEtest(QWidget, Ui_ElectronicTest):

    def __init__(self):
        super().__init__()
        self.setupUi(self)

    def init_UI(self):
        pass



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myEtest()
    window.show()
    sys.exit(app.exec_())
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication
from MainMenu import myMainMenu



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myMainMenu()
    window.show()
    sys.exit(app.exec_())


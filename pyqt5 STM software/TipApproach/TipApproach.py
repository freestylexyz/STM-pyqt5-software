# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:49:07
@Author   : milier00
@FileName : TipApproach.py
"""
import sys
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication , QWidget , QDesktopWidget, QMessageBox
from PyQt5.QtCore import pyqtSignal
from TipApproach_ui import Ui_TipApproach
import conversion as cnv



class myTipApproach(QWidget, Ui_TipApproach):
    close_signal = pyqtSignal()
    stop_signal = pyqtSignal()
    mode_signal = pyqtSignal(int, bool)
    giant_signal = pyqtSignal(int, int, int, int, int, int)
    approach_signal = pyqtSignal(int, int, int, int, int, int, int)

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()
        self.idling = True

    # Init UI for tip approach
    def init_UI(self):
        # Set window geometry
        screen = QDesktopWidget().screenGeometry()
        size = self.frameGeometry()
        self.move(int((screen.width()-size.width())/2), int((screen.height()-size.height())/2))
        self.setFixedSize(self.width(), self.height())
        
        #
        # Connect signals
        #
        # Waveform
        self.spinBox_Delay.editingFinished.connect(lambda: self.delay_slot(True))
        self.scrollBar_Delay.valueChanged.connect(lambda: self.delay_slot(False))
        self.spinBox_Xstep.editingFinished.connect(lambda: self.x_slot(True))
        self.scrollBar_Xstep.valueChanged.connect(lambda: self.x_slot(False))
        self.spinBox_Zstep.editingFinished.connect(lambda: self.z_slot(True))
        self.scrollBar_Zstep.valueChanged.connect(lambda: self.z_slot(False))
        self.spinBox_Accel.editingFinished.connect(lambda: self.accel_slot(True))
        self.scrollBar_Accel.valueChanged.connect(lambda: self.accel_slot(False))
        self.spinBox_MInCurr.editingFinished.connect(lambda: self.min_curr_slot(True))
        self.scrollBar_MInCurr.valueChanged.connect(lambda: self.min_curr_slot(False))
        # Approach parameters
        self.spinBox_Giant.valueChanged.connect(self.scrollBar_Giant.setValue)
        self.scrollBar_Giant.valueChanged.connect(self.spinBox_Giant.setValue)
        self.spinBox_Baby.valueChanged.connect(self.scrollBar_Baby.setValue)
        self.scrollBar_Baby.valueChanged.connect(self.spinBox_Baby.setValue)
        self.spinBox_StepNum.valueChanged.connect(self.scrollBar_StepNum.setValue)
        self.scrollBar_StepNum.valueChanged.connect(self.spinBox_StepNum.setValue)
        # Translation
        self.groupBox_Trans_TipAppr.toggled.connect(self.mode_change)
        self.pushButton_Xfwd_Trans.clicked.connect(lambda: self.giant_step(0x10, True))
        self.pushButton_Xbkwd_Trans.clicked.connect(lambda: self.giant_step(0x10, False))
        self.pushButton_Yfwd_Trans.clicked.connect(lambda: self.giant_step(0x1f, True))
        self.pushButton_Ybkwd_Trans.clicked.connect(lambda: self.giant_step(0x1f, False))
        # Rotation
        self.pushButton_TipAppr_Steps.clicked.connect(self.tip_approach)
        self.pushButton_Down_Steps.clicked.connect(lambda: self.giant_step(0x10, False))
        self.pushButton_Up_Steps.clicked.connect(lambda: self.giant_step(0x10, True))
        # Stop
        self.pushButton_Stop.clicked.connect(self.stop)

        
    # Re-init tip approach view
    def init_tipAppr(self, succeed, lastdigital):
        # Enable serial related buttons
        self.groupBox_Trans_TipAppr.setEnabled(succeed)
        self.pushButton_Up_Steps.setEnabled(succeed)
        self.pushButton_Down_Steps.setEnabled(succeed)
        self.pushButton_TipAppr_Steps.setEnabled(succeed)
        
        # Set push buttons enable based on dsp translation status
        self.groupBox_Trans_TipAppr.setChecked(lastdigital[5])
        self.set_translation(lastdigital[5])
        
    # Set translation based on variable
    def set_translation(self, translation):
        self.pushButton_Up_Steps.setEnabled(not translation)
        self.pushButton_Down_Steps.setEnabled(not translation)
        self.pushButton_TipAppr_Steps.setEnabled(not translation)
    
    # Delay conversion slot
    def delay_slot(self, box_to_bar):
        if box_to_bar:
            self.scrollBar_Delay.setValue(int(self.spinBox_Delay.value() * 1000))
        else:
            self.spinBox_Delay.setValue(int(self.scrollBar_Delay.value() / 1000))
            
    # X step conversion slot
    def x_slot(self, box_to_bar):
        if box_to_bar:
            self.scrollBar_Xstep.setValue(cnv.vb(self.spinBox_Xstep.value(), 'd', 10) - 0x8000)
        else:
            self.spinBox_Xstep.setValue(cnv.bv(self.scrollBar_Xstep.value() + 0x8000, 'd', 10))
            
    # Z step conversion slot
    def z_slot(self, box_to_bar):
        if box_to_bar:
            self.scrollBar_Zstep.setValue(cnv.vb(self.spinBox_Zstep.value(), 'd', 10) - 0x8000)
        else:
            self.spinBox_Zstep.setValue(cnv.bv(self.scrollBar_Zstep.value() + 0x8000, 'd', 10))
            
    # Acceleration conversion slot
    def accel_slot(self, box_to_bar):
        if box_to_bar:
            self.scrollBar_Accel.setValue(int(self.spinBox_Accel.value() * 8.163))
        else:
            self.spinBox_Accel.setValue(self.scrollBar_Accel.value() / 8.163)
            
    # Minimum tunneling current conversion slot
    def min_curr_slot(self, box_to_bar):
        if box_to_bar:
            self.scrollBar_MInCurr.setValue(cnv.vb(self.spinBox_MInCurr.value(), 'a') - 0x8000)
        else:
            self.spinBox_MInCurr.setValue(cnv.bv(self.scrollBar_MInCurr.value() + 0x8000, 'a'))
    
    # Giant step slot
    def giant_step(self, channel, direction):
        # Obtain all required information
        x = self.scrollBar_Xstep.value()
        z = self.scrollBar_Zstep.value() + 0x8000
        delay = self.scrollBar_Delay.value()
        g = self.scrollBar_Accel.value()
        stepnum = self.scrollBar_StepNum.value()
        
        x = (0x8000 + x) if direction else (0x8000 - x )            # Calculate the real x based on direction
        chtxt = 'X' if channel == 0x10 else 'Y'                     # Figure out channel text
        dirtxt = '+' if direction else '-'                          # Figure out direction text
        labeltxt = 'Stepping Down' if direction else 'Stepping Up'  # Figure out steping direction text

        # Set status label
        if self.groupBox_Trans_TipAppr.isChecked():
            self.label_Status_Status.setText('Translating to' + ' ' + dirtxt + chtxt +' direction')
        else:
            self.label_Status_Status.setText(labeltxt)
        
        # Emit signal for giant steps
        self.giant_signal.emit(channel, x, z, delay, g, stepnum)
        
    # Tip approach slot
    def tip_approach(self):
        # Obtain all required information
        x = 0x8000 - self.scrollBar_Xstep.value() 
        z = self.scrollBar_Zstep.value() + 0x8000
        delay = self.scrollBar_Delay.value()
        g = self.scrollBar_Accel.value()
        giant = self.scrollBar_Giant.value()
        baby = self.scrollBar_Baby.value()
        minCurr = self.scrollBar_MInCurr.value()

        self.label_Status_Status.setText('Tip approaching')     # Set status label
        
        # Emit signal for tip approach
        self.approach_signal.emit(x, z, delay, g, giant, baby, minCurr)
    
    # Change translation mode
    def mode_change(self, translation):
        # Set push buttons enable based on dsp translation status
        self.set_translation(translation)
        # Emit signal to change dsp translation mode
        self.mode_signal.emit(5, translation)
    
    # Stop button
    def stop(self):
        self.pushButton_Stop.setEnabled(False)      # Disable Stop button to prevent emitting signal twice
        self.stop_signal.emit()                     # Emit stop signal
    
    # Emit close signal
    def closeEvent(self, event):
        if self.idling:
            self.close_signal.emit()    # Emit close signal
            event.accept()              # Accept close signal
        else:
            QMessageBox.warning(None, "Tip approach", "Operation is on going", QMessageBox.Ok)
            event.ignore()
    
    # Enable serial
    def enable_serial(self, enable):
        self.groupBox_WaveForm_TipAppr.setEnabled(enable)
        self.groupBox_ApprPar_TipAppr.setEnabled(enable)
        self.groupBox_Steps_TipAppr.setEnabled(enable)
        self.groupBox_Trans_TipAppr.setEnabled(enable)
        self.groupBox_MInCurr_TipAppr.setEnabled(enable)
        self.pushButton_Stop.setEnabled(False)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myTipApproach()
    window.enable_serial(True)
    window.show()
    sys.exit(app.exec_())
# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:49:07
@Author   : milier00
@FileName : TipApproach.py
"""
import sys
sys.path.append("./ui/")
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
        # Init value for all input
        #
        # Waveform
        self.spinBox_Delay.setValue(10)
        self.scrollBar_Delay.setValue(10000)
        self.spinBox_Accel.setValue(5)
        self.scrollBar_Accel.setValue(5)
        self.spinBox_Xstep.setValue(cnv.bv(0xc000, 'd', 10))
        self.scrollBar_Xstep.setValue(0xc000)
        self.spinBox_Zstep.setValue(cnv.bv(0xc000, 'd', 10))
        self.scrollBar_Zstep.setValue(0xc000)
        # Approach parameters
        self.scrollBar_Giant.setValue(5)
        self.spinBox_Giant.setValue(5)
        self.spinBox_Baby.setValue(2)
        self.scrollBar_Baby.setValue(2)
        # Minimum tunneling current
        self.spinBox_MInCurr.setValue(cnv.bv(0x8140, 'a'))
        self.scrollBar_MInCurr.setValue(0x0140)
        # Step number
        self.spinBox_StepNum.setValue(1000)
        self.scrollBar_StepNum.setValue(1000)
        # Translation
        self.groupBox_Trans_TipAppr.setChecked(False)
        
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
        self.pushButton_Down_Steps.clicked.connect(lambda: self.giant_step(0x10, True))
        self.pushButton_Up_Steps.clicked.connect(lambda: self.giant_step(0x10, False))
        # Stop
        self.pushButton_Stop.clicked.connect(self.stop)


        
    # Re-init tip approach view
    def init_tipAppr(self, succeed, lastdigital):
        self.enable_serial(succeed)                                     # Enable serial related button based on DSP initial succeed
        
        # Set push buttons enable based on dsp translation status
        self.groupBox_Trans_TipAppr.setChecked(lastdigital[5])
        self.pushButton_Up_Steps.setEnabled(not lastdigital[5])
        self.pushButton_Down_Steps.setEnabled(not lastdigital[5])
        self.pushButton_TipAppr_Steps.setEnabled(not lastdigital[5])
    
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
            self.scrollBar_Accel.setValue(int(self.spinBox_Accel.value() * 100))
        else:
            self.spinBox_Accel.setValue(self.scrollBar_Accel.value() / 100)
            
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
        z = self.scrollBar_Zstep.value()
        delay = self.scrollBar_Delay.value()
        g = self.scrollBar_Accel.value()
        stepnum = self.scrollBar_StepNum.value()
        
        # Calculate the real x based on direction
        if direction:
            x = 0x8000 + x
        else:
            x = 0x8000 - x 
        
        # Figure out channel text
        chtxt = ' '
        if channel == 0x10:
            chtxt = 'X'
        elif channel == 0x1f:
            chtxt = 'Y'
            
        # Figure out direction text
        dirtxt = ' '
        if direction:
            dirtxt = '+'
        else:
            dirtxt = '-'
            
        # Set status label
        if self.groupBox_Trans_TipAppr.isChecked():
            self.label_Status.setText('Translating to' + ' ' + dirtxt + chtxt +' direction')
        else:
            if direction:
                self.label_Status.setText('Stepping Down')
            else:
                self.label_Status.setText('Stepping Up')
        
        # Emit signal for giant steps
        self.giant_signal.emit(channel, x, z, delay, g, stepnum)
        
    # Tip approach slot
    def tip_approach(self):
        # Obtain all required information
        x = self.scrollBar_Xstep.value()
        z = self.scrollBar_Zstep.value()
        delay = self.scrollBar_Delay.value()
        g = self.scrollBar_Accel.value()
        giant = self.scrollBar_Giant.value()
        baby = self.scrollBar_Baby.value()
        minCurr = self.scrollBar_MInCurr.value()
        
        x = 0x8000 + x                                  # Calculate the real x
        self.label_Status.setText('Tip approaching')    # Set status label
        
        # Emit signal for tip approach
        self.approach_signal = pyqtSignal(x, z, delay, g, giant, baby, minCurr)
    
    # Change translation mode
    def mode_change(self, translation):
        # Set push buttons enable based on dsp translation status
        self.pushButton_Up_Steps.setEnabled(not translation)
        self.pushButton_Down_Steps.setEnabled(not translation)
        self.pushButton_TipAppr_Steps.setEnabled(not translation)
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
        self.pushButton_Up_Steps.setEnabled(enable)
        self.pushButton_Down_Steps.setEnabled(enable)
        self.pushButton_TipAppr_Steps.setEnabled(enable)
        self.groupBox_Trans_TipAppr.setEnabled(enable)
        self.pushButton_Stop.setEnabled(False)



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myTipApproach()
    window.enable_serial(True)
    window.show()
    sys.exit(app.exec_())
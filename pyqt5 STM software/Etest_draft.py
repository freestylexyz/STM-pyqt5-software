# -*- coding: utf-8 -*-
"""
@Date     : 2020/11/16 17:46:12
@Author   : milier00
@FileName : Etest.py
"""
import sys
sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication , QWidget, QDesktopWidget, QMessageBox
from PyQt5.QtCore import pyqtSignal , Qt
from Etest_ui import Ui_ElectronicTest


class myEtest(QWidget, Ui_ElectronicTest):

    close_signal = pyqtSignal()
    adc_range_signal = pyqtSignal(int)
    adc_ch_signal =  pyqtSignal(int)
    adc_input_signal = pyqtSignal(int)
    dac_send_signal = pyqtSignal(int,int,int,int)
    bit20_dac_signal = pyqtSignal(int)
    rtest_ramp_signal = pyqtSignal(int, int, int, int, int)
    swave_start_signal = pyqtSignal(int, int, int, int)
    ftest_start_signal = pyqtSignal(int, int, int, int, int, int)

    def __init__(self):
        super().__init__()
        self.idling = True
        self.setupUi(self)
        self.init_UI()


    def init_UI(self):
        # init ui position and size
        screen = QDesktopWidget().screenGeometry()
        self.resize(780, 540)
        size = self.frameGeometry()
        self.move(int((screen.width()-size.width())/2), int((screen.height()-size.height())/2))
        self.setFixedSize(self.width(), self.height())

        # adc
        self.adc_in.clicked.connect(self.adc_input_slot)

        # dac
        self.dac_bit.valueChanged.connect(self.dac_val.setValue)
        self.dac_val.valueChanged.connect(self.dac_bit.setValue)
        self.dac_out.clicked.connect(self.dac_output_slot)

        # 20 bit dac
        self.bit20_bit.valueChanged.connect(self.bit20_val.setValue)
        self.bit20_val.valueChanged.connect(self.bit20_bit.setValue)
        self.bit20_out.clicked.connect(self.bit20_output_slot)

        # ramp test
        self.pushButton_Ramp_RTest.clicked.connect(self.rtest_ramp_slot)
        self.pushButton_RRread_RTest.clicked.connect(self.rtest_rread_slot)

        # square wave
        self.pushButton_Start_SWave.clicked.connect(self.swave_start_slot)

        # oscilloscope test
        self.pushButton_SorS_Osci.clicked.connect(self.osci_start_slot)

        # echo
        self.pushButton_Query_Echo.clicked.connect(self.echo_query_slot)
        self.pushButton_Start_Echo.clicked.connect(self.echo_start_slot)

        # feedback test
        self.pushButton_Start_FTest.clicked.connect(self.ftest_start_slot)

    # adc
    def get_adc_ch(self):
        self.adc_ch.currentIndexChanged().connect(self.adc_ch_slot)

    def adc_ch_slot(self,in_ch):
        self.adc_ch_signal.emit(in_ch)
        
    def get_adc_range(self):
        self.adc_b20.toggled.connect(lambda: self.adc_range_slot(0))
        self.adc_b10.toggled.connect(lambda: self.adc_range_slot(1))
        self.adc_b5.toggled.connect(lambda: self.adc_range_slot(2))
        self.adc_u10.toggled.connect(lambda: self.adc_range_slot(5))
        self.adc_u5.toggled.connect(lambda: self.adc_range_slot(6))

    def adc_range_slot(self,range):
        self.adc_range_signal.emit(range)

    def adc_input_slot(self):
        self.adc_input_signal.emit()

    # dac
    def dac_output_slot(self):
        out_ch = self.dac_ch.currentIndex()
        out_range = self.get_dac_range()
        out_val = self.get_dac_val()
        out_iset_val = self.dac_Iset_val.value()
        self.dac_send_signal.emit(out_ch, out_range, out_val, out_iset_val)

    def get_dac_val(self,int):
        bits = self.dac_bit.value()  # Obtain target bits
        bits = bits + 0x8000  # Add view offset
        return bits

    def get_dac_range(self):
        if self.dac_b20.isChecked():
            range = 10
        elif self.dac_b10.isChecked():
            range = 9
        elif self.dac_b5.isChecked():
            range = 14
        elif self.dac_b10_4.isChecked():
            range = 1
        elif self.dac_u5.isChecked():
            range = 0
        return range


    # bit 20 dac
    def bit20_output_slot(self):
        out_val = self.bit20_bit.value()
        self.bit20_dac_signal.emit(out_val)

    # Emit close signal
    def closeEvent(self, event):
        if self.idling:
            self.close_signal.emit()
            event.accept()
        else:
            # !!! pop window, ongoing
            event.ignore()

    # Enable serial
    def enable_serial(self, enable):
        # I/O
        # DAC
        self.dac_out.setEnabled(enable)
        self.dac_u5.setEnabled(enable)
        self.dac_b5.setEnabled(enable)
        self.dac_b20.setEnabled(enable)
        self.dac_b10_4.setEnabled(enable)
        self.dac_b10.setEnabled(enable)
        self.dac_0.setEnabled(enable)
        # ADC
        self.adc_b10.setEnabled(enable)
        self.adc_b20.setEnabled(enable)
        self.adc_b5.setEnabled(enable)
        self.adc_u10.setEnabled(enable)
        self.adc_u5.setEnabled(enable)
        self.adc_in.setEnabled(enable)
        # 20 bit
        self.bit20_0.setEnabled(enable)
        self.bit20_out.setEnabled(enable)
        # Dither 0
        self.dither0_on.setEnabled(enable)
        self.dither0_off.setEnabled(enable)
        # Dither 1
        self.dither1_on.setEnabled(enable)
        self.dither1_off.setEnabled(enable)
        # Coarse
        self.coarse.setEnabled(enable)
        self.fine.setEnabled(enable)
        # Feedback
        self.feedback_on.setEnabled(enable)
        self.feedback_off.setEnabled(enable)
        # Retract
        self.retract_on.setEnabled(enable)
        self.retract_off.setEnabled(enable)
        # Rotation
        self.rotation.setEnabled(enable)
        self.translation.setEnabled(enable)
        # X gain
        self.xgain0_1.setEnabled(enable)
        self.xgain1.setEnabled(enable)
        self.xgain10.setEnabled(enable)
        # Y gain
        self.ygain0_1.setEnabled(enable)
        self.ygain1.setEnabled(enable)
        self.ygain10.setEnabled(enable)
        # Z1 gain
        self.z1gain0_1.setEnabled(enable)
        self.z1gain1.setEnabled(enable)
        self.z1gain10.setEnabled(enable)
        # Z2 gain
        self.z2gain0_1.setEnabled(enable)
        self.z2gain1.setEnabled(enable)
        self.z2gain10.setEnabled(enable)

        # Ramp test
        self.pushButton_Ramp_RTest.setEnabled(enable)
        self.pushButton_RRead_RTest.setEnabled(enable)

        # Square wave
        self.pushButton_Start_SWave.setEnabled(enable)

        # Oscilloscope
        self.pushButton_SorS_Osci.setEnabled(enable)

        # Echo
        self.pushButton_Query_Echo.setEnabled(enable)
        self.pushButton_Start_Echo.setEnabled(enable)

        # Feedback test
        self.pushButton_Start_FTest.setEnabled(enable)
        self.radioButton_ON_Fdbk.setEnabled(enable)
        self.radioButton_OFF_Fdbk.setEnabled(enable)
        self.radioButton_ON_Retr.setEnabled(enable)
        self.radioButton_OFF_Retr.setEnabled(enable)

    # !!! need to convert values/indexes and set idling flag
    def rtest_ramp_slot(self):
        in_ch = self.comboBox_InCh_RTest.currentIndex()
        out_ch = self.comboBox_OutCh_RTest.currentIndex()
        init_val = self.spinBox_InitVal_RTest.value()
        fin_val = self.spinBox_FinVal_RTest.value()
        step_val = self.spinBox_StepSize_RTest.value()
        self.rtest_ramp_signal.emit(in_ch, out_ch, init_val, fin_val, step_val)
        self.pushButton_Ramp_RTest.setEnabled(False)
        self.punshButton_RRead_RTest.setEnabled(True)

    def rtest_rread_slot(self):
        pass

    # !!! need to convert values/indexes and set idling flag
    def swave_start_slot(self):
        in_ch = self.comboBox_Ch_SWave.currentIndex()
        delay_val = self.spinBox_Delay_SWave.value()
        v1_val = self.spinBox_V1_SWave.value()
        v2_val = self.spinBox_V2_SWave.value()
        self.swave_start_signal.emit(in_ch, delay_val, v1_val, v2_val)
        # self.idling = False
        self.pushButton_Start_SWave.setText("Stop")

    def osci_start_slot(self):
        pass

    def echo_query_slot(self):

        pass
    def echo_start_slot(self):
        pass

    # !!! need to convert values/indexes and set idling flag
    def ftest_start_slot(self):
        a_val = self.spinBox_AInput_FTest.value()
        z0_val = self.spinBox_Z0Input_FTest.value()
        delay_val = self.spinBox_delay_FTest.value()
        ispt_val = self.spinBox_IsptInput_FTest.value()
        out_ch = self.comboBox_OutCh_FTest.currentIndex()
        in_ch = self.comboBox_InCh_FTest.currentIndex()
        self.ftest_start_signal.emit(a_val, z0_val, delay_val, ispt_val, out_ch, in_ch)
        self.pushButton_Start_FTest.serEnabled(False)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myEtest()
    window.show()
    sys.exit(app.exec_())
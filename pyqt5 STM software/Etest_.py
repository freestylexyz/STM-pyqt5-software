# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/10 01:52:03
@Author   : milier00
@FileName : Etest_.py
"""
import sys

sys.path.append("./ui/")
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox,QButtonGroup
from PyQt5.QtCore import pyqtSignal, Qt
from Etest_ui import Ui_ElectronicTest
import conversion as cnv
import functools as ft

class myEtest(QWidget, Ui_ElectronicTest):
    close_signal = pyqtSignal()
    # I/O signals
    range_changed_signal = pyqtSignal(int, int)
    ch_changed_signal = pyqtSignal(int, int)
    digital_changed_signal = pyqtSignal(int, int)
    gain_changed_signal = pyqtSignal(int, int)
    adc_input_signal = pyqtSignal()
    dac_output_signal = pyqtSignal(int, int)
    bit20_output_signal = pyqtSignal(int)

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

        # adc | pushButton
        self.adc_in.clicked.connect(self.adc_input_signal)

        # adc | radioButton
        self.adc_range_group = QButtonGroup()
        self.adc_range_group.addButton(self.adc_b20,0)
        self.adc_range_group.addButton(self.adc_b10, 1)
        self.adc_range_group.addButton(self.adc_b5, 2)
        self.adc_range_group.addButton(self.adc_u10, 5)
        self.adc_range_group.addButton(self.adc_u5, 6)
        self.adc_range_group.buttonToggled[int,bool].connect(ft.partial(self.range_changed_emit,0))

        # adc | comboBox
        self.adc_ch.currentIndexChanged.connect(lambda : self.ch_changed_emit(0))

        # dac | spinBox and scrollBar
        self.dac_bit.valueChanged.connect(self.dac_val.setValue)
        self.dac_val.valueChanged.connect(self.dac_bit.setValue)

        # dac | pushButton
        self.dac_out.clicked.connect(lambda: self.dac_output_emit(1))    # output button clicked
        self.dac_0.clicked.connect(lambda: self.dac_output_emit(0))      # zero button clicked
        self.dac_bit.setMaximum(0xffff)

        # dac | radioButton
        self.dac_range_group = QButtonGroup()
        self.dac_range_group.addButton(self.dac_u5, 0)
        self.dac_range_group.addButton(self.dac_b10_4, 1)
        self.dac_range_group.addButton(self.dac_b10, 9)
        self.dac_range_group.addButton(self.dac_b20, 10)
        self.dac_range_group.addButton(self.dac_b5, 14)
        self.dac_range_group.buttonToggled[int, bool].connect(ft.partial(self.range_changed_emit, 1))

        # dac | comboBox
        self.dac_ch.currentIndexChanged.connect(lambda : self.ch_changed_emit(1))

        # 20 bit dac | spinBox and scrollBar
        self.bit20_bit.valueChanged.connect(self.bit20_val.setValue)
        self.bit20_val.valueChanged.connect(self.bit20_bit.setValue)
        self.bit20_val.setMinimum(cnv.bv(0, '20'))
        self.bit20_val.setMaximum(cnv.bv(0xfffff, '20'))
        self.bit20_bit.setMaximum(0xfffff)

        # 20 bit dac | pushButton
        self.bit20_out.clicked.connect(lambda: self.bit20_output_emit(1))    # output button clicked
        self.bit20_0.clicked.connect(lambda: self.bit20_output_emit(0))      # zero button clicked

        # last digital | radioButton
        self.dither0_on.toggled.connect(lambda: self.digital_changed_emit(0))
        self.dither1_on.toggled.connect(lambda: self.digital_changed_emit(1))
        self.feedback_on.toggled.connect(lambda: self.digital_changed_emit(2))
        self.retract_on.toggled.connect(lambda: self.digital_changed_emit(3))
        self.coarse.toggled.connect(lambda: self.digital_changed_emit(4))
        self.translation.toggled.connect(lambda: self.digital_changed_emit(5))

        # last gain | radioButton
        self.x_gain_group = QButtonGroup()
        self.x_gain_group.addButton(self.xgain10, 0)
        self.x_gain_group.addButton(self.xgain1, 1)
        self.x_gain_group.addButton(self.xgain0_1, 3)
        self.x_gain_group.buttonToggled[int, bool].connect(ft.partial(self.gain_changed_emit, 0))
        self.y_gain_group = QButtonGroup()
        self.y_gain_group.addButton(self.ygain10, 0)
        self.y_gain_group.addButton(self.ygain1, 1)
        self.y_gain_group.addButton(self.ygain0_1, 3)
        self.y_gain_group.buttonToggled[int, bool].connect(ft.partial(self.gain_changed_emit, 1))
        self.z1_gain_group = QButtonGroup()
        self.z1_gain_group.addButton(self.z1gain0_1, 0)
        self.z1_gain_group.addButton(self.z1gain1, 1)
        self.z1_gain_group.addButton(self.z1gain10, 3)
        self.z1_gain_group.buttonToggled[int, bool].connect(ft.partial(self.gain_changed_emit, 2))
        self.z2_gain_group = QButtonGroup()
        self.z2_gain_group.addButton(self.z2gain10, 0)
        self.z2_gain_group.addButton(self.z2gain1, 1)
        self.z2_gain_group.addButton(self.z2gain0_1, 3)
        self.z2_gain_group.buttonToggled[int, bool].connect(ft.partial(self.gain_changed_emit, 3))

        # # ramp test
        # self.pushButton_Ramp_RTest.clicked.connect(self.rtest_ramp_slot)
        # self.pushButton_RRread_RTest.clicked.connect(self.rtest_rread_slot)
        #
        # # square wave
        # self.pushButton_Start_SWave.clicked.connect(self.swave_start_slot)
        #
        # # oscilloscope test
        # self.pushButton_SorS_Osci.clicked.connect(self.osci_start_slot)
        #
        # # echo
        # self.pushButton_Query_Echo.clicked.connect(self.echo_query_slot)
        # self.pushButton_Start_Echo.clicked.connect(self.echo_start_slot)
        #
        # # feedback test
        # self.pushButton_Start_FTest.clicked.connect(self.ftest_start_slot)

    def init_etest(self, succeed, dacrange, adcrange, lastdigital, lastgain):
        # Set up view in case of successfully finding DSP
        self.enable_serial(succeed)
        '''
        self.lastdac = [0x8000] * 16    # Last ouput of all DAC channels
        self.dacrange = [10] * 16       # All DAC channels' current range
        self.adcrange = [0] * 8         # All ADC channels' current range
        self.last20bit = 0x80000        # Last ouput of 20bit DAC
        self.lastdigital = [False] * 6  # 0 - 5 : bias dither, z dither, feedback, retract, coarse, translation
        self.lastgain = [1] * 4         # 0 -> gain 10.0, 1 -> gain 1.0, 3 -> gain 0.1
                                        # Z1 gain is different from others, 3 -> gain 10.0, 1 -> gain 1.0, 0 -> gain 0.1
        self.offset = [0] * 16          # 0 - 14 are bias offset for different range, 15 is Iset offset
        '''
        # load digital status from dsp
        self.dither0_on.setChecked(lastdigital[0])
        self.dither0_off.setChecked(not lastdigital[0])
        self.dither1_on.setChecked(lastdigital[1])
        self.dither1_off.setChecked(not lastdigital[1])
        self.feedback_on.setChecked(lastdigital[2])
        self.feedback_off.setChecked(not lastdigital[2])
        self.retract_on.setChecked(lastdigital[3])
        self.retract_off.setChecked(not lastdigital[3])
        self.coarse.setChecked(lastdigital[4])
        self.fine.setChecked(not lastdigital[4])
        self.rotation.serChecked(not lastdigital[5])
        self.translation.setCheked(lastdigital[5])
        self.load_x_gain(lastgain)
        self.load_y_gain(lastgain)
        self.load_z1_gain(lastgain)
        self.load_z2_gain(lastgain)

        # load adc range from dsp
        adc_ch = self.adc_ch.currentIndex()
        adc_ran = adcrange[adc_ch]
        self.load_range(0, adc_ran)

        # load dac range from dsp
        dac_ch = self.dac_ch.currentIndex()
        dac_ran = dacrange[dac_ch]
        self.load_range(1, dac_ran)

            
    # load adc(index = 0) or dac(index = 1) range from dsp
    def load_range(self, index, range):
        if index == 0:      # adc 
            if range == 0:
                self.adc_b20.setChecked(True)
            elif range == 1:
                self.adc_b10.setChecked(True)
            elif range == 2:
                self.adc_b5.setChecked(True)
            elif range == 5:
                self.adc_u10.setChecked(True)
            elif range == 6:
                self.adc_u5.setChecked(True)
        elif index == 1:    # dac
            if range == 0:
                self.dac_u5.setChecked(True)
            elif range == 1:
                self.dac_b10_4.setChecked(True)
            elif range == 9:
                self.dac_b10.setChecked(True)
            elif range == 10:
                self.dac_u10.setChecked(True)
            elif range == 14:
                self.dac_b5.setChecked(True)
            self.set_dac_spinBox_range(range)

    # set dac spinBox range
    def set_dac_spinBox_range(self, range):
        minimum = cnv.bv(0, 'd', range)
        maximum = cnv.bv(0xffff, 'd', range)
        self.dac_val.setMinimum(minimum)
        self.dac_val.setMaximum(maximum)

    # when adc(index = 0) or dac(index = 1) range changed by user, send signal
    def range_changed_emit(self, index, range, status):
        if status:              # only receive signal from checked button
            self.range_changed_signal.emit(index, range)
        else:
            pass

    # when adc(index = 0) or dac(index = 1) channel changed by user, send signal
    def ch_changed_emit(self, index, ch):
        self.ch_changed_signal.emit(index, ch)

    # when digital changed by user, send signal
    def digital_changed_emit(self, ch, status):
        self.digital_changed_signal.emit(ch, status)

    # when gain changed by user, send signal
    def gain_changed_emit(self, ch, val, status):
        if status:  # only receive signal from checked button
            self.gain_cahnged_signal.emit(ch, val)
        else:
            pass

    # dac output/zero button clicked, send output signal
    def dac_output_emit(self, index):
        if index == 0:          # zero button clicked
            bits = 0x8000;
            self.dac_bit.setValue(ox8000)
        else:
            bits = self.dac_bit.value() + 0x8000
        self.dac_output_signal.emit(bits)

    # 20 bit dac output/zero button clicked, send output signal
    def bit20_output_emit(self, index):
        if index == 0:  # zero button clicked
            bits = 0x80000;
            self.dac_bit.setValue(ox80000)
        else:
            bits = self.bit20_bit.value()
        self.bit20_output_signal.emit(bits)

    # load x gain radio button status from dsp
    def load_x_gain(self,lastgain):
        if lastgain[0] == 0:
            self.xgain10.setChecked(True)
        elif lastgain[0] == 1:
            self.xgain1.setChecked(True)
        elif lastgain[0] == 3:
            self.xgain0_1.setChecked(True)

    # load y gain radio button status from dsp
    def load_y_gain(self,lastgain):
        if lastgain[1] == 0:
            self.ygain10.setChecked(True)
        elif lastgain[1] == 1:
            self.ygain1.setChecked(True)
        elif lastgain[1] == 3:
            self.ygain0_1.setChecked(True)

    # load z1 gain radio button status from dsp
    def load_z1_gain(self,lastgain):
        if lastgain[2] == 0:
            self.z1gain0_1.setChecked(True)
        elif lastgain[2] == 1:
            self.z1gain1.setChecked(True)
        elif lastgain[2] == 3:
            self.z1gain10.setChecked(True)

    # load z2 gain radio button status from dsp
    def load_z2_gain(self,lastgain):
        if lastgain[3] == 0:
            self.z2gain10.setChecked(True)
        elif lastgain[3] == 1:
            self.z2gain1.setChecked(True)
        elif lastgain[3] == 3:
            self.z2gain0_1.setChecked(True)

    # Emit close signal
    def closeEvent(self, event):
        if self.idling:
            self.close_signal.emit()
            event.accept()
        else:
            # !!! pop window, ongoing
            QMessageBox.warning(None,"Etest","Process ongoing!",QMessageBox.Ok)
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

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myEtest()
    window.show()
    sys.exit(app.exec_())

    '''
      in main control
    # self.range_changed_signal.connect(self.range_changed_slot)
    # self.ch_changed_signal.connect(self.ch_changed_slot)
    # self.digital_changed_signal.connect(self.digital_changed_slot)
    # self.gain_changed_signal.connect(self.gain_changed_slot)
    # self.adc_input_signal.connect(self.adc_input_slot)
    # self.dac_output_signal.connect(self.dac_output_slot)
    # self.bit20_output_signal.connect(self.bit20_output_slot)

    '''


# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/10 01:52:03
@Author   : milier00
@FileName : Etest_.py
"""
import sys

sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup
from PyQt5.QtCore import pyqtSignal, Qt
from PyQt5.QtGui import QIntValidator
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
from Etest_ui import Ui_ElectronicTest
import numpy as np
import conversion as cnv
import functools as ft
import math

class myEtest(QWidget, Ui_ElectronicTest):
    # Common signal
    close_signal = pyqtSignal()
    stop_signal = pyqtSignal()
    
    # I/O signals
    range_changed_signal = pyqtSignal(int, int)
    ch_changed_signal = pyqtSignal(int, int)
    digital_changed_signal = pyqtSignal(int, int)
    gain_changed_signal = pyqtSignal(int, int)
    adc_input_signal = pyqtSignal()
    dac_output_signal = pyqtSignal(int, int)
    bit20_output_signal = pyqtSignal(int, int)
    
    # Ramp Test signals
    rtest_ramp_signal = pyqtSignal(int, int, int, int, int, int)
    
    # Square Wave signals
    swave_start_signal = pyqtSignal(int, int, int, int)
    
    # Oscilloscope signals
    osci_start_signal = pyqtSignal(int, int, int, int, int)

    # Echo signals
    echo_start_signal = pyqtSignal()
    echo_query_signal = pyqtSignal(int)

    # Feedback Test
    ftest_stop_signal = pyqtSignal()
    ftest_start_signal = pyqtSignal(int)

    def __init__(self):
        super().__init__()
        self.idling = True
        self.mode = 0   # 0: I/O, 1: RampTest, 2: SquareWave, 3: Oscilloscope, 4: Echo, 5: FeedbackTest
        self.ftest_stop = False
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        # init ui position and size
        screen = QDesktopWidget().screenGeometry()
        self.resize(780, 540)
        size = self.frameGeometry()
        self.move(int((screen.width()-size.width())/2), int((screen.height()-size.height())/2))
        self.setFixedSize(self.width(), self.height())

        # define variables
        self.dac_range = [10,10,10]            # dac range variable: 0: I/O, 1: Ramp Test, 2: Square Wave
        self.range_text_dict = {2: {0: '-10.24V to +10.24V', 1: '-5.12 V to +5.12 V', \
                                    2: '-2.56 V to +2.56 V', 5: '0 to 10.24 V', \
                                    6: '0 to 5.12 V'}, \
                                3: {0: '0 to 5V', 1: '0 to 10V', 2: '0 to 20V', 4: '0 to 40 V', \
                                    9: '-5 V to +5 V', 10: '-10 V to +10 V', 12: '-20 V to +20 V', \
                                    14: '-2.5 V to +2.5 V', 16: '-5V to +5V'}}    # input(2), output(3) range -> text dictionary
        self.rtest_ramp_read_outdata = [] * 100        # Ramp Test | ramp read output data
        self.rtest_ramp_read_indata = [] * 100         # Ramp Test | ramp read input data
        self.rtest_ramp_data = [] * 100                # Ramp Test | ramp data
        self.ptr2 = 0                          # Ramp Test | ramp plot update count
        self.ptr1 = 0                          # Oscilloscope | plot update count
        self.ptr3 = 0                          # Feedback Test | plot update count
        self.osci_nsample_data = []                     # Oscilloscope | N sample data
        self.osci_continuous_data = np.zeros(1)         # Oscilloscope | Continuous data
        self.ftest_zout_data = np.zeros(1)     # Feedback Test | feedback z data
        self.ftest_vout_data = np.zeros(1)     # Feedback Test | preamp voltage data
        self.ftest_parm = [0.1] * 3            # Feedback Test | parameters A, Z0, Delay


        # adc | pushButton
        self.adc_in.clicked.connect(self.adc_input_signal)

        # adc | radioButton
        self.adc_range_group = QButtonGroup()
        self.adc_range_group.addButton(self.adc_b20, 0)
        self.adc_range_group.addButton(self.adc_b10, 1)
        self.adc_range_group.addButton(self.adc_b5, 2)
        self.adc_range_group.addButton(self.adc_u10, 5)
        self.adc_range_group.addButton(self.adc_u5, 6)
        self.adc_range_group.buttonToggled[int, bool].connect(ft.partial(self.range_changed_emit, 0))

        # adc | comboBox
        self.adc_ch.currentIndexChanged.connect(ft.partial(self.ch_changed_emit, 0))

        # dac | spinBox and scrollBar
        self.dac_bit.valueChanged.connect(ft.partial(self.scroll2spin, 0))
        self.dac_val.editingFinished.connect(lambda: self.spin2scroll(0))

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
        self.dac_ch.currentIndexChanged.connect(ft.partial(self.ch_changed_emit, 1))

        # 20 bit dac | spinBox and scrollBar
        self.bit20_bit.valueChanged.connect(ft.partial(self.scroll2spin, 1))
        self.bit20_val.editingFinished.connect(lambda: self.spin2scroll(1))
        self.bit20_val.setMinimum(cnv.bv(0, '20'))
        self.bit20_val.setMaximum(cnv.bv(0xfffff, '20'))
        self.bit20_bit.setMaximum(0xfffff)

        # 20 bit dac | pushButton
        self.bit20_out.clicked.connect(lambda: self.bit20_output_emit(1))    # output button clicked
        self.bit20_0.clicked.connect(lambda: self.bit20_output_emit(0))      # zero button clicked

        # last digital | radioButton
        self.dither0_on.toggled.connect(ft.partial(self.digital_changed_emit, 0))
        self.dither1_on.toggled.connect(ft.partial(self.digital_changed_emit, 1))
        self.feedback_on.toggled.connect(ft.partial(self.digital_changed_emit, 2))
        self.retract_on.toggled.connect(ft.partial(self.digital_changed_emit, 3))
        self.coarse.toggled.connect(ft.partial(self.digital_changed_emit, 4))
        self.translation.toggled.connect(ft.partial(self.digital_changed_emit, 5))

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

        # Ramp Test | pushButton
        self.pushButton_Ramp_RTest.clicked.connect(lambda: self.rtest_ramp_emit(0))
        self.pushButton_RRead_RTest.clicked.connect(lambda: self.rtest_ramp_emit(1))

        # Ramp Test | comboBox
        self.comboBox_InCh_RTest.currentIndexChanged.connect(ft.partial(self.ch_changed_emit, 2))
        self.comboBox_OutCh_RTest.currentIndexChanged.connect(ft.partial(self.ch_changed_emit, 3))

        # Ramp Test | graphicsView
        self.rtest_ramp_plot = self.view_RTest.addPlot()
        self.rtest_ramp_plot.setDownsampling(mode='peak')
        self.rtest_ramp_plot.setClipToView(True)
        self.rtest_ramp_plot.addLegend()
        self.rtest_output_curve = self.rtest_ramp_plot.plot(pen='w', name='Output')
        self.rtest_input_curve = self.rtest_ramp_plot.plot(pen='y', name='Input')

        # Square Wave | pushButton
        self.pushButton_Start_SWave.clicked.connect(self.swave_start_emit)
        self.comboBox_Ch_SWave.currentIndexChanged.connect(ft.partial(self.ch_changed_emit, 4))

        # Oscilloscope | radioButton
        self.radioButton_NSample_Osci.toggled.connect(lambda: self.osci_set_delay_range(0))
        self.radioButton_Contin_Osci.toggled.connect(lambda: self.osci_set_delay_range(1))
        self.radioButton_NSample_Osci.setChecked(True)

        # Oscilloscope | pushButton
        self.pushButton_SorS_Osci.clicked.connect(self.osci_start_emit)

        # Oscilloscope | graphicsView
        self.osci_plot = self.view_Osci.addPlot()
        self.osci_plot.setDownsampling(mode='peak')
        self.osci_plot.setClipToView(True)
        self.osci_continuous_curve = self.osci_plot.plot(pen='y', name='Continuous')
        self.osci_nsample_curve = self.osci_plot.plot(pen='w', name='N sample')


        # Echo | pushButton
        self.pushButton_Start_Echo.clicked.connect(self.echo_start_emit)
        self.pushButton_Query_Echo.clicked.connect(self.echo_query_emit)

        # Echo | lineEdit
        self.lineEdit_1_Echo.setValidator(QIntValidator())

        # Feedback Test | comboBox
        self.comboBox_OutCh_FTest.currentIndexChanged.connect(self.ftest_get_ch)

        # Feedback Test | pushButton
        self.pushButton_Start_FTest.clicked.connect(self.ftest_start_emit)
        self.ftest_stop_signal.connect(self.ftest_stop_slot)

        # Feedback Test | graphicsView
        self.ftest_plot = self.view_FTest.addPlot()
        self.ftest_plot.setDownsampling(mode='peak')
        self.ftest_plot.setClipToView(True)
        self.ftest_plot.addLegend()
        self.ftest_output_curve = self.ftest_plot.plot(pen='w', name='Preamp voltage/ Current(nA)')
        self.ftest_input_curve = self.ftest_plot.plot(pen='y', name='feedback Z')

        # Feedback Test | spinBox
        self.spinBox_AInput_FTest.editingFinished.connect(self.get_ftest_parm)
        self.spinBox_Z0Input_FTest.editingFinished.connect(self.get_ftest_parm)
        self.spinBox_DelayInput_FTest.editingFinished.connect(self.get_ftest_parm)

    def init_etest(self, succeed, dacrange, adcrange, lastdigital, lastgain, lastdac, last20bit):
        # Set up view in case of successfully finding DSP
        self.enable_serial(succeed)
        self.init_io(dacrange, adcrange, lastdigital, lastgain, lastdac, last20bit)
        self.init_rtest(adcrange, dacrange)
        self.init_swave(dacrange)
        self.init_osci()
        self.init_ftest(lastdigital)
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

    # init I/O tab
    def init_io(self, dacrange, adcrange, lastdigital, lastgain, lastdac, last20bit):

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
        self.rotation.setChecked(not lastdigital[5])
        self.translation.setChecked(lastdigital[5])
        self.load_x_gain(lastgain)
        self.load_y_gain(lastgain)
        self.load_z1_gain(lastgain)
        self.load_z2_gain(lastgain)

        # load adc status from dsp
        adc_ch = self.adc_ch.currentIndex()
        adc_ran = adcrange[adc_ch]
        self.load_range(0, adc_ran)

        # load 16 bit dac status from dsp
        dac_ch = self.dac_ch.currentIndex()
        dac_ran = dacrange[dac_ch]
        self.load_range(1, dac_ran)
        self.load_dac_output(0, lastdac[dac_ch])

        # load 20 bit dac status from dsp
        self.load_dac_output(1, last20bit)

    # init Ramp Test tab
    def init_rtest(self, adcrange, dacrange):
        # load range from dsp
        outch = self.rtest_get_outch()
        if outch != 16:
            outran = dacrange[outch]
            self.set_spinBox_range(3, outch, outran)
            self.set_range_text(3, outran)
        else:
            outran = 16
            self.set_spinBox_range(3, outch, outran)
            self.set_range_text(3, outran)

        inch = self.rtest_get_inch()
        inran = adcrange[inch]
        self.set_range_text(2, inran)

    # init Square Wave tab
    def init_swave(self, dacrange):
        # load range from dsp
        ch = self.swave_get_ch()
        if ch != 16:
            ran = dacrange[ch]
            self.set_spinBox_range(4, ch, ran)
            self.set_range_text(4, ran)
        else:
            ran = 16
            self.set_spinBox_range(4, ch, ran)
            self.set_range_text(4, ran)

    # init Oscilloscope tab
    def init_osci(self):
        self.radioButton_NSample_Osci.setChecked(True)
        self.comboBox_InCh_Osci.setCurrentIndex(0)
        self.spinBox_AvTimes_Osci.setValue(0)
        self.spinBox_Samples_Osci.setValue(0)
        self.spinBox_Delay_Osci.setValue(0)

    # init Echo tab
    def init_echo(self):
        pass

    # init Feedback Test tab
    def init_ftest(self, lastdigital):
        self.radioButton_ON_Fdbk.setChecked(lastdigital[2])
        self.radioButton_OFF_Fdbk.setChecked(not lastdigital[2])
        self.radioButton_ON_Retr.setChecked(lastdigital[3])
        self.radioButton_OFF_Retr.setChecked(not lastdigital[3])


    # I/O | load dac(index = 0) and 20 bit dac(index = 1) output from dsp
    def load_dac_output(self, index, bits):
        if index == 0:  # 16 bit dac
            self.dac_bit.setValue(bits)
        else:           # 20 bit dac
            self.bit20_bit.setValue(bits)
            
    # I/O | load adc(index = 0) or dac(index = 1) range from dsp
    def load_range(self, index, ran):
        if index == 0:      # adc 
            if ran == 0:
                self.adc_b20.setChecked(True)
            elif ran == 1:
                self.adc_b10.setChecked(True)
            elif ran == 2:
                self.adc_b5.setChecked(True)
            elif ran == 5:
                self.adc_u10.setChecked(True)
            elif ran == 6:
                self.adc_u5.setChecked(True)
        elif index == 1:    # dac
            if ran == 0:
                self.dac_u5.setChecked(True)
            elif ran == 1:
                self.dac_b10_4.setChecked(True)
            elif ran == 9:
                self.dac_b10.setChecked(True)
            elif ran == 10:
                self.dac_b20.setChecked(True)
            elif ran == 14:
                self.dac_b5.setChecked(True)
            self.set_dac_spinBox_range(ran)      # update spinBox range
            self.dac_range[0] = ran              # update range variable

    # I/O | set dac spinBox range
    def set_dac_spinBox_range(self, ran):
        minimum = cnv.bv(0, 'd', ran)
        maximum = cnv.bv(0xffff, 'd', ran)
        self.dac_val.setMinimum(minimum)
        self.dac_val.setMaximum(maximum)
        self.dac_val.setValue(cnv.bv(self.dac_bit.value(), 'd', ran))

    # I/O | when adc(index = 0) or dac(index = 1) range changed by user, send signal
    def range_changed_emit(self, index, ran, status):
        if status:              # only receive signal from checked button
            self.range_changed_signal.emit(index, ran)
        else:
            pass

    # I/O & Ramp Test & Square Wave |
    # when I/O adc(index = 0) or dac(index = 1) channel changed by user, send signal
    # when Ramp Test input(index = 2) or output(index = 3) channel changed by user, send signal
    # when Square Wave output(index = 4) channel changed by user, send signal
    def ch_changed_emit(self, index, ch):
        if index == 0 or index ==1:     # I/O
            self.ch_changed_signal.emit(index, ch)
        elif index == 2:                # Ramp Test input
            ch = self.rtest_get_inch()
            self.ch_changed_signal.emit(index, ch)
        elif index == 3:                # Ramp Test output
            ch = self.rtest_get_outch()
            self.ch_changed_signal.emit(index, ch)
        elif index == 4:                # Square Wave
            ch = self.swave_get_ch()
            self.ch_changed_signal.emit(index, ch)

    # I/O | Feedback Test
    # when digital changed by user, send signal
    def digital_changed_emit(self, ch, status):
        self.digital_changed_signal.emit(ch, status)

    # I/O | when gain changed by user, send signal
    def gain_changed_emit(self, ch, val, status):
        if status:  # only receive signal from checked button
            self.gain_changed_signal.emit(ch, val)
        else:
            pass

    # I/O | dac output/zero button clicked, send output signal
    def dac_output_emit(self, index):
        addr = self.dac_ch.currentIndex() + 16
        if index == 0:          # zero button clicked
            bits = cnv.vb(0.0, 'd', self.dac_range[0])
            self.dac_bit.setValue(bits)
        else:
            bits = self.dac_bit.value()
        self.dac_output_signal.emit(addr, bits)

    # I/O | 20 bit dac output/zero button clicked, send output signal
    def bit20_output_emit(self, index):
        adrr = 0x10
        if index == 0:  # zero button clicked
            bits = 0x80000
            self.bit20_bit.setValue(0x80000)
        else:
            bits = self.bit20_bit.value()
        self.bit20_output_signal.emit(adrr, bits)

    # I/O | load x gain radio button status from dsp
    def load_x_gain(self, lastgain):
        if lastgain[0] == 0:
            self.xgain10.setChecked(True)
        elif lastgain[0] == 1:
            self.xgain1.setChecked(True)
        elif lastgain[0] == 3:
            self.xgain0_1.setChecked(True)

    # I/O | load y gain radio button status from dsp
    def load_y_gain(self, lastgain):
        if lastgain[1] == 0:
            self.ygain10.setChecked(True)
        elif lastgain[1] == 1:
            self.ygain1.setChecked(True)
        elif lastgain[1] == 3:
            self.ygain0_1.setChecked(True)

    # I/O | load z1 gain radio button status from dsp
    def load_z1_gain(self,lastgain):
        if lastgain[2] == 0:
            self.z1gain0_1.setChecked(True)
        elif lastgain[2] == 1:
            self.z1gain1.setChecked(True)
        elif lastgain[2] == 3:
            self.z1gain10.setChecked(True)

    # I/O | load z2 gain radio button status from dsp
    def load_z2_gain(self,lastgain):
        if lastgain[3] == 0:
            self.z2gain10.setChecked(True)
        elif lastgain[3] == 1:
            self.z2gain1.setChecked(True)
        elif lastgain[3] == 3:
            self.z2gain0_1.setChecked(True)

    # I/O | convert spinBox to scrollBar
    def spin2scroll(self, dac):
        if dac == 0:   # 16 bit dac
            value = self.dac_val.value()
            self.dac_bit.setValue(cnv.vb(value, 'd', self.dac_range[0]))
        else:  # 20 bit dac
            value = self.bit20_val.value()
            self.bit20_bit.setValue(cnv.vb(value,'20'))

    # I/O | convert scrollBar to spinBox
    def scroll2spin(self, dac, bit):
        if dac == 0:   # 16 bit dac
            self.dac_val.setValue(cnv.bv(bit, 'd', self.dac_range[0]))
        else:  # 20 bit dac
            self.bit20_val.setValue(cnv.bv(bit, '20'))
    
    # Ramp Test | get output channel
    def rtest_get_outch(self):
        outch = self.comboBox_OutCh_RTest.currentIndex()
        if outch == 13:     # 20 bit DAC
            return outch+3
        elif outch <= 3:    # 16 bit DAC 0~3
            return outch
        elif outch <= 10:   # 16 bit DAC 5~11
            return outch+1
        else:               # 16 bit DAC 13~15
            return outch+2
            
    # Ramp Test | get input channel
    def rtest_get_inch(self):
        inch = self.comboBox_InCh_RTest.currentIndex()
        if inch == 6:
            return inch+1
        else:
            return inch

    # Ramp Text | set initial and final spinBox range, set step size minimum
    # Square Wave | set voltage1 and voltage2 spinBox range
    def set_spinBox_range(self, index, ch, ran):
        if index == 3:  # ramp test
            if ch != 16:
                minimum = cnv.bv(0, 'd', ran)
                maximum = cnv.bv(0xffff, 'd', ran)
                self.spinBox_InitVal_RTest.setMinimum(minimum)
                self.spinBox_InitVal_RTest.setMaximum(maximum)
                self.spinBox_FinVal_RTest.setMinimum(minimum)
                self.spinBox_FinVal_RTest.setMaximum(maximum)
                self.spinBox_StepSize_RTest.setMinimum(cnv.bv(1, 'd', ran) - cnv.bv(0, 'd', ran))
            else:
                minimum = cnv.bv(0, '20')
                maximum = cnv.bv(0xfffff, '20')
                self.spinBox_InitVal_RTest.setMinimum(minimum)
                self.spinBox_InitVal_RTest.setMaximum(maximum)
                self.spinBox_FinVal_RTest.setMinimum(minimum)
                self.spinBox_FinVal_RTest.setMaximum(maximum)
                self.spinBox_StepSize_RTest.setMinimum(cnv.bv(1, '20') - cnv.bv(0, '20'))
        elif index == 4:    # square wave
            if ch != 16:
                minimum = cnv.bv(0, 'd', ran)
                maximum = cnv.bv(0xffff, 'd', ran)
                self.spinBox_V1_SWave.setMinimum(minimum)
                self.spinBox_V1_SWave.setMaximum(maximum)
                self.spinBox_V2_SWave.setMinimum(minimum)
                self.spinBox_V2_SWave.setMaximum(maximum)
            else:
                minimum = cnv.bv(0, '20')
                maximum = cnv.bv(0xfffff, '20')
                self.spinBox_V1_SWave.setMinimum(minimum)
                self.spinBox_V1_SWave.setMaximum(maximum)
                self.spinBox_V2_SWave.setMinimum(minimum)
                self.spinBox_V2_SWave.setMaximum(maximum)



    # Ramp Test | load input(index =2) and output(index = 3) range from dsp
    # Square Wave | load output(index=4) range from dsp
    def set_range_text(self, index, ran):
        if index == 2:                                 # ramp test input
            self.label_InRan_RTest.setText(self.range_text_dict[index][ran])
        elif index == 3:                               # ramp test output
            self.label_OutRan_RTest.setText(self.range_text_dict[index][ran])
        elif index == 4:                               # square wave output
            self.label_Range_SWave.setText(self.range_text_dict[3][ran])

    
    # Ramp Test | ramp (index=0) / ramp read (index=1) button
    def rtest_ramp_emit(self, index):
            outch = self.rtest_get_outch()
            inch = self.rtest_get_inch()
            if outch == 16:                             # 20 bit DAC
                init = cnv.vb(self.spinBox_InitVal_RTest.value(), '20')
                final = cnv.vb(self.spinBox_FinVal_RTest.value(), '20')
                step_size = (cnv.vb(self.spinBox_StepSize_RTest.value(), '20') - cnv.vb(0, '20'))
            else:                                       # 16 bit DAC
                init = cnv.vb(self.spinBox_InitVal_RTest.value(), 'd', self.dac_range[1])
                final = cnv.vb(self.spinBox_FinVal_RTest.value(), 'd', self.dac_range[1])
                step_size = (cnv.vb(self.spinBox_StepSize_RTest.value(), 'd', self.dac_range[1]) - cnv.vb(0, 'd', self.dac_range[1]))
            if self.idling:                             # emit ramp signal
                self.ptr2 = 0                           # init ramp update count
                self.rtest_ramp_read_outdata = [] * 100 # init ramp read output data
                self.rtest_ramp_read_indata = [] * 100  # init ramp read input data
                self.rtest_ramp_data = [] * 100         # init ramp data
                self.rtest_output_curve.clear()         # clear old plot
                self.rtest_input_curve.clear()          # clear old plot
                self.rtest_ramp_signal.emit(index, inch, outch, init, final, step_size)
            else:                                       # emit stop signal
                self.enable_serial(False)
                self.stop_signal.emit()                 # flip dsp.stop to True

    # Square Wave | start(stop) button slot
    def swave_start_emit(self):
        ch = self.swave_get_ch()
        delay = self.spinBox_Delay_SWave.value()
        if ch == 16:                    # 20 bit DAC
            voltage1 = cnv.vb(self.spinBox_V1_SWave.value(), '20')
            voltage2 = cnv.vb(self.spinBox_V2_SWave.value(), '20')
        else:                           # 16 bit DAC
            voltage1 = cnv.vb(self.spinBox_V1_SWave.value(), 'd', self.dac_range[2])
            voltage2 = cnv.vb(self.spinBox_V2_SWave.value(), 'd', self.dac_range[2])
        if self.idling:                 # emit signal
            self.swave_start_signal.emit(ch, voltage1, voltage2, delay)
        else:                           # emit stop signal
            self.enable_serial(False)
            self.stop_signal.emit()     # flip dsp.stop to True

    # Square Wave | get output channel
    def swave_get_ch(self):
        ch = self.comboBox_Ch_SWave.currentIndex()
        if ch == 13:        # 20 bit DAC
            return ch+3
        elif ch <= 3:       # 16 bit DAC 0~3
            return ch
        elif ch <= 10:      # 16 bit DAC 5~11
            return ch+1
        else:               # 16 bit DAC 13~15
            return ch+2

    # Oscilloscope | get input channel
    def osci_get_ch(self):
        ch = self.comboBox_InCh_Osci.currentIndex()
        if ch == 6:
            return ch+1
        else:
            return ch

    # Oscilloscope | get mode
    def osci_get_mode(self):
        if self.radioButton_NSample_Osci.isChecked():
            return 0
        elif self.radioButton_Contin_Osci.isChecked():
            return 1

    # Oscilloscope | start(stop) button slot
    def osci_start_emit(self):
        ch = self.osci_get_ch()
        mode = self.osci_get_mode()
        N = self.spinBox_Samples_Osci.value()
        avg_times = self.spinBox_AvTimes_Osci.value()
        delay = self.spinBox_Delay_Osci.value()
        if self.idling:                             # emit signal
            self.ptr1 = 0                           # init osc update count
            self.osci_nsample_data = []             # init N sample data
            self.osci_continuous_data = np.zeros(1) # init Continuous data
            self.osci_continuous_curve.clear()      # clear old plot
            self.osci_nsample_curve.clear()         # clear old plot
            self.osci_start_signal.emit(ch, mode, N, avg_times, delay)
        else:                                       # emit stop signal
            self.enable_serial(False)
            self.stop_signal.emit()                 # flip dsp.stop to True

    # Oscilloscope | change delay range based on mode
    def osci_set_delay_range(self, index):
        if index == 0:  # N sample mode selected
            minimum = 0
            maximum = 5000
            self.spinBox_Delay_Osci.setMinimum(minimum)
            self.spinBox_Delay_Osci.setMaximum(maximum)
            self.label_12.setText('us')
        else:           # Continuous mode selected
            minimum = 2
            maximum = 1000
            self.spinBox_Delay_Osci.setMinimum(minimum)
            self.spinBox_Delay_Osci.setMaximum(maximum)
            self.label_12.setText('ms')

    # Echo | start(stop) button slot
    def echo_start_emit(self):
        if self.idling:
            self.echo_start_signal.emit()
        else:
            self.enable_serial(False)
            self.stop_signal.emit()                 # flip dsp.stop to True

    # Echo | start(stop) button slot
    def echo_query_emit(self, input):
        input = self.lineEdit_1_Echo.text()
        input = int(input)
        self.echo_query_signal.emit(input)

    # Feedback Test | get output channel
    def ftest_get_ch(self):
        ch = self.comboBox_OutCh_FTest.currentIndex()
        if ch <= 3:
            return ch + 6
        else:
            return ch + 7

    # Feedback Test | get feedback A, Z0, delay
    def get_ftest_parm(self):
        self.ftest_parm[0] = self.spinBox_AInput_FTest.value()
        self.ftest_parm[1] = self.spinBox_Z0Input_FTest.value()
        self.ftest_parm[2] = self.spinBox_DelayInput_FTest.value()

    # Feedback Test | start(stop) button slot
    def ftest_start_emit(self):
        if self.idling:
            outch = self.ftest_get_ch()
            self.ptr3 = 0                           # init osc update count
            self.ftest_zout_data = np.zeros(1)      # init N sample data
            self.ftest_vout_data = np.zeros(1)      # init N sample data
            self.ftest_output_curve.clear()         # clear old plot
            self.ftest_input_curve.clear()          # clear old plot
            self.ftest_output_curve.setPos(0, 0)                 # reset plot origin
            self.ftest_input_curve.setPos(0, 0)                  # reset plot origin
            self.ftest_start_signal.emit(outch)
        else:
            self.enable_serial(False)
            self.ftest_stop_signal.emit()           # flip ftest_stop to True

    # Feedback Test | stop signal slot
    def ftest_stop_slot(self):
        if not self.ftest_stop:
            self.ftest_stop = True


    # Enable serial
    def enable_serial(self, enable):
        # tabs
        for tab in range(6):
            if tab != self.mode:
                self.Etest.setTabEnabled(tab, enable)

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
        self.comboBox_InCh_RTest.setEnabled(enable)
        self.comboBox_OutCh_RTest.setEnabled(enable)
        self.spinBox_InitVal_RTest.setEnabled(enable)
        self.spinBox_FinVal_RTest.setEnabled(enable)
        self.spinBox_StepSize_RTest.setEnabled(enable)

        # Square wave
        self.pushButton_Start_SWave.setEnabled(enable)
        self.comboBox_Ch_SWave.setEnabled(enable)
        self.spinBox_Delay_SWave.setEnabled(enable)
        self.spinBox_V1_SWave.setEnabled(enable)
        self.spinBox_V2_SWave.setEnabled(enable)

        # Oscilloscope
        self.pushButton_SorS_Osci.setEnabled(enable)
        self.comboBox_InCh_Osci.setEnabled(enable)
        self.radioButton_Contin_Osci.setEnabled(enable)
        self.radioButton_NSample_Osci.setEnabled(enable)
        self.spinBox_AvTimes_Osci.setEnabled(enable)
        self.spinBox_Samples_Osci.setEnabled(enable)
        self.spinBox_Delay_Osci.setEnabled(enable)

        # Echo
        # self.pushButton_Query_Echo.setEnabled(enable)
        self.pushButton_Start_Echo.setEnabled(enable)

        # Feedback test
        self.pushButton_Start_FTest.setEnabled(enable)
        self.radioButton_ON_Fdbk.setEnabled(enable)
        self.radioButton_OFF_Fdbk.setEnabled(enable)
        self.radioButton_ON_Retr.setEnabled(enable)
        self.radioButton_OFF_Retr.setEnabled(enable)
        self.comboBox_OutCh_FTest.setEnabled(enable)

    # Emit close signal
    def closeEvent(self, event):
        if self.idling:
            self.close_signal.emit()
            event.accept()
        else:
            QMessageBox.warning(None, "Etest", "Process ongoing!", QMessageBox.Ok)
            event.ignore()



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myEtest()
    window.show()
    sys.exit(app.exec_())

    '''
       -> signal - slot connection <-
    # Common
    self.etest.Etest.currentChanged.connect(self.init_tab_slot)
    self.etest.Etest.tabBarClicked.connect(self.init_tab_slot)
       
    # I/O
    self.etest.range_changed_signal.connect(self.range_changed_slot)
    self.etest.ch_changed_signal.connect(self.ch_changed_slot)
    self.etest.digital_changed_signal.connect(self.dsp.digital_o)   
    self.etest.gain_changed_signal.connect(self.dsp.gain)  
    self.etest.adc_input_signal.connect(self.adc_input_slot)
    self.etest.dac_output_signal.connect(self.dsp.dac_W)   
    self.etest.bit20_output_signal.connect(self.dsp.bit20_W)
    
    # Ramp Test
    self.etest.stop_signal.connect(self.stop_slot)
    
    # Square Wave
    self.etest.swave_start_signal.connect(self.swave_start_slot)
    
    # Oscilloscope
    self.etest.osci_start_signal.connect(self.osci_start_slot)
    self.dsp.oscc_signal.connect(self.osci_update)
    
    # Echo
    self.etest.echo_start_signal.connect(self.echo_start_slot)
    self.etest.echo_query_signal.connect(self.echo_query_slot)
    
    # Feedback
    self.etest.ftest_start_signal.connect(self.ftest_start_slot)
    

    '''


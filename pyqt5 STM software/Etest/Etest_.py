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
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
from Etest_ui import Ui_ElectronicTest
import numpy as np
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
    bit20_output_signal = pyqtSignal(int, int)
    # Ramp Test signals
    rtest_ramp_signal = pyqtSignal(int, int, int, int, int, int)
    stop_signal = pyqtSignal()
    # Ramp Test plot timer
    timer = QtCore.QTimer()

    def __init__(self):
        super().__init__()
        self.idling = True
        self.mode = 0   # 0: I/O, 1: RampTest, 2: SquareWave, 3: Oscilloscope, 4: Echo, 5: FeedbackTest
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
        self.dac_range = 10     # dac range variable
        self.range_text_dict = {2: {0: '-10.24V to +10.24V', 1: '-5.12 V to +5.12 V', \
                                    2: '-2.56 V to +2.56 V', 5: '0 to 10.24 V', \
                                    6: '0 to 5.12 V'}, \
                                3: {0: '0 to 5V', 1: '0 to 10V', 2: '0 to 20V', 4: '0 to 40 V', \
                                    9: '-5 V to +5 V', 10: '-10 V to +10 V', 12: '-20 V to +20 V', \
                                    14: '-2.5 V to +2.5 V', 20: '-5V to +5V'}}    # input(2), output(3) range -> text dictionary
        self.rtest_ramp_read_outdata = [] * 100        # Ramp Test | ramp read output data
        self.rtest_ramp_read_indata = [] * 100         # Ramp Test | ramp read input data
        self.rtest_ramp_data = [] * 100                # Ramp Test | ramp data
        self.ptr2 = 0                          # Ramp Test | ramp plot update count
        self.plot_test_data1 = [0] * 100       # !!! just for plot test
        self.plot_test_data2 = [0] * 100

        # adc | pushButton
        self.adc_in.clicked.connect(self.adc_input_signal)

        # adc | radioButton
        self.adc_range_group = QButtonGroup()
        self.adc_range_group.addButton(self.adc_b20,0)
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
        self.dither0_on.toggled.connect(lambda: self.digital_changed_emit(0))
        self.dither1_on.toggled.connect(lambda: self.digital_changed_emit(1))
        self.feedback_on.toggled.connect(ft.partial(self.digital_changed_emit, 2))
        self.retract_on.toggled.connect(lambda: self.digital_changed_emit(3))
        self.coarse.toggled.connect(ft.partial(self.digital_changed_emit, 4))
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
        self.rtest_output_curve1 = self.rtest_ramp_plot.plot(pen='y', name='RampTo')
        self.rtest_output_curve2 = self.rtest_ramp_plot.plot(pen='w', name='RampMeasure')
        self.timer.timeout.connect(self.rtest_plot_update)


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

    def init_etest(self, succeed, dacrange, adcrange, lastdigital, lastgain, lastdac, last20bit):
        # Set up view in case of successfully finding DSP
        self.enable_serial(succeed)
        self.init_io(dacrange, adcrange, lastdigital, lastgain, lastdac, last20bit)
        self.init_rtest(adcrange, dacrange)
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
        # !!! rtest_get_inch() and rtest_get_outch() can NOT be merged into one function
        outch = self.rtest_get_outch()
        if outch != 20:
            outran = dacrange[outch]
            self.set_rtest_spinBox_range(outch, outran)
            self.set_range_text(3, outran)
        else:
            outran = 20
            self.set_rtest_spinBox_range(outch, outran)
            self.set_range_text(3, outran)

        inch = self.rtest_get_inch()
        inran = adcrange[inch]
        self.set_range_text(2, inran)

    # init Square Wave tab
    def init_swave(self):
        pass

    # init Oscilloscope tab
    def init_osci(self):
        pass

    # init Echo tab
    def init_echo(self):
        pass

    # init Feedback Test tab
    def init_ftest(self):
        pass


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
            self.set_dac_spinBox_range(ran)   # update spinBox range
            self.dac_range = ran              # update range variable

    # I/O | set dac spinBox range
    def set_dac_spinBox_range(self, ran):
        minimum = cnv.bv(0, 'd', ran)
        maximum = cnv.bv(0xffff, 'd', ran)
        self.dac_val.setMinimum(minimum)
        self.dac_val.setMaximum(maximum)

    # I/O | when adc(index = 0) or dac(index = 1) range changed by user, send signal
    def range_changed_emit(self, index, ran, status):
        if status:              # only receive signal from checked button
            self.range_changed_signal.emit(index, ran)
        else:
            pass

    # I/O & Ramp Test |
    # when I/O adc(index = 0) or dac(index = 1) channel changed by user, send signal
    # when Ramp Test input(index = 2) or output(index = 3) channel changed by user, send signal
    def ch_changed_emit(self, index, ch):
        if index == 0 or index ==1:     # I/O
            self.ch_changed_signal.emit(index, ch)
        elif index == 2:                # Ramp Test input
            ch = self.rtest_get_inch()
            self.ch_changed_signal.emit(index, ch)
        elif index == 3:                # Ramp Test output
            ch = self.rtest_get_outch()
            self.ch_changed_signal.emit(index, ch)

    # I/O | when digital changed by user, send signal
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
        adrr = self.dac_ch.currentIndex() + 16
        if index == 0:          # zero button clicked
            bits = 0x8000;
            self.dac_bit.setValue(0x8000)
        else:
            bits = self.dac_bit.value()
        self.dac_output_signal.emit(adrr, bits)

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
            self.dac_bit.setValue(cnv.vb(value, 'd', self.dac_range))
        else:  # 20 bit dac
            value = self.bit20_val.value()
            self.bit20_bit.setValue(cnv.vb(value,'20'))

    # I/O | convert scrollBar to spinBox
    def scroll2spin(self, dac, bit):
        if dac == 0:   # 16 bit dac
            self.dac_val.setValue(cnv.bv(bit, 'd', self.dac_range))
        else:  # 20 bit dac
            self.bit20_val.setValue(cnv.bv(bit, '20'))
    
    # Ramp Test | get output channel
    def rtest_get_outch(self):
        outch = self.comboBox_OutCh_RTest.currentIndex()
        if outch == 14:     # 20 bit DAC
            return outch+6
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
    def set_rtest_spinBox_range(self, ch, ran):
        if ch != 20:
            minimum = cnv.bv(0, 'd', ran)
            maximum = cnv.bv(0xffff, 'd', ran)
            self.spinBox_InitVal_RTest.setMinimum(minimum)
            self.spinBox_FinVal_RTest.setMaximum(maximum)
            self.spinBox_StepSize_RTest.setMinimum(cnv.bv(-1, 'd', ran) - cnv.bv(0, 'd', ran))
        else:
            minimum = cnv.bv(0, '20')
            maximum = cnv.bv(0xfffff, '20')
            self.spinBox_InitVal_RTest.setMinimum(minimum)
            self.spinBox_FinVal_RTest.setMaximum(maximum)
            self.spinBox_StepSize_RTest.setMinimum(cnv.bv(-1, '20') - cnv.bv(0, '20'))

    # Ramp Test | load input(index =2) and output(index = 3) range from dsp
    def set_range_text(self, index, ran):
        if index == 2:                                 # input
            self.label_InRan_RTest.setText(self.range_text_dict[index][ran])
        else:                                          # output
            self.label_OutRan_RTest.setText(self.range_text_dict[index][ran])
    
    # Ramp Test | ramp (index=0) / ramp read (index=1) button
    def rtest_ramp_emit(self, index):
            outch = self.rtest_get_outch()
            inch = self.rtest_get_inch()
            if outch == 20:                             # 20 bit DAC
                init = cnv.vb(self.spinBox_InitVal_RTest.value(), '20')
                final = cnv.vb(self.spinBox_FinVal_RTest.value(), '20')
                step_size = (cnv.vb(self.spinBox_StepSize_RTest.value(), '20') - cnv.vb(0, '20'))
            else:                                       # 16 bit DAC
                init = cnv.vb(self.spinBox_InitVal_RTest.value(), 'd', self.dac_range)
                final = cnv.vb(self.spinBox_FinVal_RTest.value(), 'd', self.dac_range)
                step_size = (cnv.vb(self.spinBox_StepSize_RTest.value(), 'd', self.dac_range) - cnv.vb(0, 'd', self.dac_range))
            if self.idling:                             # emit ramp signal
                self.ptr2 = 0                           # init ramp update count
                self.rtest_ramp_read_outdata = [] * 100         # init ramp data list
                self.rtest_ramp_read_indata = [] * 100    # init ramp read data list
                self.rtest_ramp_signal.emit(index, inch, outch, init, final, step_size)
                self.timer.start(50)                    # plot update start
            else:                                       # emit stop signal
                self.timer.stop()
                # self.enable_serial(False)
                self.stop_signal.emit()                 # flip dsp.stop to True

    # Ramp Test | plot update
    def rtest_plot_update(self):

        # if self.ptr2 >= len(self.rtest_ramp_read_outdata):
        #     tmp = self.rtest_ramp_read_outdata
        #     self.rtest_ramp_read_outdata = [0] * (len(self.rtest_ramp_read_outdata) * 2)
        #     self.rtest_ramp_read_outdata[:len(tmp)] = tmp
        # self.rtest_output_curve.setData(self.rtest_ramp_read_outdata[:self.ptr2])
        # self.rtest_output_curve.setPos(self.ptr2, 0)

        #
        # !!! just for plot test
        #
        # self.plot_test_data1[self.ptr2] = np.random.normal()
        # self.plot_test_data2[self.ptr2] = np.sin(self.ptr2/10)
        # self.ptr2 += 1
        # if self.ptr2 >= len(self.plot_test_data1):
        #     tmp1 = self.plot_test_data1
        #     self.plot_test_data1 = [0] * (len(self.plot_test_data1) * 2)
        #     self.plot_test_data1[:len(tmp1)] = tmp1
        # self.rtest_output_curve1.setData(self.plot_test_data1[:self.ptr2])
        # self.rtest_output_curve1.setPos(self.ptr2, 0)
        #
        # if self.ptr2 >= len(self.plot_test_data2):
        #     tmp2 = self.plot_test_data2
        #     self.plot_test_data2 = [0] * (len(self.plot_test_data2) * 2)
        #     self.plot_test_data2[:len(tmp2)] = tmp2
        # self.rtest_output_curve2.setData(self.plot_test_data2[:self.ptr2])
        # self.rtest_output_curve2.setPos(self.ptr2, 0)
        pass

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

    # Emit close signal
    def closeEvent(self, event):
        if self.idling:
            self.close_signal.emit()
            self.timer.stop()
            event.accept()
        else:
            # !!! pop window, ongoing
            QMessageBox.warning(None,"Etest","Process ongoing!",QMessageBox.Ok)
            event.ignore()



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myEtest()
    window.show()
    sys.exit(app.exec_())

    '''
       -> signal - slot connection <-
       
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
    '''


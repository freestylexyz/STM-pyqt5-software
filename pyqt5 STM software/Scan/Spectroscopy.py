# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:19:02 2020

@author: yaoji
"""

import sys
sys.path.append("../ui/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
from PyQt5.QtWidgets import QApplication, QWidget, QFileDialog, QMessageBox
from PyQt5.QtCore import pyqtSignal
from Spectroscopy_ui import Ui_Spectroscopy
from AdvanceOption import myAdvanceOption
from SpectroscopyInfo import mySpectroscopyInfo
from DataStruct import SpcData
from images import myImages
import pyqtgraph as pg
import numpy as np
import conversion as cnv
import functools as ft
import pickle
from datetime import datetime

class mySpc(QWidget, Ui_Spectroscopy):
    # Control signal
    close_signal = pyqtSignal()         # Close spectroscopy window signal
    spectroscopy_signal = pyqtSignal()  # Spectroscopy start scan signal
    stop_signal = pyqtSignal()          # Spectroscopy stop scan signal
    seq_list_signal = pyqtSignal(str)   # Open sequence list window signal
    update_plot_signal = pyqtSignal()   # Update current plot signal
    update_ran_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()
    
    # Setup UI
    def init_UI(self):
        self.adv = myAdvanceOption()        # Advance option window
        self.info = mySpectroscopyInfo()    # Spectroscopy data information window
        self.dlg = QFileDialog()            # File dialog window
        self.data = SpcData()               # Spectroscopy data
        self.img = myImages()               # Plot processing function

        # Set up file dialog window
        self.dlg.setFileMode(QFileDialog.AnyFile)
        self.dlg.setAcceptMode(QFileDialog.AcceptSave)
        self.dlg.setNameFilter('SPC Files (*.spc)')
        
        # For automatically assign file name
        self.today = datetime.now().strftime("%m%d%y")
        self.file_index = 0
        
        # System status
        self.bias_dac = False   # Bias DAC selection
        self.bias_ran = 9       # Bias DAC range
        self.idling = True      # Idling status
        self.saved = True       # Saved status
        self.point_index = 0    # Current displayed point index
        self.point_num = 0      # Number of points

        # Auto save variables
        self.autosave_name = ''
        self.every = False

        # Data for plot
        self.fwd_data = []      # Current plot forward data
        self.bwd_data = []      # Current plot bacward data

        # Conversions
        self.spinBox_Min_General.editingFinished.connect(lambda: self.min_cnv(True, 0))
        self.spinBox_Max_General.editingFinished.connect(lambda: self.max_cnv(True, 0))
        self.spiBox_StepSize_General.editingFinished.connect(lambda: self.step_cnv(True, 0))
        self.spinBox_Bias_Delta.editingFinished.connect(lambda: self.bias_cnv(True, 0))
        self.spinBox_Delta_Z.editingFinished.connect(lambda: self.scrollBar_Z_Delta.setValue(self.spinBox_Delta_Z.value()))
        self.scrollBar_Min_General.valueChanged.connect(ft.partial(self.min_cnv, False))
        self.scrollBar_Max_General.valueChanged.connect(ft.partial(self.max_cnv, False))
        self.scrollBar_StepSize_General.valueChanged.connect(ft.partial(self.step_cnv, False))
        self.scrollBar_Bias_Delta.valueChanged.connect(ft.partial(self.bias_cnv, False))
        self.scrollBar_Z_Delta.valueChanged.connect(self.spinBox_Delta_Z.setValue)
        
        # Status change
        self.comboBox_RampCh_General.currentIndexChanged.connect(self.channel_change)
        self.comboBox_RampCh_General.currentIndexChanged.connect(self.delta_change)

        # Flag change
        self.checkBox_record_General.stateChanged.connect(self.record_lockin)
        
        # Buttons
        self.pushButton_Save.clicked.connect(self.save)                     # Save data
        self.pushButton_Scan.clicked.connect(self.scan_emit)                # Scan button
        self.adv.pushButton_SeqEditor.clicked.connect(self.open_seq_list)   # Open sequence list
        self.pushButton_Info.clicked.connect(self.open_info)                # Open data info window
        self.pushButton_Advance.clicked.connect(self.adv.show)              # Open advance option window
        
        # Graph
        self.scrollBar_point_Graph.valueChanged.connect(lambda: self.label_point_Graph.setText(str(self.scrollBar_point_Graph.value())))
        self.scrollBar_point_Graph.valueChanged.connect(self.point_changed)
        self.pushButton_Scanner_Cur.clicked.connect(lambda: self.show_scanner(0))
        self.pushButton_Scanner_Avg.clicked.connect(lambda: self.show_scanner(1))
        self.pushButton_xScale_Cur.clicked.connect(lambda: self.scale_cur(0))
        self.pushButton_yScale_Cur.clicked.connect(lambda: self.scale_cur(1))
        self.pushButton_xScale_Avg.clicked.connect(lambda: self.scale_avg(0))
        self.pushButton_yScale_Avg.clicked.connect(lambda: self.scale_avg(1))
        self.update_plot_signal.connect(self.update_cur_plot)

        # label | display scanner coordinates
        self.label_cur = pg.LabelItem(justify='right')
        self.graphicsView_current_Spectro.addItem(self.label_cur, row=0, col=0)
        self.label_avg = pg.LabelItem(justify='right')
        self.graphicsView_avg_Spectro.addItem(self.label_avg, row=0, col=0)

        # graphicsView |
        self.plot_cur = self.graphicsView_current_Spectro.addPlot(row=1, col=0)
        # self.plot_cur.disableAutoRange()
        self.view_box_cur = self.plot_cur.getViewBox()
        self.plot_avg = self.graphicsView_avg_Spectro.addPlot(row=1, col=0)
        # self.plot_avg.disableAutoRange()
        self.view_box_avg = self.plot_avg.getViewBox()

        # ROI | data scanner
        gray_pen = pg.mkPen((150, 150, 150, 255), width=1)
        self.scanner_vLine_cur = pg.InfiniteLine(angle=90, movable=False, pen=gray_pen)
        self.plot_cur.addItem(self.scanner_vLine_cur, ignoreBounds=True)
        self.scanner_vLine_cur.hide()
        self.scanner_vLine_avg = pg.InfiniteLine(angle=90, movable=False, pen=gray_pen)
        self.plot_avg.addItem(self.scanner_vLine_avg, ignoreBounds=True)
        self.scanner_vLine_avg.hide()

    # Initial spectroscopy window
    def init_spc(self, succeed, bias_dac, bias_ran, seq_list, selected):
        # Variables
        self.bias_dac = bias_dac
        self.bias_ran = bias_ran
        
        # Enable buttons based on succeed
        self.pushButton_Scan.setEnabled(succeed)

        # Setup controls
        self.setup_spin(0)
        self.setup_scroll(0)
        self.min_cnv(False, 0)
        self.max_cnv(False, 0xfffff)
        self.step_cnv(False, 1)

        # Set selected sequence name
        seq_name = '' if selected < 0 else seq_list[selected].name
        self.adv.label_Seq_AdvOption.setText(seq_name)
        
    # Emit open sequence list signal
    def open_seq_list(self):
        self.seq_list_signal.emit(self.adv.label_Seq_AdvOption.text())    # Get current selected sequence name
        
    # Emit scan signal
    def scan_emit(self):
        flag = self.saved        # If able to start flag
        
        # Ask if overwrite data if not saved
        if self.idling and (not flag):
            msg = QMessageBox.question(None, "Spectroscopy", "Spectrum not saved, do you want to continue?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            flag = (msg == QMessageBox.Yes)

        if self.idling and flag:    # Start case if idling and able to start
            self.spectroscopy_signal.emit()         # Emit spectroscopy start signal
        elif not self.idling:       # Stop case if not idling
            self.pushButton_Scan.setEnabled(False)  # Disable stop button to avoid sending stop signal twice
            self.stop_signal.emit()                 # Emit stop signal

    # Point scrollBar slot
    def point_changed(self, index):
        if self.data.data.shape[0] == self.point_num:
            self.point_index = index - 1
            self.update_plot_signal.emit()
            self.update_avg_plot()
        elif self.data.data.shape[0] < self.point_num:
            if index <= self.data.data.shape[0]:
                self.point_index = index - 1
                self.update_plot_signal.emit()
                self.update_avg_plot()
            else:
                self.plot_cur.clear()

    # Update spectroscopy current pass data
    def update_spc(self, rdata):
        self.update_ran_signal.emit()
        f, b = self.data.update_data(rdata)     # Update current pass data and obtain forward data and backward data for plot
        self.fwd_data = f
        self.bwd_data = b
        self.update_plot_signal.emit()

    # Update spectroscopy averaged data
    def update_spc_(self, pass_num):
        self.data.combine_data()                # Combine forward and backward data
        if self.every:
            self.auto_save(self.auto_save_name, pass_num+1)
        self.data.avg_data()                    # Average current pass data with previous passes
        self.update_avg_plot()

    # Get current z offset fine range to convert data unit
    def get_zfine_range(self, ran):
        self.zfine_ran = ran

    # Convert data unit from bits to volts
    def cnv_bit2volt(self, bit_data):
        ch = self.comboBox_RampCh_General.currentIndex()    # Determine ramp channel
        bias_flag = '20' if self.bias_dac else 'd'          # Conversion flag
        if ch == 0: # Bias
            volt_data = [cnv.bv(data, bias_flag, self.bias_ran) for data in bit_data]
        else: # Z offset fine
            volt_data = [cnv.bv(data, 'd', self.zfine_ran) for data in bit_data]
        return volt_data

    # Update plot signal slot
    def update_cur_plot(self):
        self.plot_cur.clear()
        for i in range(1, self.fwd_data.shape[0]):
            x = self.cnv_bit2volt(self.fwd_data[0])
            y = self.cnv_bit2volt(self.fwd_data[i])
            self.plot_cur.plot(x=x, y=y, pen=self.img.color[i % 16])
        for i in range(1, self.bwd_data.shape[0]):
            x = self.cnv_bit2volt(self.bwd_data[0])
            y = self.cnv_bit2volt(self.bwd_data[i])
            self.plot_cur.plot(x=x, y=y, pen=self.img.color[i % 16])

    # Update averaged plot, called by update_spc_
    def update_avg_plot(self):
        self.plot_avg.clear()
        for i in range(1, self.data.data[self.point_index].shape[0]):
            x = self.cnv_bit2volt(self.data.data[self.point_index][0])
            y = self.cnv_bit2volt(self.data.data[self.point_index][i])
            self.plot_avg.plot(x=x, y=y, pen=self.img.color[i])

    # Graphics | scanner mouse moved signal slot
    def mouseMoved(self, index, evt):
        if index == 0:
            pos = evt[0]  ## using signal proxy turns original arguments into a tuple
            if self.plot_cur.sceneBoundingRect().contains(pos):
                mousePoint = self.plot_cur.vb.mapSceneToView(pos)
                data_x = [] * len(self.plot_cur.listDataItems())
                data_y = [] * len(self.plot_cur.listDataItems())
                color = [] * len(self.plot_cur.listDataItems())
                for curve in self.plot_cur.listDataItems():
                    near_x = min(curve.xData, key=lambda x: abs(x - mousePoint.x()))
                    index = list(curve.xData).index(near_x)
                    data_x += [near_x]
                    data_y += [curve.yData[index]]
                    color += [self.img.RGB_to_Hex(curve.curve.opts['pen'].brush().color().getRgb())]

                text = "<span style='font-size: 7pt'><span style='color:'w'>x=%0.2f   <br /></span>" % (data_x[0])
                num = 0
                for i in range(len(data_x)):
                    num += 1
                    text += "<span style='font-size: 7pt'><span style='color: " + color[
                        i] + "'>y = %0.2f </span>" % (data_y[i])
                    if num % 5 == 0:
                        text += "<br />"
                self.label_cur.setText(text)
                self.scanner_vLine_cur.setPos(mousePoint.x())

        elif index == 1:
            pos = evt[0]  ## using signal proxy turns original arguments into a tuple
            if self.plot_avg.sceneBoundingRect().contains(pos):
                mousePoint = self.plot_avg.vb.mapSceneToView(pos)
                data_x = [] * len(self.plot_avg.listDataItems())
                data_y = [] * len(self.plot_avg.listDataItems())
                color = [] * len(self.plot_avg.listDataItems())
                for curve in self.plot_avg.listDataItems():
                    near_x = min(curve.xData, key=lambda x: abs(x - mousePoint.x()))
                    index = list(curve.xData).index(near_x)
                    data_x += [near_x]
                    data_y += [curve.yData[index]]
                    color += [self.img.RGB_to_Hex(curve.curve.opts['pen'].brush().color().getRgb())]

                text = "<span style='font-size: 7pt'><span style='color:'w'>x=%0.2f   <br /></span>" % (data_x[0])
                num = 0
                for i in range(len(data_x)):
                    num += 1
                    text += "<span style='font-size: 7pt'><span style='color: " + color[
                        i] + "'>y = %0.2f </span>" % (data_y[i])
                    if num % 5 == 0:
                        text += "<br />"
                self.label_avg.setText(text)
                self.scanner_vLine_avg.setPos(mousePoint.x())

    # Graphics | show crosshair data scanner
    def show_scanner(self, index):
        if index == 0:
            if self.pushButton_Scanner_Cur.isChecked():
                self.plot_cur.addItem(self.scanner_vLine_cur, ignoreBounds=True)
                self.proxy = pg.SignalProxy(self.plot_cur.scene().sigMouseMoved, rateLimit=60, slot=ft.partial(self.mouseMoved, 0))
                self.scanner_vLine_cur.show()
                self.label_cur.show()
            else:
                self.plot_cur.removeItem(self.scanner_vLine_cur)
                self.label_cur.hide()
        elif index == 1:
            if self.pushButton_Scanner_Avg.isChecked():
                self.plot_avg.addItem(self.scanner_vLine_avg, ignoreBounds=True)
                self.proxy = pg.SignalProxy(self.plot_avg.scene().sigMouseMoved, rateLimit=60, slot=ft.partial(self.mouseMoved, 1))
                self.scanner_vLine_avg.show()
                self.label_avg.show()
            else:
                self.plot_avg.removeItem(self.scanner_vLine_avg)
                self.label_avg.hide()
                
    # Graphics | scale by X/Y axis
    def scale_cur(self, index):
        if index == 0:  # scale x
            self.plot_cur.vb.enableAutoRange(axis='x', enable=True)
            self.plot_cur.vb.updateAutoRange()
        elif index == 1:  # sacle y
            self.plot_cur.vb.enableAutoRange(axis='y', enable=True)
            self.plot_cur.vb.updateAutoRange()
            
    # Graphics | scale by X/Y axis
    def scale_avg(self, index):
        if index == 0:  # scale x
            self.plot_avg.vb.enableAutoRange(axis='x', enable=True)
            self.plot_avg.vb.updateAutoRange()
        elif index == 1:  # sacle y
            self.plot_avg.vb.enableAutoRange(axis='y', enable=True)
            self.plot_avg.vb.updateAutoRange()

    # Open spectroscopy information window
    def open_info(self):
        self.info.init_spcInfo(self.data)
        self.info.show()
    
    # Emit close signal
    def closeEvent(self, event):
        if self.idling:
            self.close_signal.emit()
            event.accept()
        else:
            self.message('Process ongoing')
            event.ignore()
    
    # Pop out message
    def message(self, text):
        QMessageBox.warning(None, "Spectroscopy", text, QMessageBox.Ok)
    
    # Enable serial
    def enable_serial(self, enable):
        self.General.setEnabled(enable)             # Enable general options groupbox
        self.pushButton_Save.setEnabled(enable)     # Enable save button
        self.pushButton_Scan.setEnabled(enable)     # Enable scan button
        self.pushButton_Info.setEnabled(enable)     # Enable info button
        self.view_box_cur.setMouseEnabled(x=enable, y=enable)   # Enable and disable mouse event
        self.view_box_avg.setMouseEnabled(x=enable, y=enable)  # Enable and disable mouse event

    # Bias range changing slot
    def bias_ran_change(self, bias_ran):
        self.bias_ran = bias_ran    # Set up storage variable
        
        # Set up spinboxes and scrollbars appearance
        ch = self.comboBox_RampCh_General.currentIndex()
        if ch == 0:                 # Only reset controls when dealing with bias ramping
            self.channel_change(0)
    
    # Change ramp channel slot
    def channel_change(self, ch):
        self.setup_spin(ch)             # Set up spinboxes range
        self.setup_scroll(ch)           # Set up scrollbars range
        self.min_cnv(False, 0)          # Set minimum scrollbar to 0
        self.max_cnv(False, 0xfffff)    # Set maximum scrollbar to 0xfffff
        self.step_cnv(False, 1)         # Set step to scrollbar 1

    # Change ramp channel slot
    def delta_change(self, ch):
        self.spinBox_Bias_Delta.setEnabled(ch)
        self.scrollBar_Bias_Delta.setEnabled(ch)
        self.spinBox_Delta_Z.setEnabled(1-ch)
        self.scrollBar_Z_Delta.setEnabled(1-ch)

    # Record lock-in params checkBox slot
    def record_lockin(self, index):
        state = True if index == 2 else False
        self.data.lockin_flag = state
        self.pushButton_LockIn_General.setEnabled(state)
    
    # Set scrollbars range
    def setup_scroll(self, ch):
        if (not ch) and self.bias_dac:  # If ramp 20bit bias DAC
            scroll_max = 0xfffff    # 20bit maximum
        else:
            scroll_max = 0xffff     # 16bit maximum
                
        self.scrollBar_Max_General.setMaximum(scroll_max)       # Set the maximum of 'Max'
        self.scrollBar_Min_General.setMaximum(scroll_max - 1)   # Set the maximum of 'Min' (Min need always smaller than Max)
        
        # Start bias for delta channel
        if self.bias_dac:   # 20bit DAC
            self.scrollBar_Bias_Delta.setMaximum(0xfffff)
        else:               # 16bit DAC
            self.scrollBar_Bias_Delta.setMaximum(0xffff)
            
    # Set spinboxes range
    def setup_spin(self, ch):
        ran_dict = {9: 5.0, 10: 10.0, 15: 2.5}                      # Voltage limit for 16bit bias DAC
        ran = 5.0 if self.bias_dac else ran_dict[self.bias_ran]     # 20bit always +/- 5.0V
            
        if ch:      # Ramp Z offset fine
            # Use double spinboxes as spinboxes
            self.spinBox_Min_General.setDecimals(0)
            self.spinBox_Max_General.setDecimals(0)
            self.spiBox_StepSize_General.setDecimals(0)
            
            self.spinBox_Min_General.setMinimum(-0x8000)    # Set the minimum of 'Min'
            self.spinBox_Max_General.setMaximum(0x7fff)     # Set the maximum of 'Max'
            self.spiBox_StepSize_General.setMinimum(1)      # Set the minimum of 'step'
        else:       # Ramp bias
            # Determine precision based on 20bit DAC
            self.spinBox_Min_General.setDecimals(7)
            self.spinBox_Max_General.setDecimals(7)
            self.spiBox_StepSize_General.setDecimals(7)
            
            self.spinBox_Min_General.setMinimum(-ran)       # Set the minimum of 'Min'
            self.spinBox_Max_General.setMaximum(ran)        # Set the maximum of 'Max'
            self.spiBox_StepSize_General.setMinimum(0)      # Set the minimum of 'step'
            
        # Set up delta channel range
        self.spinBox_Bias_Delta.setMinimum(-ran)
        self.spinBox_Bias_Delta.setMaximum(ran)
        
    # Min spin box to scroll bar
    def min_cnv(self, flag, value):
        ch = self.comboBox_RampCh_General.currentIndex()    # Determine ramp channel
        bias_flag = '20' if self.bias_dac else 'd'          # Conversion flag
        bias_mid = 0x80000 if self.bias_dac else 0x8000     # Different bias mid scale
        step = self.scrollBar_StepSize_General.value()      # Current step size
        if ch:      # Ramp Z offset fine
            if flag:    # Spinbox to scrollbar
                value = self.spinBox_Min_General.value() + 0x8000                                       # Get value from spinbox
            value = min(value, max(self.scrollBar_Max_General.value() - step, 0))                       # Manually limit (at least one step)
            self.spinBox_Min_General.setValue(value - 0x8000)                                           # Set spinbox
            self.scrollBar_Min_General.setValue(int(value))                                             # Set scrollbar
            self.spinBox_Max_General.setMinimum(self.scrollBar_Min_General.value() - 0x8000 + step)     # Set the minimum of 'Max' (at least one step)
            
            step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()          # Set the maximum of 'Step' (at least one step)
            self.spiBox_StepSize_General.setMaximum(step_max)   
            data_num = int(step_max / self.scrollBar_StepSize_General.value())                          # Display data number
            self.label_DataNum_General.setText(str(data_num))
        else:       # Ramp bias
            if flag:    # Spinbox to scrollbar
                value = cnv.vb(self.spinBox_Min_General.value(), bias_flag, self.bias_ran)
            value = min(value, max(self.scrollBar_Max_General.value() - step, 0))   # 0 -1.000061
            self.spinBox_Min_General.setValue(cnv.bv(value, bias_flag, self.bias_ran))
            self.scrollBar_Min_General.setValue(int(value))
            self.spinBox_Max_General.setMinimum(cnv.bv(self.scrollBar_Min_General.value() + step, bias_flag, self.bias_ran))
            
            step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
            self.spiBox_StepSize_General.setMaximum(cnv.bv(step_max + bias_mid, bias_flag, self.bias_ran))
            data_num = int(step_max / self.scrollBar_StepSize_General.value())
            self.label_DataNum_General.setText(str(data_num))
            
    # Max spin box to scroll bar
    def max_cnv(self, flag, value):
        ch = self.comboBox_RampCh_General.currentIndex()
        bias_flag = '20' if self.bias_dac else 'd'
        bias_max = 0xfffff if self.bias_dac else 0xffff
        bias_mid = 0x80000 if self.bias_dac else 0x8000
        step = self.scrollBar_StepSize_General.value()
        if ch:      # Ramp Z offset fine
            if flag:    # Spinbox to scrollbar
                value = self.spinBox_Max_General.value() + 0x8000
            value = max(value, min(self.scrollBar_Min_General.value() + step, 0xffff))    
            self.spinBox_Max_General.setValue(value - 0x8000)
            self.scrollBar_Max_General.setValue(int(value))
            self.spinBox_Min_General.setMaximum(self.scrollBar_Max_General.value() - 0x8000 - step)
            
            step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
            self.spiBox_StepSize_General.setMaximum(step_max)
        else:       # Ramp bias
            if flag:    # Spinbox to scrollbar
                value = cnv.vb(self.spinBox_Max_General.value(), bias_flag, self.bias_ran)
            value = max(value, min(self.scrollBar_Min_General.value() + step, bias_max))
            self.spinBox_Max_General.setValue(cnv.bv(value, bias_flag, self.bias_ran))
            self.scrollBar_Max_General.setValue(int(value))
            self.spinBox_Min_General.setMaximum(cnv.bv(self.scrollBar_Max_General.value() - step, bias_flag, self.bias_ran))

            step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
            self.spiBox_StepSize_General.setMaximum(cnv.bv(step_max + bias_mid, bias_flag, self.bias_ran))
        data_num = int(step_max / self.scrollBar_StepSize_General.value())
        self.label_DataNum_General.setText(str(data_num))

    # Step change slot
    def step_cnv(self, flag, value):
        ch = self.comboBox_RampCh_General.currentIndex()
        bias_flag = '20' if self.bias_dac else 'd'
        bias_mid = 0x80000 if self.bias_dac else 0x8000
        step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
        if ch:      # Ramp Z offset fine
            if flag:    # Spinbox to scrollbar
                value = self.spiBox_StepSize_General.value()
            value = min(max(step_max, 1), value)
            self.spiBox_StepSize_General.setValue(value)
            self.scrollBar_StepSize_General.setValue(int(value))
            self.spinBox_Max_General.setMinimum(self.scrollBar_Min_General.value() - 0x8000 + value)
            self.spinBox_Min_General.setMaximum(self.scrollBar_Max_General.value() - 0x8000 - value)
        else:       # Ramp bias
            if flag:    # Spinbox to scrollbar
                value = cnv.vb(self.spiBox_StepSize_General.value(), bias_flag, self.bias_ran) - bias_mid
            value = min(max(step_max, 1), value)
            self.scrollBar_StepSize_General.setValue(value)
            self.spiBox_StepSize_General.setValue(cnv.bv(value + bias_mid, bias_flag, self.bias_ran))
            self.spinBox_Max_General.setMinimum(cnv.bv(self.scrollBar_Min_General.value() + value, bias_flag, self.bias_ran))
            self.spinBox_Min_General.setMaximum(cnv.bv(self.scrollBar_Max_General.value() - value, bias_flag, self.bias_ran))

        step_max = self.scrollBar_Max_General.value() - self.scrollBar_Min_General.value()
        data_num = int(step_max / self.scrollBar_StepSize_General.value())
        self.label_DataNum_General.setText(str(data_num))
            
    # Start bias conversion slot
    def bias_cnv(self, flag, value):
        bias_flag = '20' if self.bias_dac else 'd'
        if flag:    # Spinbox to scrollbar
            value = self.spinBox_Bias_Delta.value()
            self.scrollBar_Bias_Delta.setValue(cnv.vb(value, bias_flag, self.bias_ran))
        else:       # Scrollbar to spinbox
            self.spinBox_Bias_Delta.setValue(cnv.bv(value, bias_flag, self.bias_ran))
            
    # Configure spectroscopy general options
    def configure_spc(self, dsp):
        start = self.scrollBar_Min_General.value()
        final = self.scrollBar_Max_General.value()
        step = self.scrollBar_StepSize_General.value()
        data_num = int((final - start) / step)
        pass_num = self.spinBox_Pass_General.value()
        delta_flag = self.groupBox_Delta_General.isChecked()      
        if self.comboBox_RampCh_General.currentIndex():             # Ramp Z offset fine
            ramp_ch = 0x12                                                              # Ramp channel address
            delta_ch = 0x20 if self.bias_dac else 0x1d                                  # Delta channel address
            b = dsp.lasdac[13] if self.bias_dac else dsp.last20bit                      # Current bias data
            delta_data = (self.scrollBar_Bias_Delta.value() - b) if delta_flag else 0   # Calculate delta data
        else:                                                      # Ramp bias
            ramp_ch = 0x20 if self.bias_dac else 0x1d
            delta_ch = 0x12
            delta_data = self.scrollBar_Z_Delta.value() if delta_flag else 0            # Collect delta data

        return start, step, data_num, pass_num, ramp_ch, delta_ch, delta_data

    # Save data slot
    def save(self):
        name = self.auto_save_window()      # Pop out file dialog for saving
        self.auto_save(name, 0)             # Save averaged data

    # Configure auto save
    def configure_autosave(self):
        # Auto save is only enabled when auto save enabled AND setup proper file name
        self.autosave_name = self.auto_save_window() if self.adv.groupBox_AutoSave_AdvOption.isChecked() else ''
        # Save every pass will not be enabled is auto save is not enabled
        self.every = self.adv.checkBox_SaveEveryPasses_Autosave.isChecked() and self.autosave_name

    # Auto save pop window
    def auto_save_window(self):
        # Re-init file index if date change
        if self.data.time.strftime("%m%d%y") != self.today:
            self.today = self.data.time.strftime("%m%d%y")
            self.file_index = 0
        name_list = '0123456789abcdefghijklmnopqrstuvwxyz'                                      # Name list
        name = self.today + name_list[self.file_index // 36] + name_list[self.file_index % 36]    # Auto configure file name
        self.dlg.selectFile(self.dlg.directory().path() + '/' + name + '.spc')                  # Set default file name as auto configured
        
        if self.dlg.exec_():        # File selected
            directory = self.dlg.directory().path()                                                     # Get directory path
            self.dlg.setDirectory(directory)                                                            # Set directory path for next call
            save_name = self.dlg.selectedFiles()[0].replace(directory + '/', '').replace('.spc', '')    # Get the real file name
            
            # If default file name is not used
            if save_name != name:
                try:    # See if current file name is in our naming system
                    if save_name[0:6] == self.today:        # Reset file index if match
                        self.file_index = name_list.index(save_name[6]) * 36 + name_list.index(save_name[7])
                    else:
                        self.file_index -= 1
                except: # Otherwise do not consume file index
                    self.file_index -= 1
                name = save_name
        else:                       # No file selected
            name = ''   # Empty file name
        return name

    # Auto save function
    def auto_save(self, name, pass_num):
        if pass_num:        # Save single pass
            fname = self.dlg.directory().path() + '/' + name + '_pass' + str(pass_num) + '.spc'     # Single pass file name
        else:               # Save averaged pass data
            fname = self.dlg.directory().path() + '/' + name + '.spc'   # Averaged data file name
            self.file_index += 1                                        # Consume 1 file index
            self.saved = True                                           # Toggle saved flag
            self.setWindowTitle('Spectroscopy-' + name)                 # Change window title for saving status indication
        with open(fname, 'wb') as output:
            self.data.path = fname                                      # Save path
            pickle.dump(self.data, output, pickle.HIGHEST_PROTOCOL)     # Save data
            # print(self.data.data.shape)
            print('save finished')


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mySpc()
    window.init_spc(True, False, 9, [], -1)
    '''' test plot '''
    for i in range(16):
        window.plot_cur.plot(np.random.normal(size=100), pen=window.img.color[i])
        window.plot_avg.plot(np.random.normal(size=100), pen=window.img.color[i])
    window.show()
    sys.exit(app.exec_())
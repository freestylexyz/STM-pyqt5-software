# -*- coding: utf-8 -*-
"""
@Date     : 2021/3/21 20:57:05
@Author   : milier00
@FileName : ImageProcessing.py
"""
import sys
import os
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication, QWidget, QDesktopWidget, QMessageBox, QButtonGroup, QMainWindow, QAction, QVBoxLayout, QCheckBox, QRadioButton, QFileDialog, QShortcut
from PyQt5.QtCore import pyqtSignal, Qt, QDir, QRectF, QThread, QTimer
from PyQt5.QtGui import QIntValidator, QIcon
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph
import pyqtgraph.opengl as gl
from ImageProcessing_ui import Ui_ImageProcessing
from GraphWindow import myGraphWindow
import numpy as np
import conversion as cnv
import functools as ft
import math
import pickle


class WorkThread(QThread):
    trigger = pyqtSignal()

    def __int__(self):
        super(WorkThread, self).__init__()

    def run(self):
        for i in range(2000000000):
            pass
        self.trigger.emit()

class myImageProcessing(QMainWindow, Ui_ImageProcessing):
    # Common signal
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()

    def init_UI(self):
        self.dir_path = ''
        self.file_paths = []    # file paths in the list widget
        self.file_index = 0
        self.file_names = []    # file names in the list widget
        self.windows = []       # window object list
        self.window_names = []  # window names
        self.linecut_option = 1  # 0: option1, 1: option2

        self.current_window = None
        self.previous_window = None

        self.timer = QTimer()
        self.workThread = WorkThread()

        # pushButton |
        self.pushButton_Open.clicked.connect(self.open_file)
        self.pushButton_Saveas.clicked.connect(self.save_as)
        self.pushButton_Saveall.clicked.connect(self.save_all)
        self.pushButton_Previous.clicked.connect(lambda: self.select_file(0))
        self.pushButton_Next.clicked.connect(lambda: self.select_file(1))
        self.pushButton_Refresh.clicked.connect(self.refresh_file)
        # self.pushButton_PlaneFitDo.clicked.connect(self.plane_fit)
        self.pushButton_Export.clicked.connect(self.export_linecut)
        self.pushButton_Open3D.clicked.connect(self.open_3D)

        # groupBox |
        self.groupBox_LineCut.toggled.connect(self.linecut_checked)
        self.groupBox_Option1.toggled.connect(ft.partial(self.linecut_option_changed, 0))
        self.groupBox_Option2.toggled.connect(ft.partial(self.linecut_option_changed, 1))

        # spinBox and slider |
        self.spinBox_Angle_Linecut.editingFinished.connect(lambda: self.spin2slider(2))
        self.slider_Angle_Linecut.valueChanged.connect(ft.partial(self.slider2spin, 2))

        self.spinBox_x_Linecut.editingFinished.connect(lambda: self.linecut_changed(0))
        self.spinBox_y_Linecut.editingFinished.connect(lambda: self.linecut_changed(0))
        self.spinBox_Angle_Linecut.editingFinished.connect(lambda: self.linecut_changed(0))
        self.slider_Angle_Linecut.valueChanged.connect(lambda: self.linecut_changed(0))

        self.spinBox_x1_Linecut.editingFinished.connect(lambda: self.linecut_changed(1))
        self.spinBox_y1_Linecut.editingFinished.connect(lambda: self.linecut_changed(1))
        self.spinBox_x2_Linecut.editingFinished.connect(lambda: self.linecut_changed(1))
        self.spinBox_y2_Linecut.editingFinished.connect(lambda: self.linecut_changed(1))

        # listWidget |
        self.listWidget.itemDoubleClicked.connect(lambda: self.file_changed(0))
        self.listWidget.itemClicked.connect(lambda: self.file_changed(1))
        self.listWidget.setCurrentRow(-1)

        # signals |
        QApplication.instance().focusObjectChanged.connect(self.focus_object_changed)
        QApplication.instance().focusChanged.connect(self.focus_window_changed)
        self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint)

        # keyboard event |
        QShortcut(QtGui.QKeySequence('Up', ), self, lambda: self.select_file(0))
        QShortcut(QtGui.QKeySequence('Down', ), self, lambda: self.select_file(1))
        QShortcut(QtGui.QKeySequence('Left', ), self, lambda: self.select_file(0))
        QShortcut(QtGui.QKeySequence('Right', ), self, lambda: self.select_file(1))

    # Data List | open folder button slot
    def open_file(self):
        self.dir_path = QFileDialog.getExistingDirectory(self, "Please choose folder", "../data/")

        self.file_paths.clear()
        self.file_names.clear()
        self.listWidget.clear()

        for root, dirs, files in os.walk(self.dir_path, topdown=False):
            for file in files:
                if file[-4:] == ".stm":
                    data_path = os.path.join(root, file)
                    with open(data_path, 'rb') as input:
                        data = pickle.load(input)
                        data.path = data_path  # Change file path
                    if data.data.shape[0] == 1:
                        self.file_paths.append(data_path)
                        self.file_names.append(file)
                    else:
                        for i in range(data.data.shape[0]):
                            self.file_paths.append(data_path)
                            # name = file if i==0 else (file+"_"+str(i))
                            self.file_names.append(file+"_No"+str(i+1))

        if len(self.file_paths) <= 0:
            return

        self.lineEdit.setText(self.dir_path)
        self.listWidget.addItems(self.file_names)

        # refresh file list every second
        # self.timer.start(5000)
        # self.workThread.start()
        # self.workThread.trigger.connect(self.timer.stop)
        # self.timer.timeout.connect(self.refresh_file)

    # Data List | select previous/next file
    def select_file(self, index):
        if index == 0:      # previous
            if self.listWidget.currentRow()-1 == -1:
                self.listWidget.setCurrentRow(len(self.file_names)-1)
            else:
                self.listWidget.setCurrentRow(self.listWidget.currentRow()-1)
        elif index == 1:    # next
            if self.listWidget.currentRow()+1 == len(self.file_names):
                self.listWidget.setCurrentRow(0)
            else:
                self.listWidget.setCurrentRow(self.listWidget.currentRow()+1)
        self.file_index = self.listWidget.currentRow()
        self.file_changed(1)

    # Data List | refresh file list
    def refresh_file(self):
        # print("refresh!!!")
        self.file_paths.clear()
        self.file_names.clear()

        for root, dirs, files in os.walk(self.dir_path, topdown=False):
            for file in files:
                if file[-4:] == ".stm":
                    data_path = os.path.join(root, file)
                    with open(data_path, 'rb') as input:
                        data = pickle.load(input)
                        data.path = data_path  # Change file path
                    if data.data.shape[0] == 1:
                        self.file_paths.append(data_path)
                        self.file_names.append(file)
                    else:
                        for i in range(data.data.shape[0]):
                            self.file_paths.append(data_path)
                            # name = file if i==0 else (file+"_"+str(i))
                            self.file_names.append(file + "_No" + str(i + 1))

        if len(self.file_paths) <= 0:
            return

        self.listWidget.clear()
        self.listWidget.addItems(self.file_names)
        self.listWidget.setCurrentRow(self.file_index)

    # Data List | file selection changed slot
    def file_changed(self, index):

        if index == 0:  # double click slot
            self.file_index = self.listWidget.currentRow()
            window_name = self.file_names[self.listWidget.currentRow()]
            if len(window_name)>12:
                img_index = int(window_name[15:])-1
            else:
                img_index = 0

            name_count = 0
            for name in self.window_names:
                if window_name in name:
                    name_count += 1

            # img_index = 0
            imgwindow = myGraphWindow(self.file_paths[self.listWidget.currentRow()], img_index)
            imgwindow.setObjectName(window_name)
            imgwindow.close_signal.connect(self.window_closed)

            if name_count == 0:
                imgwindow.setWindowTitle(window_name)
                self.window_names.append(window_name)
            else:
                imgwindow.setWindowTitle(window_name + "_" + str(name_count))
                self.window_names.append(window_name + "_" + str(name_count))
            self.windows.append(imgwindow)

            self.init_info(imgwindow.data)
            imgwindow.show()

        elif index == 1:    # single click slot
            if self.listWidget.count()>0:
                self.file_index = self.listWidget.currentRow()
                data_path = self.file_paths[self.listWidget.currentRow()]
                window_name = self.file_names[self.listWidget.currentRow()]

                # get image index if multi-img file
                if len(window_name) > 12:
                    img_index = int(window_name[15:]) - 1
                else:
                    img_index = 0

                # get the num of opened windows for this file
                name_count = 0
                for name in self.window_names:
                    if window_name in name:
                        name_count += 1

                # delete old window name from window name list
                if self.previous_window != None:
                    name_index = self.window_names.index(self.previous_window.windowTitle())
                    self.window_names.pop(name_index)

                    # set up window title
                    if name_count == 0:
                        self.previous_window.setWindowTitle(window_name)
                        self.window_names.insert(name_index, window_name)
                    else:
                        self.previous_window.setWindowTitle(window_name + "_" + str(name_count))
                        self.window_names.insert(name_index, window_name + "_" + str(name_count))

                    # update data
                    self.previous_window.update_data(data_path, img_index)
                    self.init_info(self.previous_window.data)

    # Data List | save as button slot
    def save_as(self):
        if self.previous_window != self.window():
            file_name = self.previous_window.windowTitle().replace('.stm', '') if  self.previous_window.windowTitle().find('.stm') != -1 else self.previous_window.windowTitle()
            default_name = "../data/" + file_name
            fileName, ok = QFileDialog.getSaveFileName(self, "Save", default_name, "DAT(*.dat);; PNG (*.png);; GIF(*.gif);; JPG(*.jpg);; BMP(*.bmp)")
            if fileName[-4:] in [".png", ".jpg", ".gif", ".bmp"]:
                self.previous_window.img_display.save(fileName)
            elif fileName[-4:] == ".dat":
                np.savetxt(fileName, self.previous_window.data.data[self.previous_window.img_index])

    # Data List | save all button slot
    def save_all(self):

        # get directory and file type
        fileName, ok = QFileDialog.getSaveFileName(self, "Save all", 'default',
                                                   "DAT(*.dat);; PNG(*.png);; GIF(*.gif);; JPG(*.jpg);; BMP(*.bmp)")
        dir = fileName.replace(fileName[fileName.find("default"):],'')
        type_dic = {"DAT(*.dat)": ".dat", "PNG(*.png)": ".png", "GIF(*.gif)": ".gif", "JPG(*.jpg)": ".jpg", "BMP(*.bmp)": ".bmp"}
        type = type_dic[ok]

        # save all windows
        for window in self.windows:
            fileName = window.windowTitle().replace('.stm','') \
                if window.windowTitle().find('.stm') != -1 \
                else window.windowTitle()
            default_name = dir + fileName + type
            if type in [".png", ".jpg", ".gif", ".bmp"]:
                print("saving!!!")
                window.img_display.save(default_name)
            elif type == ".dat":
                np.savetxt(default_name, window.data.data[window.img_index])

    # Data List | change graph in current widow
    def focus_window_changed(self, old, new):
        if new != None:
            self.current_window = new.window()
        if old != None and old.window() != self.window():
            self.previous_window = old.window()

    # Info | show top window info
    def init_info(self, data):
        # Title
        file_name = data.path[-12:]
        self.setWindowTitle('Scan Data Info --- ' + str(file_name))

        # Date and Time
        self.label_date.setText(str(data.time)[0:10])
        self.label_time.setText(str(data.time)[11:19])

        # Delay
        self.label_move_delay.setText(str(data.move_delay))
        self.label_read_delay.setText(str(data.measure_delay))
        self.label_line_delay.setText(str(data.line_delay))

        # Scan
        direction = ', Read Forward' if data.dir_x else ', Read Backward'
        order = 'Y first' if data.channel_x == 0x1f else 'X first'
        self.label_scan_direction.setText(order + direction)
        scan_protection_dict = {0: 'No protection', 1: 'Stop scan', 2: 'Auto 0 and continue',
                                3: 'Auto to previous midpoint and continue'}
        if data.scan_protect_flag == 0:
            self.label_scan_protection.setText(scan_protection_dict[data.scan_protect_flag])
        else:
            self.label_scan_protection.setText(
                scan_protection_dict[data.scan_protect_flag] + 'with Zout limit' + str(data.limit))
        tip_protection = 'Protected' if data.tip_protection else 'No protection'
        tip_protect_data = ',retracted Zoffset fine '
        if data.tip_protection:
            self.label_tip_protection.setText(tip_protection + tip_protect_data + str(data.tip_protect_data) + ' bits')
        else:
            self.label_tip_protection.setText(tip_protection)

        # !!! Feedback off pre-scan
        self.label_pre_scan.setText('')
        self.label_advance_z.setText(str(data.advance_bit))

        # Status
        bias_dac = '20bit DAC' if data.bias_dac else '16bit DAC'
        self.label_bias_dac.setText(bias_dac)
        bias = cnv.bv(data.lastdac[13], 'd', data.dacrange[13])
        self.label_bias.setText(str(bias))
        bias_dither = 'ON' if data.lastdigital[0] == 1 else 'OFF'
        self.label_bias_dither.setText(str(bias_dither))
        z_dither = 'ON' if data.lastdigital[1] == 1 else 'OFF'
        self.label_zdither.setText(str(z_dither))
        feedback = 'ON' if data.lastdigital[2] == 1 else 'OFF'
        self.label_feedback.setText(str(feedback))
        x_in = data.offset[0]
        self.label_xin.setText(str(x_in))
        x_offset = data.lastdac[1]
        self.label_xoffset.setText(str(x_offset))
        z_offset_fine = data.lastdac[2]
        self.label_zoffset_fine.setText(str(z_offset_fine))
        z_offset = data.lastdac[3]
        self.label_zoffset.setText(str(z_offset))
        i_set = cnv.bv(data.lastdac[5], 'd', data.dacrange[5])
        self.label_iset.setText(str(i_set))
        y_offset = data.lastdac[14]
        self.label_yoffset.setText(str(y_offset))
        y_in = data.lastdac[15]
        self.label_yin.setText(str(y_in))
        x_gain = '10.0' if data.lastgain[0] == 0 else ('1.0' if data.lastgain[0] == 1 else '0.1')
        self.label_xgain.setText(str(x_gain))
        y_gain = '10.0' if data.lastgain[1] == 0 else ('1.0' if data.lastgain[1] == 1 else '0.1')
        self.label_ygain.setText(str(y_gain))
        z1_gain = '10.0' if data.lastgain[2] == 3 else ('1.0' if data.lastgain[2] == 1 else '0.1')
        self.label_zgain1.setText(z1_gain)
        z2_gain = '10.0' if data.lastgain[3] == 3 else ('1.0' if data.lastgain[3] == 1 else '0.1')
        self.label_zgain2.setText(str(z2_gain))
        preamp_gain = data.preamp_gain
        self.label_preamp_gain.setText(str(preamp_gain))
        dac_range_dict = {0: '0 to 5V', 1: '0 to 10V', 2: '0 to 20V', 4: '0 to 40V', 9: '-V to +5V', 10: '-10V to +10V',
                          12: '-20V to +20V', 14: '-2.5V to +2.5V'}
        bias_range = dac_range_dict[data.dacrange[13]]
        self.label_bias_range.setText(bias_range)

        # !!! Lock-in parameter
        # if data.lockin_flag:
        if False:
            self.label_modulation_type.setText(data.osc_type)
            self.label_osc_amp.setText(str(data.osc_rms))
            self.label_lockin_frequency.setText(str(data.lockin_freq))
            self.label_sensitivity1.setText(str(data.sen1))
            self.label_sensitivity2.setText(str(data.sen2))
            self.label_osc_frequency.setText(str(data.osc_freq))
            self.label_phase1.setText(str(data.phase1))
            self.label_phase2.setText(str(data.phase2))
            self.label_offset1.setText(str(data.offset1))
            self.label_offset2.setText(str(data.offset2))
            self.label_time_const1.setText(str(data.tc1))
            self.label_time_const2.setText(str(data.tc2))
        else:
            self.label_modulation_type.setText('NA')
            self.label_osc_amp.setText('NA')
            self.label_lockin_frequency.setText('NA')
            self.label_sensitivity1.setText('NA')
            self.label_sensitivity2.setText('NA')
            self.label_osc_frequency.setText('NA')
            self.label_phase1.setText('NA')
            self.label_phase2.setText('NA')
            self.label_offset1.setText('NA')
            self.label_offset2.setText('NA')
            self.label_time_const1.setText('NA')
            self.label_time_const2.setText('NA')

        # Sequence
        self.label_seq_name.setText(data.seq.name)
        seq_feedback = 'ON' if data.seq.feedback else 'OFF'
        self.label_seq_feedback.setText(seq_feedback)
        seq_bias_dither = 'ON' if data.seq.ditherB else 'OFF'
        self.label_seq_bias_dither.setText(seq_bias_dither)
        seq_z_dither = 'ON' if data.seq.ditherZ else 'OFF'
        self.label_seq_z_dither.setText(seq_z_dither)

        # If command_list and command do not match, use command_list, else command
        if len(data.seq.command_list) != len(data.seq.command):

            # Init 5 sequence lists
            data.seq.command = []
            data.seq.channel = []
            data.seq.option1 = []
            data.seq.option2 = []
            data.seq.data = []

            # Translate command_list and data_list to 5 sequence lists
            for i in range(data.seq.seq_num):
                if data.seq.command_list[i] == 0x00:
                    data.seq.command += ['Wait']
                    channel = data.seq.command_list[i] - 0x00
                    data.seq.channel += [
                        list(data.seq.channelDict.keys())[list(data.seq.channelDict.values()).index(channel)]]
                    data.seq.data += [str(data.seq.data_list[i])]
                    data.seq.option1 += [0]
                    data.seq.option2 += [0]
                elif (data.seq.command_list[i] >= 0x40) and (data.seq.command_list[i] <= 0x45):
                    data.seq.command += ['Dout']
                    channel = data.seq.command_list[i] - 0x40
                    data.seq.channel += [
                        list(data.seq.channelDict.keys())[list(data.seq.channelDict.values()).index(channel)]]
                    data.seq.data += [str(data.seq.data_list[i] & 0x1)]
                    data.seq.option1 += [0]
                    data.seq.option2 += [0]
                elif (data.seq.command_list[i] >= 0xc0) and (data.seq.command_list[i] <= 0xdc):
                    data.seq.command += ['Read']
                    channel = (data.seq.command_list[i] - 0xc0) / 4
                    data.seq.channel += [
                        list(data.seq.channelDict.keys())[list(data.seq.channelDict.values()).index(channel)]]
                    data.seq.data += [str(data.seq.data_list[i] & 0xffff)]
                    data.seq.option1 += [0]
                    data.seq.option2 += [0]

        # Use 5 sequence lists to generate str description list
        seq_description = []  # Init str description list
        for i in range(data.seq.seq_num):
            if data.seq.command[i] == 'Wait':
                seq_description += [str(i + 1) + '  Wait ' + data.seq.data[i] + ' us']
            elif data.seq.command[i] == 'Match':
                option = '  Fast' if data.seq.option1[i] else '  Slow'
                seq_description += [str(i + 1) + option + ' Match current']
            elif data.seq.command[i] == 'Dout':
                status = 'ON' if data.seq.data[i] else 'OFF'
                if data.seq.channel[i] == 'DitherZ':
                    seq_description += [str(i + 1) + '  Z dither ' + status]
                elif data.seq.channel[i] == 'DitherB':
                    seq_description += [str(i + 1) + '  Bias dither ' + status]
                elif data.seq.channel[i] == 'Feedback':
                    seq_description += [str(i + 1) + '  Feedback ' + status]
            elif data.seq.command[i] == 'Shift':
                direction = ' Up' if data.seq.option1[i] else ' Down'
                if (data.seq.channel[i] == 'Z offset fine') or (data.seq.channel[i] == 'Z offset'):
                    seq_description += [
                        str(i + 1) + '  Shift ' + data.seq.channel[i] + direction + ' by ' + data.seq.data[i] + ' bits']
                elif data.seq.channel[i] == 'Iset':
                    seq_description += [
                        str(i + 1) + '  Shift ' + data.seq.channel[i] + direction + ' by ' + data.seq.data[i] + ' nA']
                else:
                    seq_description += [
                        str(i + 1) + '  Shift ' + data.seq.channel[i] + direction + ' by ' + data.seq.data[
                            i] + ' volts']
            elif data.seq.command[i] == 'Aout':
                if data.seq.option1 == 1:
                    seq_description += [str(i + 1) + '  Analog Output ' + data.seq.channel[i] + ' to Original']
                else:
                    if (data.seq.channel[i] == 'Z offset fine') or (data.seq.channel[i] == 'Z offset'):
                        seq_description += [
                            str(i + 1) + '  Analog Output ' + data.seq.channel[i] + ' to ' + data.seq.data[i] + ' bits']
                    elif data.seq.channel[i] == 'Iset':
                        seq_description += [
                            str(i + 1) + '  Analog Output ' + data.seq.channel[i] + ' to ' + data.seq.data[i] + ' nA']
                    else:
                        seq_description += [
                            str(i + 1) + '  Analog Output ' + data.seq.channel[i] + ' to ' + data.seq.data[
                                i] + ' volts']
            elif data.seq.command[i] == 'Ramp':
                if data.seq.option1[i] == 1:
                    seq_description += [str(i + 1) + '  Ramp ' + data.seq.channel[i] + ' to Original with speed ' + str(
                        float(data.seq.option2[i]) / 10.0) + ' bits/ms']
                else:
                    if (data.seq.channel[i] == 'Z offset fine') or (data.seq.channel[i] == 'Z offset'):
                        seq_description += [str(i + 1) + '  Ramp ' + data.seq.channel[i] + ' to ' + data.seq.data[
                            i] + ' bits with speed ' + str(float(data.seq.option2[i]) / 10.0) + ' bits/ms']
                    elif data.seq.channel[i] == 'Iset':
                        seq_description += [str(i + 1) + '  Ramp ' + data.seq.channel[i] + ' to ' + data.seq.data[
                            i] + ' nA with speed ' + str(float(data.seq.option2[i]) / 10.0) + ' bits/ms']
                    else:
                        seq_description += [str(i + 1) + '  Ramp ' + data.seq.channel[i] + ' to ' + data.seq.data[
                            i] + ' volts with speed ' + str(float(data.seq.option2[i]) / 10.0) + ' bits/ms']
            elif data.seq.command[i] == 'Read':
                seq_description += [str(i + 1) + '  Read ' + data.seq.channel[i] + data.seq.data[i] + ' times']
            elif data.seq.command[i] == 'ShiftRamp':
                if (data.seq.channel[i] == 'Z offset fine') or (data.seq.channel[i] == 'Z offset'):
                    seq_description += [str(i + 1) + '  ShiftRamp ' + data.seq.channel[i] + ' by ' + data.seq.data[
                        i] + ' bits with speed ' + str(float(data.seq.option2[i]) / 10.0) + ' bits/ms']
                elif data.seq.channel[i] == 'Iset':
                    seq_description += [str(i + 1) + '  ShiftRamp ' + data.seq.channel[i] + ' by ' + data.seq.data[
                        i] + ' nA with speed ' + str(float(data.seq.option2[i]) / 10.0) + ' bits/ms']
                else:
                    seq_description += [str(i + 1) + '  ShiftRamp ' + data.seq.channel[i] + ' by ' + data.seq.data[
                        i] + ' volts with speed ' + str(float(data.seq.option2[i]) / 10.0) + ' bits/ms']

            self.listWidget_seq.addItem(seq_description[i])

        # Other information
        self.label_of_image.setText(str(data.data.shape[0]))
        self.label_steps.setText(str(data.step_num))
        self.label_step_size.setText(str(data.step_size))
        match_curr = 'ON' if data.match_curr else 'OFF'
        self.label_match_curr.setText(match_curr)

    # Graph Windows | top level graph window changed slot
    def focus_object_changed(self, obj):
        if not obj or obj.window() == self.window() or obj.window().objectName()=='dockWidget_2':
            return
        if obj.window().objectName() != '':
            current_index = self.file_names.index(obj.window().objectName())
            self.listWidget.setCurrentRow(current_index)
            self.current_window = obj.window()
            self.current_window.linecut.sigRegionChanged.connect(lambda: self.linecut_changed(2))
            self.current_window.update_display_signal.connect(self.setup_range)

    # Graph Windows | window close slot
    def window_closed(self, title):
        index = self.window_names.index(title)
        self.window_names.pop(index)
        self.windows.pop(index)

    # Processing | set up line cut range
    def setup_range(self):
        if (self.current_window != None) and (self.current_window != self.window()):
            width = self.current_window.img_display.width()
            height = self.current_window.img_display.height()
            self.spinBox_x1_Linecut.setMaximum(width)
            self.spinBox_y1_Linecut.setMaximum(height)
            self.spinBox_x2_Linecut.setMaximum(width)
            self.spinBox_y2_Linecut.setMaximum(height)
            self.spinBox_x_Linecut.setMaximum(width)
            self.spinBox_y_Linecut.setMaximum(height)

    # Processing | Linecut slot
    def linecut_checked(self, status):
        if status:
            self.previous_window.linecut.show()
        else:
            self.previous_window.linecut.hide()

    # Processing | Linecut option slot
    def linecut_option_changed(self, index, status):
        if status:
            self.previous_window.linecut.setPos([0, 0])
            if index == 0:
                self.previous_window.linecut.removeHandle(0)
                pos = [self.previous_window.linecut.pos()[0], self.previous_window.linecut.pos()[1]]
                self.previous_window.linecut_rot_handle1 = self.previous_window.linecut.addRotateHandle(pos=pos, center=pos, index=0)
                self.groupBox_Option2.setChecked(False)
                self.linecut_option = 0
            elif index == 1:
                self.previous_window.linecut.removeHandle(0)
                pos = [self.previous_window.linecut.pos()[0], self.previous_window.linecut.pos()[1]]
                self.previous_window.linecut_free_handle1 = self.previous_window.linecut.addTranslateHandle(pos=pos, index=0)
                # self.previous_window.linecut.setAngle(0, snap=True)
                # self.previous_window.linecut.update()
                self.groupBox_Option1.setChecked(False)
                self.linecut_option = 1

    # Processing | Linecut
    def linecut_changed(self, index):
        if index == 0:
            x = self.spinBox_x_Linecut.value()
            y = self.spinBox_y_Linecut.value()
            angle = self.spinBox_Angle_Linecut.value()
            self.previous_window.linecut.setPos([x, y])
            self.previous_window.linecut.movePoint(self.previous_window.linecut.getHandles()[0], [x, y])
            self.previous_window.linecut.setAngle(angle, snap=True)
        elif index == 1:
            x1 = self.spinBox_x1_Linecut.value()
            y1 = self.spinBox_y1_Linecut.value()
            x2 = self.spinBox_x2_Linecut.value()
            y2 = self.spinBox_y2_Linecut.value()
            self.previous_window.linecut.movePoint(self.previous_window.linecut.getHandles()[0], [x1, y1], finish=False)
            self.previous_window.linecut.movePoint(self.previous_window.linecut.getHandles()[1], [x2 + x1, y2 + y1])
            self.previous_window.linecut.setPos([x1, y1])
        elif index == 2:
            if self.linecut_option == 0:
                x = int(self.previous_window.linecut.pos()[0])
                y = int(self.previous_window.linecut.pos()[1])
                angle = self.previous_window.linecut.angle()
                self.spinBox_x_Linecut.setValue(x)
                self.spinBox_y_Linecut.setValue(y)
                self.spinBox_Angle_Linecut.setValue(angle)
            elif self.linecut_option == 1:
                x = int(self.previous_window.linecut.pos()[0])
                y = int(self.previous_window.linecut.pos()[1])
                x1 = int(self.previous_window.linecut.getHandles()[0].pos()[0])
                y1 = int(self.previous_window.linecut.getHandles()[0].pos()[1])
                if len(self.previous_window.linecut.handles) > 1:
                    x2 = int(self.previous_window.linecut.getHandles()[1].pos()[0])
                    y2 = int(self.previous_window.linecut.getHandles()[1].pos()[1])
                    self.spinBox_x1_Linecut.setValue(x)
                    self.spinBox_y1_Linecut.setValue(y)
                    self.spinBox_x2_Linecut.setValue(x2)
                    self.spinBox_y2_Linecut.setValue(y2)

    # Processing | connect spinBox and slider
    def slider2spin(self, index, bits):
        if index == 2:  # line cut angle
            self.spinBox_Angle_Linecut.setValue(round(bits / 100, 2))

    # Processing | connect spinBox and slider
    def spin2slider(self, index):
        if index == 2:  # line cut angle
            value = int(self.spinBox_Angle_Linecut.value() * 100)
            self.slider_Angle_Linecut.setValue(value)

    # Processing | export line cut data
    def export_linecut(self):
        ret = self.previous_window.linecut.getArrayRegion(self.previous_window.raw_img, self.previous_window.img_display, axes=(0, 1), returnMappedCoords=True)
        default_name = "../data/" + self.previous_window.windowTitle()[:8] + "_linecut"
        fileName, ok = QFileDialog.getSaveFileName(self, "Save", default_name, "DAT Files(*.dat)")
        np.savetxt(fileName, ret[0])

    def open_3D(self):
        ## Create a GL View widget to display data
        w = gl.GLViewWidget()
        w.show()
        w.setWindowTitle('pyqtgraph example: GLSurfacePlot')
        w.setCameraPosition(distance=100)

        ## Add a grid to the view
        g = gl.GLGridItem()
        g.scale(5, 5, 1)
        # mean = np.average(self.previous_window.raw_img)
        g.setDepthValue(10)  # draw grid after surfaces since they may be translucent
        # w.addItem(g)

        print(self.previous_window.raw_img.shape[:])
        p1 = gl.GLSurfacePlotItem(z=self.previous_window.current_img, shader='shaded', color=(0.5, 0.5, 1, 1))
        w.addItem(p1)
        # p1.translate(-200, -200, -100)
        # g.setParentItem(p1)



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myImageProcessing()
    window.show()
    sys.exit(app.exec_())
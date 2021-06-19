# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:20:55 2020

@author: yaoji
"""

import sys
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication , QWidget
from PyQt5.QtCore import pyqtSignal , Qt
from DepositionInfo_ui import Ui_DepositionInfo
import conversion as cnv


class myDepositionInfo(QWidget, Ui_DepositionInfo):
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)

    def init_UI(self):
        pass
    
    def init_depInfo(self, data):

        # Title
        file_name = data.path
        self.setWindowTitle('Deposition Data Info --- ' + str(file_name))

        # Date and Time
        self.label_date.setText(str(data.time)[0:10])
        self.label_time.setText(str(data.time)[11:19])

        # Status
        bias_dac = '20bit DAC' if data.bias_dac else '16bit DAC'
        self.label_bias_dac.setText(bias_dac)
        bias = cnv.bv(data.lastdac[13], 'd', data.dacrange[13])
        self.label_bias.setText(str(bias))
        bias_dither = 'ON' if data.lastdigital[0] == 1 else 'OFF'
        self.label_bias_dither.setText(bias_dither)
        z_dither = 'ON' if data.lastdigital[1] == 1 else 'OFF'
        self.label_zdither.setText(z_dither)
        feedback = 'ON' if data.lastdigital[2] == 1 else 'OFF'
        self.label_feedback.setText(feedback)
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
        self.label_zgain1.setText(str(z1_gain))
        z2_gain = '10.0' if data.lastgain[3] == 3 else ('1.0' if data.lastgain[3] == 1 else '0.1')
        self.label_zgain2.setText(str(z2_gain))
        pream_gain = data.preamp_gain
        self.label_pream_gain.setText(str(pream_gain))
        dac_range_dict = {0: '0 to 5V', 1: '0 to 10V', 2: '0 to 20V', 4: '0 to 40V', 9: '-V to +5V', 10: '-10V to +10V',
                          12: '-20V to +20V', 14: '-2.5V to +2.5V'}
        bias_range = dac_range_dict[data.dacrange[13]]
        self.label_bias_range.setText(str(bias_range))

        # Sequence
        self.label_seq_name.setText(data.seq.name)
        feedback_seq = 'ON' if data.seq.feedback else 'OFF'
        self.label_feedback_seq.setText(feedback_seq)
        bias_dither_seq = 'ON' if data.seq.ditherB else 'OFF'
        self.label_bias_dither_seq.setText(bias_dither_seq)
        z_dither_seq = 'ON' if data.seq.ditherZ else 'OFF'
        self.label_z_dither_seq.setText(z_dither_seq)


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
        seq_description = []    # Init str description list
        for i in range(data.seq.seq_num):
            if data.seq.command[i] == 'Wait':
                seq_description += [str(i+1) + '  Wait ' + data.seq.data[i] + ' us']
            elif data.seq.command[i] == 'Match':
                option = '  Fast' if data.seq.option1[i] else '  Slow'
                seq_description += [str(i+1) + option + ' Match current']
            elif data.seq.command[i] == 'Dout':
                status = 'ON' if data.seq.data[i] else 'OFF'
                if data.seq.channel[i] == 'DitherZ':
                    seq_description += [str(i+1) + '  Z dither ' + status]
                elif data.seq.channel[i] == 'DitherB':
                    seq_description += [str(i+1) + '  Bias dither ' + status]
                elif data.seq.channel[i] == 'Feedback':
                    seq_description += [str(i+1) + '  Feedback ' + status]
            elif data.seq.command[i] == 'Shift':
                direction = ' Up' if data.seq.option1[i] else ' Down'
                if (data.seq.channel[i] == 'Z offset fine') or (data.seq.channel[i] == 'Z offset'):
                    seq_description += [str(i+1) + '  Shift ' + data.seq.channel[i] + direction + ' by ' + data.seq.data[i] + ' bits']
                elif data.seq.channel[i] == 'Iset':
                    seq_description += [str(i+1) + '  Shift ' + data.seq.channel[i] + direction + ' by ' + data.seq.data[i] + ' nA']
                else:
                    seq_description += [str(i+1) + '  Shift ' + data.seq.channel[i] + direction + ' by ' + data.seq.data[i] + ' volts']
            elif data.seq.command[i] == 'Aout':
                if data.seq.option1 == 1:
                    seq_description += [str(i+1) + '  Analog Output ' + data.seq.channel[i] + ' to Original']
                else:
                    if (data.seq.channel[i] == 'Z offset fine') or (data.seq.channel[i] == 'Z offset'):
                        seq_description += [str(i+1) + '  Analog Output ' + data.seq.channel[i] + ' to ' + data.seq.data[i] + ' bits']
                    elif data.seq.channel[i] == 'Iset':
                        seq_description += [str(i+1) + '  Analog Output ' + data.seq.channel[i] + ' to ' + data.seq.data[i] + ' nA']
                    else:
                        seq_description += [str(i+1) + '  Analog Output ' + data.seq.channel[i] + ' to ' + data.seq.data[i] + ' volts']
            elif data.seq.command[i] == 'Ramp':
                if data.seq.option1[i] == 1:
                    seq_description += [str(i+1) + '  Ramp ' + data.seq.channel[i] + ' to Original with speed ' + str(float(data.seq.option2[i])/10.0) + ' bits/ms']
                else:
                    if (data.seq.channel[i] == 'Z offset fine') or (data.seq.channel[i] =='Z offset'):
                        seq_description += [str(i+1) + '  Ramp ' + data.seq.channel[i] + ' to ' + data.seq.data[i] + ' bits with speed ' + str(float(data.seq.option2[i])/10.0) + ' bits/ms']
                    elif data.seq.channel[i] == 'Iset':
                        seq_description += [str(i+1) + '  Ramp ' + data.seq.channel[i] + ' to ' + data.seq.data[i] + ' nA with speed ' + str(float(data.seq.option2[i])/10.0) + ' bits/ms']
                    else:
                        seq_description += [str(i+1) + '  Ramp ' + data.seq.channel[i] + ' to ' + data.seq.data[i] + ' volts with speed ' + str(float(data.seq.option2[i])/10.0) + ' bits/ms']
            elif data.seq.command[i] == 'Read':
                seq_description += [str(i+1) + '  Read ' + data.seq.channel[i] + data.seq.data[i] + ' times']
            elif data.seq.command[i] == 'ShiftRamp':
                if (data.seq.channel[i] == 'Z offset fine') or (data.seq.channel[i] == 'Z offset'):
                    seq_description += [str(i+1) + '  ShiftRamp ' + data.seq.channel[i] + ' by ' + data.seq.data[i] + ' bits with speed ' + str(float(data.seq.option2[i])/10.0) + ' bits/ms']
                elif data.seq.channel[i] == 'Iset':
                    seq_description += [str(i+1) + '  ShiftRamp ' + data.seq.channel[i] + ' by ' + data.seq.data[i] + ' nA with speed ' + str(float(data.seq.option2[i])/10.0) + ' bits/ms']
                else:
                    seq_description += [str(i+1) + '  ShiftRamp ' + data.seq.channel[i] + ' by ' + data.seq.data[i] + ' volts with speed ' + str(float(data.seq.option2[i])/10.0) + ' bits/ms']

            self.listWidget_seq.addItem(seq_description[i])

        # Read
        self.label_read_ch.setText(str(data.read_ch))
        self.label_read_mode.setText(str(data.read_mode))
        self.label_avg_num.setText(str(data.read_avg))
        self.label_read_delay.setText(str(data.read_delay))
        self.label_sampling_delay.setText(str(data.read_delay2))
    
    # Emit close signal
    def closeEvent(self, event):
        self.close_signal.emit()
        event.accept()
        
    def enable_serial(self, enable):
        pass



if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myDepositionInfo()
    window.show()
    sys.exit(app.exec_())
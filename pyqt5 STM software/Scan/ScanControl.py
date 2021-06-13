# -*- coding: utf-8 -*-
"""
Created on Thu Dec  3 11:11:38 2020

@author: yaoji
"""

import sys

sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Model/")

from MainMenu import myMainMenu
from DataStruct import ScanData, DepData, SpcData, STMData
from sequence import mySequence
import threading
import copy
import numpy as np
import time


class myScanControl(myMainMenu):

    # Scan initial operation
    def enter_scan(self):
        if self.idling:
            self.idling = False  # Toggle idling flag mode to Falser
            self.enable_mode_serial(False)  # Disable serial based on current mode
            self.dsp.rampTo(0x1a, 0x8000, 2, 1000, 0, False)  # Ramp Zouter to 0V
            self.dsp.rampDiag(0x10, 0x1f, 0x8000, 0x8000, 2, 1000, 0, False)  # Ramp XY in to 0V
            self.dsp.rampDiag(0x11, 0x1e, 0x8000, 0x8000, 2, 1000, 0, False)  # Ramp XY offset to 0V
            self.dsp.gain(0, 1)  # Change X gain to 1.0
            self.dsp.gain(1, 1)  # Change Y gain to 1.0
            self.dsp.digital_o(4, 0)  # Change Zouter to fine mode
            self.dsp.digital_o(5, 1)  # Change XY to translation mode
            self.enable_mode_serial(True)  # Enable serial based on current mode
            self.idling = True  # Toggle idling flag back
            self.init_dock()  # Reload all 3 dock view
            self.enable_mode_serial(True)  # Enable serial based on current mode

    # Exit scan operation
    def exit_scan(self):
        if self.scan.idling:
            self.enable_mode_serial(False)  # Disable serial based on current mode
            self.scan.idling = False  # Toggle scan idling flag
            # !!! May stuck in endless loop
            # while not self.idling:                                              # Wait until all docks are idling
            #     time.sleep(1)
            self.dsp.rampTo(0x1a, 0x8000, 2, 1000, 0, False)  # Ramp Zouter to 0V
            self.dsp.rampDiag(0x10, 0x1f, 0x8000, 0x8000, 2, 1000, 0, False)  # Ramp XY in to 0V
            self.dsp.rampDiag(0x11, 0x1e, 0x8000, 0x8000, 2, 1000, 0, False)  # Ramp XY offset to 0V
            self.dsp.gain(0, 3)  # Change X gain to 10.0
            self.dsp.gain(1, 3)  # Change Y gain to 10.0
            self.dsp.digital_o(4, 1)  # Change Zouter to coarse mode
            self.dsp.digital_o(5, 0)  # Change XY to rotation mode
            self.dsp.rampTo(0x12, 0x8000, 100, 1000, 0, False)  # Return Z offset fine to zero
            # !!! Avoid stuck for no scanner testing
            # self.dsp.zAuto0()                                                   # Command DSP to do Z auto
            bits = self.dsp.lastdac[3] + 300  # Adjust Z feedback to be little bit contracted
            self.dsp.rampTo(0x13, bits, 1, 500, 0, False)  # Execute the adjustment
            time.sleep(1)  # Wait 1 seconds to wait for feedback Z to respond
            self.dsp.gain(3, 3)  # Change Z gain 2 to 10.0
            self.dsp.gain(3, 3)  # Change Z gain 1 to 0.1
            self.dsp.rampTo(0x13, 0x8000, 2, 1000, 0, False)  # Ramp Z offset to 0V

            self.scan.idling = True  # Restore scan idling flag
            self.enable_mode_serial(True)  # Enable serial based on current mode

    # Scan related stop slot
    def scan_stop(self):
        if not self.scan.stop:  # Avoid toggle dsp stop flag twice
            self.scan.stop = True
            self.dsp.stop = True

    # Sequence list close signal
    def close_seq_list(self, index):
        # Dictionaries based on different mode
        list_dict = {0: self.scan.scan_seq_list, 1: self.scan.dep_seq_list, 2: self.scan.spc_seq_list}
        select_dict = {0: self.scan.scan_seq_selected, 1: self.scan.dep_seq_selected, 2: self.scan.spc_seq_selected}
        label_dict = {0: self.scan.label_Seq_ScanControl, 1: self.scan.dep.label_Seq_Deposition,
                      2: self.scan.spc.adv.label_Seq_AdvOption}

        # Determine name
        name = '' if self.scan.seq_list.selected < 0 else self.scan.seq_list.seqlist[self.scan.seq_list.selected].name

        select_dict[index] = self.scan.seq_list.selected  # Load selected sequence
        label_dict[index].setText(name)  # Set corresponding label
        list_dict[index] = []  # Empty corresponding sequence list

        # Load sequence list
        for seq in self.scan.seq_list.seqlist:
            list_dict[index] += [copy.deepcopy(seq)]

    # Open sequence list window
    def open_seq_list(self, index, selected_name):
        # Sequence list dictionary
        list_dict = {0: self.scan.scan_seq_list, 1: self.scan.dep_seq_list, 2: self.scan.spc_seq_list}
        # Init sequence list window
        self.scan.seq_list.init_seqlist(index, list_dict[index], selected_name, index != 1, \
                                        self.bias_dac, self.preamp_gain, self.dsp.dacrange, self.dsp.lastdac,
                                        self.dsp.last20bit)
        # Open sequence list window
        self.scan.seq_list.show()

    # Send execution
    def send_excu(self, index, xin, yin, xoff, yoff):
        if self.scan.idling:
            self.enable_mode_serial(False)  # Disable serial based on current mode
            self.scan.idling = False  # Toggle scan idling flag
            self.scan.stop = False  # Toggle stop flag
            delay, step, limit = self.scan.send_options.configure_send()  # Configure send options
            xygain = self.scan.imagine_gain  # Get imagine gain data
            if index and (not self.scan.stop):  # Send
                self.scan.pushButton_Send_XY.setText("Stop")  # Change button label
                self.scan.pushButton_Send_XY.setEnabled(True)  # Enable corresponding label
                self.dsp.rampDiag(0x11, 0x1e, xoff, yoff, step, delay, limit, True)  # Send XY offset
            elif not (index or self.scan.stop):  # Zero
                self.scan.pushButton_Zero_XY.setText("Stop")  # Change button label
                self.scan.pushButton_Zero_XY.setEnabled(True)  # Enable corresponding label
            if not self.scan.stop:
                self.dsp.rampDiag(0x10, 0x1f, xin, yin, int(step * 100 / xygain), delay, limit, True)  # Send XY in
            self.scan.stop = True  # Restore scan stop flag
            self.scan.idling = True  # Restore scan idling flag
            self.scan.pushButton_Zero_XY.setText("Zero")  # Restore zero button text
            self.scan.pushButton_Send_XY.setText("Send")  # Restore send button text
            self.init_dock()  # Reload all 3 dock view
            self.enable_mode_serial(True)  # Enable serial based on current mode

    # Send signal slot
    def send_thread(self, index, xin, yin, xoff, yoff):
        threading.Thread(target=(lambda: self.send_excu(index, xin, yin, xoff, yoff))).start()

    # Scan execution
    def scan_excu(self, xin, yin, xoff, yoff, xygain, step_num, step_size, seq):
        delay, step_off, _ = self.scan.send_options.configure_send()  # Configure send options
        channel_x, channel_y, dir_x = self.scan.scan_options.configure_scan()  # Configure scan direction
        move_delay, measure_delay, line_delay = self.scan.scan_options.configure_delay(
            seq.feedback)  # Configure scan delay
        scan_protect_flag, limit = self.scan.scan_options.configure_scan_protect()  # Configure scan protection
        tip_protection, tip_protect_data = self.scan.scan_options.configure_tip_protect(seq)  # Configure tip protection
        match_curr, advance_bit = self.scan.scan_options.configure_prescan(seq.feedback, self.dsp)  # Configure prescan

        step_in = int(step_off * 100 / xygain)  # Calculate send step
        y_pos = 0x8000 - int((step_num - 1) * step_size / 2)  # Calculate scan start position line
        x_pos = y_pos if dir_x else (
                0x8000 + int((step_num - 1) * step_size / 2))  # Calculate scan start position point

        if self.scan.idling:
            # Re-init scan data and load options
            self.scan.data = ScanData()
            self.scan.data.load_status(self.dsp, self.preamp_gain, self.bias_dac, seq)
            self.scan.data.load(step_num, step_size, channel_x, channel_y, dir_x, move_delay, measure_delay, line_delay, \
                                scan_protect_flag, limit, tip_protection, tip_protect_data, match_curr, advance_bit)
            if self.scan.checkBox_LockIn_ScanControl.isEnabled():
                self.scan.pushButton_LockIn_ScanControl.setEnabled(True)  # Load lock in
            self.scan.saved = False  # Set saved flag to false
            print('data finish loading')

            # Get system ready
            self.scan.setWindowTitle('Scan')  # Set window title to indicate status
            self.enable_mode_serial(False)  # Disable serial based on current mode
            self.scan.idling = False  # Toggle scan idling flag
            self.scan.stop = False  # Toggle stop flag
            self.dsp.rampDiag(1 + 16, 14 + 16, xoff, yoff, step_off, delay, 0,
                              True)  # Send XY offset without crash protection
            self.dsp.rampDiag(0 + 16, 15 + 16, xin, yin, step_in, delay, 0, True)  # Send XY in without crash protection
            print('finish ramp to reference')

            # Store system status for later restore
            ditherB_s = self.dsp.lastdigital[0]  # Store bias dither
            ditherZ_s = self.dsp.lastdigital[1]  # store Z dither
            feedback_s = self.dsp.lastdigital[2]  # Store feedback
            zofff_s = self.dsp.lastdac[2]  # Store Z offset fine

            # Handle pre-scan
            self.dsp.digital_o(2, seq.feedback)  # Turn feedback ON/OFF
            if match_curr:
                self.dsp.iAuto()  # Match current if needed
            self.dsp.rampTo(0x12, max(0, min(self.dsp.lastdac[2] + advance_bit, 0xffff)), 100, 10, 0, False)  # Advance bits
            self.dsp.digital_o(0, seq.ditherB)  # Turn bias dither ON/OFF
            self.dsp.digital_o(1, seq.ditherZ)  # Turn Z dither ON/OFF
            print('finish prescan setup')

            # Send tip to start scan start position
            if tip_protection:
                # self.dsp.tipProtect(tip_protect_data, False)  # Tip protect
                print('finish tip protection')
            self.dsp.rampDiag(channel_x, channel_y, x_pos, y_pos, step_in, delay, 0, False)  # Send XY in
            print('finish ramp to start')

            # Enable stop button
            self.scan.pushButton_Start_Scan.setText('Stop')
            self.scan.pushButton_Start_Scan.setEnabled(True)

            # Start scan
            self.dsp.scan(channel_x, channel_y, step_size, step_num, move_delay, measure_delay, line_delay, \
                          limit, tip_protect_data, seq, scan_protect_flag, tip_protection, dir_x)
            print('finish scan')


            # Send XY in back to original
            self.dsp.rampDiag(0 + 16, 15 + 16, xin, yin, step_in, delay, 0,
                              False)  # Send XY in back to original place
            print('finish ramp back to start')
            if tip_protection:
                # self.dsp.tipProtect(tip_protect_data, True)  # Tip unprotect
                print('finish tip unprotect')

            # Restore pre-scan
            self.dsp.digital_o(1, ditherZ_s)  # Restore Z dither
            self.dsp.digital_o(0, ditherB_s)  # Restore bias dither
            self.dsp.rampTo(0x12, zofff_s, 100, 10, 0, False)  # Restore Z offset fine
            self.dsp.digital_o(2, feedback_s)  # Restore feedback

            # Restore system status
            self.scan.pushButton_Start_Scan.setText('Start')  # Restore scan button text
            self.scan.stop = True  # Restore scan stop flag
            self.scan.idling = True  # Restore scan idling flag
            self.init_dock()  # Reload all 3 dock view
            self.enable_mode_serial(True)  # Enable serial based on current mode
            self.scan.pushButton_Info_Scan.setEnabled(True)  # Set Scan Info button enabled
            print('finish finish')

    # Scan signal slot
    def scan_thread(self, xin, yin, xoff, yoff, xygain, step_num, step_size):
        # Set up sequence
        if self.scan.scan_seq_selected < 0:  # No sequence selected
            flag = False  # Cannot execute scan
            self.scan.message('No sequence selected')  # Pop out message
        else:  # One sequence selected
            seq = self.scan.scan_seq_list[self.scan.scan_seq_selected]  # Get sequence
            seq.configure(self.bias_dac, self.preamp_gain, self.dsp.dacrange, self.dsp.lastdac,
                          self.dsp.last20bit)  # Configure sequence
            seq.validation(seq.ditherB, seq.ditherZ, seq.feedback, True)  # Validate sequence
            flag = seq.validated or (not seq.validation_required)  # Scan executable flag
            if not flag:
                self.scan.message('Selected sequence is not valid')  # Pop out message

        # Execute scan if executable
        if flag:
            threading.Thread(
                target=(lambda: self.scan_excu(xin, yin, xoff, yoff, xygain, step_num, step_size, seq))).start()

    # Deposition execution
    def deposition_excu(self, read_before, read, seq):
        if self.scan.idling:
            # Re-init deposition data and load options
            self.scan.dep.data = DepData()
            self.scan.dep.data.load_status(self.dsp, self.preamp_gain, self.bias_dac, seq)
            self.scan.dep.data.load(read)
            self.scan.dep.saved = (read[1] == 0)  # No need to save if not reading

            # Get system ready
            self.scan.dep.setWindowTitle('Deposition')  # Set window title to indicate status
            self.enable_mode_serial(False)  # Disable serial based on current mode
            self.scan.idling = False  # Toggle scan idling flag
            self.scan.dep.idling = False  # Toggle deposition idling flag
            self.scan.stop = False  # Toggle stop flag

            # Execute read before
            if read_before:
                rdata = self.dsp.osc_N(read_before[0], read_before[1], read_before[2], read_before[3])
                ch_range = self.dsp.adcrange[self.scan.dep.comboBox_Ch_Read.currentIndex() + 6]
                rdata_volt = self.scan.dep.cnv2volt(rdata, ch_range)
                self.scan.dep.update_N(rdata_volt, 0)  # Plot read before data

            # Set up stop button if continuous read mode
            if read[1] == 1:
                self.scan.dep.pushButton_DoIt_Deposition.setText('Stop')
                self.scan.dep.pushButton_DoIt_Deposition.setEnabled(True)

            # Execute deposition
            rdata = self.dsp.deposition(read[0], read[1], read[2], read[3], read[4], read[5], read[6], read[7], seq)

            # Load data
            if read[1] == 1:  # Continuous mode
                self.scan.dep.data.data = np.array(self.scan.dep.rdata)  # Load data for storage
            if (read[1] == 2) or (read[1] == 3):  # N sample mode
                self.scan.dep.data.data = np.array(rdata)  # Load data for storage
                ch_range = self.dsp.adcrange[self.scan.dep.comboBox_Ch_Read.currentIndex() + 6]
                rdata_volt = self.scan.dep.cnv2volt(rdata, ch_range)
                self.scan.dep.update_N(rdata_volt, 1)  # Plot N sample data

            # Execute read after
            if read_before:
                rdata = self.dsp.osc_N(read_before[0], read_before[1], read_before[2], read_before[3])
                ch_range = self.dsp.adcrange[self.scan.dep.comboBox_Ch_Read.currentIndex() + 6]
                rdata_volt = self.scan.dep.cnv2volt(rdata, ch_range)
                self.scan.dep.update_N(rdata_volt, 2)  # Plot read after data

            # Restore system status
            self.scan.dep.pushButton_DoIt_Deposition.setText('Do it')  # Restore do it button text
            self.scan.stop = True  # Restore scan stop flag
            self.scan.idling = True  # Restore scan idling flag
            self.scan.dep.idling = True  # Restore deposition idling flag
            self.init_dock()  # Reload all 3 dock view
            self.enable_mode_serial(True)  # Enable serial based on current mode
            self.scan.dep.pushButton_Info_Deposition.setEnabled(True)  # Set Deposition Info button enabled

    # Deposition signal slot
    def deposition_thread(self, read_before, read, poke_data):
        # Set up sequence
        if (self.scan.dep_seq_selected < 0) and (not poke_data):  # Use sequence and no sequence selected
            self.scan.dep.message('No sequence selected')  # Pop out message
            flag = False  # Cannot execute deposition
        else:
            # Set up sequence
            poke_command = [0x42, 0x63, 0x8d]  # Sequence command list [Feedback, Shift Z offset, Aout bias]
            seq = mySequence(poke_command, poke_data, False) if poke_data else self.scan.dep_seq_list[
                self.scan.dep_seq_selected]  # Get sequence
            seq.configure(self.bias_dac, self.preamp_gain, self.dsp.dacrange, self.dsp.lastdac,
                          self.dsp.last20bit)  # Configure sequence
            seq.validation(self.dsp.lastdigital[0], self.dsp.lastdigital[1], self.dsp.lastdigital[2],
                           False)  # Validate sequence
            flag = seq.validated or (not seq.validation_required)  # Deposition executable flag
            if not flag:
                self.scan.dep.message('Selected sequence is not valid')  # Pop out message

        # Execute deposition if executable
        if flag:
            threading.Thread(target=(lambda: self.deposition_excu(read_before, read, seq))).start()

    # Track execution
    def track_excu(self, track, loop_num):
        if self.scan.idling or loop_num:
            # Get system ready
            if not loop_num:
                self.enable_mode_serial(False)  # Disable serial based on current mode
                self.scan.idling = False  # Toggle scan idling flag
                self.scan.stop = False  # Toggle stop flag

                # Set up stop button
                self.scan.track.pushButton_Start_Track.setText('Stop')
                self.scan.track.pushButton_Start_Track.setEnabled(True)

                # Do track
                self.track_excu_(track, loop_num)

            if not loop_num:
                self.scan.track.pushButton_Start_Track.setText('Start')  # Restore track start button text
                self.enable_mode_serial(True)  # Enable serial based on current mode
                self.scan.stop = True  # Restore scan stop flag
                self.scan.idling = True  # Restore scan idling flag

    # Track execution called by other function
    def track_excu_(self, track, loop_num):
        self.scan.track.idling = False  # Toggle track idling flag

        # Load track XY variables for out of boundary judgement
        self.scan.track.x = self.dsp.lastdac[0]
        self.scan.track.y = self.dsp.lastdac[15]

        # Execute track
        print('track start')
        x, y = self.dsp.track(loop_num, track[0], track[1], track[2], track[3], track[4], track[5], track[6], track[7])
        self.scan.send_update(0x10, 0x1f, x, y)  # Update scan view based on returned XY value
        print('track end')
        # Restore system status
        self.scan.track.idling = True

    # Track slot
    def track_thread(self):
        track = self.scan.track.configure_track()
        threading.Thread(target=(lambda: self.track_excu(track, 0))).start()

    # !!! Spectroscopy execution
    def spectroscopy_excu(self, seq):
        # Configure spectroscopy options
        start, step, data_num, pass_num, ramp_ch, delta_ch, delta_data = self.scan.spc.configure_spc(self.dsp)
        # Configure spectroscopy delays
        move_delay, measure_delay = self.scan.spc.adv.configure_delay()
        # Configure spectroscopy correction options
        corr_pass_num, z_flag, match_flag, feedback_delay, track_flag = self.scan.spc.adv.configure_correction(
            seq.feedback)
        # Configure spectroscopy scan options
        forward, backward, average = self.scan.spc.adv.configure_scan()
        # Configure track
        track = self.scan.track.configure_track()

        # Unimplemented functions
        # Configure send
        # Use default send_delay = 2000us, send_step = 100bits, send_limit = 0
        # send_delay, send_step, send_limit = self.scan.send_options.configure_send()
        # Configure scan for rescan
        # Configure spectroscopy rescan options
        rescan = self.scan.spc.adv.configure_rescan

        if self.scan.idling:
            # Re-init spectroscopy data and load options
            self.scan.spc.saved = False  # Set saved flag to false
            self.scan.spc.data = SpcData()
            self.scan.spc.data.load_status(self.dsp, self.preamp_gain, self.bias_dac, seq)
            self.scan.spc.data.load(start, step, data_num, ramp_ch, delta_data, move_delay, measure_delay,
                                    forward, backward, average, corr_pass_num, z_flag, match_flag, feedback_delay,
                                    track_flag, rescan, self.scan.data, self.scan.point_list, self.scan.pattern)

            # Get system ready
            self.scan.spc.setWindowTitle('Spectroscopy')  # Set window title to indicate status
            self.enable_mode_serial(False)  # Disable serial based on current mode
            self.scan.idling = False  # Toggle scan idling flag
            self.scan.spc.idling = False  # Toggle spectroscopy idling flag
            self.scan.stop = False  # Toggle stop flag

            # Set up stop button
            self.scan.spc.pushButton_Scan.setText('Stop')
            self.scan.spc.pushButton_Scan.setEnabled(True)

            # Store system status for later restore
            ref_ditherB = self.dsp.lastdigital[0]  # Store bias dither
            ref_ditherZ = self.dsp.lastdigital[1]  # store Z dither
            ref_feedback = self.dsp.lastdigital[2]  # Store feedback
            ref_xy = (self.dsp.lastdac[0], self.dsp.lastdac[15])
            ref_z = self.dsp.lastdac[2]
            ref_b = self.dsp.last20bit if self.bias_dac else self.dsp.lastdac[13]
            ref_delta = ref_z if delta_ch == 0x12 else ref_b
            ref_ramp = ref_z if ramp_ch == 0x12 else ref_b
            delta_xy = (0, 0)

            # Do feedback and dither
            self.dsp.digital_o(2, seq.feedback)  # Turn feedback ON/OFF
            time.sleep(0.1)  # Wait for reed relay
            self.dsp.digital_o(0, seq.ditherB)  # Turn bias dither ON/OFF
            time.sleep(0.01)  # Wait for analog switch
            self.dsp.digital_o(1, seq.ditherZ)  # Turn Z dither ON/OFF
            time.sleep(0.01)  # Wait for analog switch

            corr_flag = True  # Initiate corr_flag
            # Passes
            for i in range(pass_num):
                print('pass' + str(i) + 'started')
                for j in self.scan.point_list:
                    print('point')
                    print(j)
                    # Send to point + delta
                    position = (j[0] + delta_xy[0], j[1] + delta_xy[1])
                    self.dsp.rampDiag(0x10, 0x1f, position[0], position[1], 100, 2000, 0, True)
                    if self.scan.stop:
                        break

                    # First pass or restore from correction or multi-pont, go to initial set point
                    if (len(self.scan.point_list) != 1) or corr_flag:
                        # Ramp to initial
                        self.dsp.rampTo(ramp_ch, start, 100, move_delay, 0, False)
                        if self.scan.stop:
                            break
                        # Do advance
                        self.dsp.rampTo(delta_ch, max(0, min(ref_delta + delta_data, 0xffff)), 100, 10, 0, False)
                        if self.scan.stop:
                            break

                    # Forward pass
                    time.sleep(0.5)
                    if forward:
                        self.dsp.rampMeasure(ramp_ch, data_num, step, move_delay, measure_delay, True, seq)
                        if self.scan.stop:
                            break
                    else:
                        self.dsp.rampTo(ramp_ch, start + (data_num * step), 100, move_delay, 0, False)
                        if self.scan.stop:
                            break

                    # Backward pass
                    time.sleep(0.5)
                    if backward:
                        self.dsp.rampMeasure(ramp_ch, data_num, step, move_delay, measure_delay, False, seq)
                        if self.scan.stop:
                            break
                    else:
                        self.dsp.rampTo(ramp_ch, start, 100, move_delay, 0, False)
                        if self.scan.stop:
                            break

                    # Multipoint go to reference set point
                    if len(self.scan.point_list) != 1:
                        # Undo advance
                        self.dsp.rampTo(delta_ch, ref_delta, 100, 10, 0, False)
                        if self.scan.stop:
                            break
                        # Ramp to reference set point
                        self.dsp.rampTo(ramp_ch, ref_ramp, 100, move_delay, 0, False)
                        if self.scan.stop:
                            break
                if self.scan.stop:
                    break
                else:
                    self.scan.spc.update_spc_(i)     # Update averaged data
                # Rescan
                # Send back to reference point
                # Restore scan dither
                # Restore scan Z
                # Restore scan feedback
                # Rescan
                # Update XY offset and XY in
                # Send
                # Sequence feedback
                # Advance bit
                # Sequence dither

                # Correction
                # Figure out if doing correction this pass
                corr_flag = (z_flag or track_flag) and ((i % corr_pass_num) == (corr_pass_num - 1))
                if corr_flag:
                    # Undo dither
                    self.dsp.digital_o(0, ref_ditherB)  # Turn bias dither ON/OFF
                    time.sleep(1)  # Wait for analog switch
                    self.dsp.digital_o(1, ref_ditherZ)  # Turn Z dither ON/OFF
                    time.sleep(1)  # Wait for analog switch
                    if self.scan.stop:
                        break

                    # Undo advance
                    self.dsp.rampTo(delta_ch, ref_delta, 100, 10, 0, False)
                    if self.scan.stop:
                        break
                    # Ramp back to reference set point
                    self.dsp.rampTo(ramp_ch, ref_ramp, 100, move_delay, 0, False)
                    if self.scan.stop:
                        break
                    # Send back to reference point
                    self.dsp.rampDiag(0x10, 0x1f, ref_xy[0], ref_xy[1], 100, 2000, 0, False)
                    if self.scan.stop:
                        break

                    # Undo feedback
                    if z_flag:
                        self.dsp.digital_o(2, ref_feedback)  # Turn feedback ON/OFF
                        time.sleep(1)  # Wait for reed relay
                    if self.scan.stop:
                        break

                    # Track (XY correction)
                    if track_flag:
                        self.track_excu_(track, 10)
                        # Update delta_xy
                        delta_xy = (self.dsp.lastdac[0] - ref_xy[0], self.dsp.lastdac[15] - ref_xy[1])
                        ref_xy = (self.dsp.lastdac[0], self.dsp.lastdac[15])
                        # Send to reference point
                        self.dsp.rampDiag(0x10, 0x1f, ref_xy[0], ref_xy[1], 100, 2000, 0, False)
                        print
                    if self.scan.stop:
                        break

                    # match current
                    if z_flag and match_flag:
                        self.dsp.iAuto()
                        # Update ref_z, ref_delta and ref_ramp
                        ref_z = self.dsp.lastdac[2]
                        ref_delta = ref_z if delta_ch == 0x12 else ref_b
                        ref_ramp = ref_z if ramp_ch == 0x12 else ref_b
                    if self.scan.stop:
                        break

                    # Redo dither, feedback
                    self.dsp.digital_o(2, seq.feedback)  # Turn feedback ON/OFF
                    time.sleep(1)  # Wait for reed relay
                    self.dsp.digital_o(0, seq.ditherB)  # Turn bias dither ON/OFF
                    time.sleep(1)  # Wait for analog switch
                    self.dsp.digital_o(1, seq.ditherZ)  # Turn Z dither ON/OFF
                    time.sleep(1)  # Wait for analog switch
                    if self.scan.stop:
                        break

            # Undo dither
            self.dsp.digital_o(0, ref_ditherB)  # Turn bias dither ON/OFF
            time.sleep(1)  # Wait for analog switch
            self.dsp.digital_o(1, ref_ditherZ)  # Turn Z dither ON/OFF
            time.sleep(1)  # Wait for analog switch
            # Undo advance
            self.dsp.rampTo(delta_ch, ref_delta, 100, 10, 0, False)
            # Ramp back to reference set point
            self.dsp.rampTo(ramp_ch, ref_ramp, 100, move_delay, 0, False)
            # Send back to reference point
            self.dsp.rampDiag(0x10, 0x1f, ref_xy[0], ref_xy[1], 100, 2000, 0, False)
            # Undo feedback
            self.dsp.digital_o(2, ref_feedback)  # Turn feedback ON/OFF
            time.sleep(1)  # Wait for reed relay

            # Restore system status
            self.scan.spc.pushButton_Scan.setText('Start')  # Restore scan button text
            self.enable_mode_serial(True)  # Enable serial based on current mode
            if track_flag:
                self.scan.track.pushButton_Start_Track.setEnabled(False)
            self.scan.stop = True  # Restore scan stop flag
            self.scan.idling = True  # Restore scan idling flag
            self.scan.spc.idling = True  # Restore spectroscopy idling flag
            self.scan.spc.pushButton_Info.setEnabled(True)  # Set Spectroscopy Info button enabled

            # Auto save
            if self.scan.spc.autosave_name:
                self.scan.spc.auto_save(self.scan.spc.autosave_name, 0)

    # Spectroscopy slot
    def spectroscopy_thread(self):
        command_list, data_list = self.scan.spc.adv.configure_measure()
        # Set up sequence
        if (self.scan.spc_seq_selected < 0) and (not command_list):  # Use sequence and no sequence selected
            flag = False  # Cannot execute scan
            self.scan.spc.message('No sequence selected')  # Pop out message
        else:  # One sequence selected
            # Get sequence
            if command_list:  # Not using sequence
                seq = mySequence(command_list, data_list, True)  # Generate sequence
                seq.feedback, seq.ditherB, seq.ditherZ = self.scan.spc.adv.configure_prescan()  # Load sequence digital
            else:  # Use sequence
                self.scan.spc_seq_list[self.scan.spc_seq_selected]  # Get selected sequence
            seq.configure(self.bias_dac, self.preamp_gain, self.dsp.dacrange, self.dsp.lastdac,
                          self.dsp.last20bit)  # Configure sequence
            seq.validation(seq.ditherB, seq.ditherZ, seq.feedback, True)  # Validate sequence
            flag = seq.validated or (not seq.validation_required)  # Scan executable flag
            if not flag:
                self.scan.spc.message('Selected sequence is not valid')  # Pop out message

        # Execute scan if executable
        if flag:
            threading.Thread(target=(lambda: self.spectroscopy_excu(seq))).start()

    # !!! Rescan function for spectroscopy
    def rescan(self):
        pass

# -*- coding: utf-8 -*-
"""
Created on Fri Jan 22 11:32:13 2021

@author: yaoji
"""
import sys
import numpy as np
sys.path.append("../Model/")
from DigitalSignalProcessor import myDSP
import conversion as cnv
from sequence import mySequence
import time
import serial
ser = serial.Serial('COM3', 9600)  # Open serial port
# time.sleep(2)
ser.flushInput()
v = 1
dither_on = 1
pot_ratio = 0.4



b = int((v+2.5) * (2**16)/5)
pot = int(pot_ratio * (2**8))
print(hex(b))
print(hex(pot))
# msb = (b & 0xff00) << 8
# lsb = b & 0x00ff
ser.write((0x01).to_bytes(1, byteorder='big'))
ser.write(int(b).to_bytes(2, byteorder='big'))


ser.write((0x03).to_bytes(1, byteorder='big'))
ser.write(int(dither_on).to_bytes(1, byteorder='big'))

ser.write((0x02).to_bytes(1, byteorder='big'))
ser.write(int(pot).to_bytes(1, byteorder='big'))
# print(hex(int.from_bytes(ser.read(1), "big")))

ser.write((0x04).to_bytes(1, byteorder='big'))
time.sleep(0.2)
print(ser.inWaiting())
# while(ser.inWaiting()):
#     print(hex(int.from_bytes(ser.read(1), "big")))
print(hex(int.from_bytes(ser.read(2), "big")))
print(hex(int.from_bytes(ser.read(1), "big")))
print(hex(int.from_bytes(ser.read(1), "big")))


# time.sleep(1)
#
# ser.write((0x03).to_bytes(1, byteorder='big'))
# data = int.from_bytes(ser.read(1), byteorder='big') << 8
# data = data | int.from_bytes(ser.read(1), byteorder='big')
# print(data)


# ser.write((0x05).to_bytes(1, byteorder='big'))
# print(hex(int.from_bytes(ser.read(1), byteorder='big')))
#
# ser.write((0x01).to_bytes(1, byteorder='big'))
# ser.write((0x57).to_bytes(1, byteorder='big'))
# ser.write((0x32).to_bytes(1, byteorder='big'))
# print(hex(int.from_bytes(ser.read(1), byteorder='big')))
# print(hex(int.from_bytes(ser.read(1), byteorder='big')))
#
# ser.write((0x09).to_bytes(1, byteorder='big'))
# print(hex(int.from_bytes(ser.read(1), byteorder='big')))
#
# ser.write((0x06).to_bytes(1, byteorder='big'))
# print(hex(int.from_bytes(ser.read(1), byteorder='big')))
#
# ser.write((0x03).to_bytes(1, byteorder='big'))
# print(hex(int.from_bytes(ser.read(1), byteorder='big')))
# i=5
# while i<10:
#     # time.sleep(0.1)
#     ser.write((i).to_bytes(1, byteorder='big'))
#     if ser.inWaiting() > 0:
#         print(hex(int.from_bytes(ser.read(1), byteorder='big')))
#     print(i)
#     i=i+1
# time.sleep(0.1)
# print(hex(int.from_bytes(ser.read(1), byteorder='big')))



ser.close()
# dsp = myDSP()

# Open COM
# dsp.port, dsp.baudrate = 'com4', 38400
# dsp.init_dsp(True)
# print(dsp.ver)

# =============================================================================
# # DAC output
# ch = 9
# volt = 0.0
# ran = 10
#
# dsp.dac_W(ch + 16, cnv.vb(volt, 'd', ran))
# =============================================================================
# Gain test
# dsp.dac_W(0x10, 0x8000)
# dsp.gain(0, 0)
# =============================================================================
# # ADC read
# ch = 4
# ran = 0
#
# bits = dsp.adc(ch * 4 + 0xc0)
# print(cnv.bv(bits, 'a', ran))

# =============================================================================
# OSC test
# ch = 4
# n = 1024
# average = 1
# delay = 100
# rdata = dsp.osc_N(ch * 4 + 0xc0, n, average, delay)
# np.savetxt("test data/OSC_N.csv", rdata, delimiter =", ", fmt ='% s')
# =============================================================================
# Verify lastdac
# dsp.lastdacAcq()
# print(dsp.lastdac)
# =============================================================================
# Ramp dialog
# chs, chl = 0, 15
# targets, targetl = 1000, 1000
# step = 100
# delay = 1
# limit = 0x7fff
# check = True

# currents, currentl = dsp.lastdac[chs], dsp.lastdac[chl]
# dsp.rampDiag(chs + 16, chl +16, targets + dsp.lastdac[chs], targetl + dsp.lastdac[chl], step, delay, limit, check)
# dsp.lastdacAcq()
# print(dsp.lastdac[chs] - currents, dsp.lastdac[chl] - currentl)
# =============================================================================
# poke_command = [0x42, 0x63, 0x8d]
# poke_data = [0, 0x80000000, 0x8000]
# seq = mySequence(poke_command, poke_data, False)
# rdata = dsp.deposition(0xc0, 3, 1000, 20, 20, 1, 0, 0, seq)
# dsp.ser.write(int(0xff).to_bytes(2, byteorder="big"))  # Send scan delay
# print(rdata)
# time.sleep(1)
# print(dsp.ser.inWaiting())
# while(dsp.ser.inWaiting()):
#     print(hex(int.from_bytes(dsp.ser.read(1), "big")))
# ch = 4
# ran = 0
#
# xx = dsp.adc(ch * 4 + 0xc0)
# print(cnv.bv(xx, 'a', ran))
# print(dsp.ser.inWaiting())
# while(dsp.ser.inWaiting()):
#     print(hex(int.from_bytes(dsp.ser.read(1), "big")))

# print(dsp.ok())
# dsp.version()
# print(dsp.ver)

# =============================================================================
# channel_x, channel_y = 0x19, 0x17
# step_size, step_num = 10, 3
# move_delay, measure_delay, line_delay = 10, 2000, 2
# limit, tip_protect_data = 0, 0
# seq = mySequence([0xdc], [500], True)
# scan_protect_flag, tip_protection, dir_x = 0, 0, 1
# dsp.scan(channel_x, channel_y, step_size, step_num, move_delay, measure_delay, line_delay,
#          limit, tip_protect_data, seq, scan_protect_flag, tip_protection, dir_x)
# =============================================================================

# =============================================================================

# =============================================================================


# =============================================================================








































# Close COM
# dsp.close()
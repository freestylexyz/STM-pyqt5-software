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
# ser = serial.Serial('com4', 38400)  # Open serial port
# ser.write((0xf2).to_bytes(1, byteorder='big'))  # 0x72 lastdigital_O command
# ser.close()
dsp = myDSP()

# Open COM
dsp.port, dsp.baudrate = 'com4', 38400
dsp.init_dsp(True)
print(dsp.ver)

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
poke_command = [0x42, 0x63, 0x8d]
poke_data = [0, 0x80000000, 0x8000]
seq = mySequence(poke_command, poke_data, False)
rdata = dsp.deposition(0xc0, 3, 1000, 20, 20, 1, 0, 0, seq)
# dsp.ser.write(int(0xff).to_bytes(2, byteorder="big"))  # Send scan delay
print(rdata)
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

# =============================================================================

# =============================================================================

# =============================================================================


# =============================================================================








































# Close COM
dsp.close()
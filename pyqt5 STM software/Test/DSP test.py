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


dsp = myDSP()

# Open COM
dsp.port, dsp.baudrate = 'com4', 38400
dsp.init_dsp(True)
print(dsp.ver)

# =============================================================================
# # DAC ouput
# ch = 9
# volt = 0.0
# ran = 10

# dsp.dac_W(ch + 16, cnv.vb(volt, 'd', ran))
# =============================================================================

# =============================================================================
# Gain test
dsp.dac_W(0x10, 0x8000)
dsp.gain(0, 0)
# =============================================================================

# =============================================================================
# # ADC read
# ch = 4
# ran = 0

# bits = dsp.adc(ch * 4 + 0xc0)
# print(cnv.bv(bits, 'a', ran))

# =============================================================================

# =============================================================================
# OSC test
# ch = 4
# n = 1024
# average = 1
# delay = 100
# rdata = dsp.osc_N(ch * 4 + 0xc0, n, average, delay)
# np.savetxt("test data/OSC_N.csv", rdata, delimiter =", ", fmt ='% s')
# =============================================================================

# =============================================================================
# Verify lastdac
# dsp.lastdacAcq()
# print(dsp.lastdac)
# =============================================================================

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








































# Close COM
dsp.close()
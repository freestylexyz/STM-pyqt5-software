# -*- coding: utf-8 -*-
"""
Created on Fri Jan 22 11:32:13 2021

@author: yaoji
"""

from DigitalSignalProcessor import myDSP
import atexit


dsp = myDSP()

# Open COM
dsp.port, dsp.baudrate = 'com5', 38400
dsp.init_dsp(True)
print(dsp.ver)

# Verify lastdac
dsp.lastdacAcq()
print(dsp.lastdac)

# Ramp dialog
chs, chl = 0, 15
targets, targetl = 1000, 1000
step = 100
delay = 1
limit = 0x7fff
check = True

currents, currentl = dsp.lastdac[chs], dsp.lastdac[chl]
dsp.rampDiag(chs + 16, chl +16, targets + dsp.lastdac[chs], targetl + dsp.lastdac[chl], step, delay, limit, check)
dsp.lastdacAcq()
print(dsp.lastdac[chs] - currents, dsp.lastdac[chl] - currentl)






































# Close COM
dsp.close()
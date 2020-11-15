# -*- coding: utf-8 -*-
"""
Created on Sun Nov  8 14:33:31 2020

@author: yaoji
"""

def b2v(b, bit_num = 16, lower_limit = -10, upper_limit = 10):
    return ((upper_limit - lower_limit) * b / (2 ** bit_num)) + lower_limit

def v2b(v, bit_num = 16, lower_limit = -10, upper_limit = 10):
    return int((v - lower_limit) / (upper_limit - lower_limit) * (2 ** bit_num))

print(hex(v2b(0, bit_num = 20, lower_limit = -5, upper_limit = 5)))
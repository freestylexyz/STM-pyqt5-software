# -*- coding: utf-8 -*-
"""
Created on Tue Nov 24 12:58:04 2020

@author: yaoji
"""
import conversion as cnv
import pickle
from sequence import mySequence

# Sequence raw data
command = ['Wait', 'Match', 'Dout', 'Shift', 'Aout', 'Ramp', 'Read', 'ShiftRamp']
channel = [' ', ' ', 'Feedback', 'Z offset', 'Bias', 'Iset', 'PREAMP', 'Z offset fine']
option1 = [0, 1, 0, 1, 0, 0, 0, 0]
option2 = [0, 0, 0, 0, 0, 0x5a, 0, 0x5a]
data = ['100', 'M', '1', '1', 'Origin', '1', '100', '100' ]

seq1 = mySequence([], [])
seq1.load_data(command, channel, option1, option2, data)
seq1.validation()
seq1.build()
seq1.name = 'test seq 2'

with open('test2.seq', 'wb') as output:
    pickle.dump(seq1, output, pickle.HIGHEST_PROTOCOL)

# del seq1

with open('test2.seq', 'rb') as input:
    seq2 = pickle.load(input)
    
# print(hex(seq2.command_list))  # -> banana
print(seq2.command)  # -> 40
for i in range(seq2.seq_num):
    print(hex(seq2.data_list[i]))
# seq2.load_data(['read'], [1], [0], [0], [10])
# print(seq2.command)  # -> 40
# print(seq1.command)  # -> 40
print(hex(cnv.vb(1, 'a')))



# class Company(object):
#     def __init__(self, name, value):
#         self.name = name
#         self.value = value

# with open('company_data.seq', 'wb') as output:
#     company1 = Company('banana', 40)
#     pickle.dump(company1, output, pickle.HIGHEST_PROTOCOL)

#     company2 = Company('spam', 42)
#     pickle.dump(company2, output, pickle.HIGHEST_PROTOCOL)

# del company1
# del company2

# with open('company_data.seq', 'rb') as input:
#     company1 = pickle.load(input)
#     print(company1.name)  # -> banana
#     print(company1.value)  # -> 40

#     company2 = pickle.load(input)
#     print(company2.name) # -> spam
#     print(company2.value)  # -> 42
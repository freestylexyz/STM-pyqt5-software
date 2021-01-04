# -*- coding: utf-8 -*-
"""
Created on Sun Jan  3 15:55:44 2021

@author: yaoji
"""
from datetime import datetime


class dep_data():
    def __init__(self):
        now = datetime.now()
        self.time = now.strftime("%d/%m/%Y %H:%M:%S")
        self.data = []
        self.seq_name = 'Poke Default'
        self.read_mode = 0
        
class image_data():
    def __init(self):
        now = datetime.now()
        self.time = now.strftime("%d/%m/%Y %H:%M:%S")
        self.data = []
        
class spc_data():
    def __init(self):
        now = datetime.now()
        self.time = now.strftime("%d/%m/%Y %H:%M:%S")
        self.data = []
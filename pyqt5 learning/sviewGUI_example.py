# -*- coding: utf-8 -*-
"""
@Date     : 2020/12/30 16:50:15
@Author   : milier00
@FileName : aa.py
"""


# ## Sample Code 1
# import sviewgui.sview as sv
# sv.buildGUI()


## Sample Code 2
import sviewgui.sview as sv
FILE_PATH = "./dataset/scan_example.csv"
sv.buildGUI(FILE_PATH)


## Sample Code 3
# import sviewgui.sview as sv
# import pandas as pd
# FILE_PATH = "User/Documents/yourdata.csv"
# df = pd.read_csv(FILE_PATH)
# sv.buildGUI(df)
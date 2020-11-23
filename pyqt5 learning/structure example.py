# -*- coding: utf-8 -*-
"""
Created on Sun Nov 22 14:23:10 2020

@author: yaoji
"""

class myMainMenu(QMainWindow, Ui_HoGroupSTM):

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()
        self.init_STM()

    def init_UI(self):
        self.actionSetting.triggered['bool'].connect(self.open_setting)
        self.actionTipAppr.triggered['bool'].connect(self.open_tipappr)
        self.actionEtest.triggered['bool'].connect(self.open_etest)

    def init_STM(self):
        
        #### Initial flags
        
        #### Instantiation
        self.config = QSettings("config.ini", QSettings.IniFormat)
        self.dsp = myDSP()
        self.setting = mySetting()
        # self.settingwidget = mySetting()
        # self.tipapprwidget = myTipApproach()
        # self.etestwidget = myEtest()
        
        #### Connect all signal after instantiation
        self.dsp.succeed_signal.connect(self.dsp_succeed_slot)                   # dsp initialed --> popup messagebox displaying result
        self.setting.initdsp_signal.connect(self.setting_initdsp_slot)
        self.setting.loadoffest_signal.connect(self.setting_loadoffest_slot)

        #### Load setting
        self.load_config()
        self.dsp.init_dsp()
        
        

    def load_config(self): 
        self.dsp.port = 
        self.dsp.baudrate =
        self.preampgain =
        # ...... (we need to load all status variable from configuration file in the future)

    def open_setting(self):
        self.setting.init_setting(many varaibles)
        self.setting.show()
        # self.settingwidget.show()
        
    # pop up a dialog to show dsp_succeed_signal
    def dsp_succeed_slot(self, ifsucceed):
        self.versionlabel = 
        if ifsucceed:
            QMessageBox.information(self, "Reminder", "DSP initialized successfully!")
        else:
            QMessageBox.information(self, "Reminder", "DSP initialization failed!")
    
    def setting_initdsp_slot(self, port, baudrate):
        self.dsp.port = port
        self.dsp.baudrate = baudrate
        self.dsp.init_dsp()     # reinitial dsp
        self.init_setting()            # change setting view
    
    def setting_loadoffset_slot(self):
        pass
    
    

            
        # self.settingwidget.initButton.clicked.connect(self.reinit_dsp)          # initButton clicked --> reinitialize dsp
        # self.settingwidget.loadButton.clicked.connect(self.setting_signal_slot) # loadButton clicked --> load offset to dsp
        ### Connect at the beginning
        
# Need to rewrite close event of control
    # self.dsp.close()


class mySetting(QWidget, Ui_Setting):
    initdsp = pyqtSignal(str, int)
    loadoffset = pyqtSignal(int, int, int, int)


    def __init__(self):                 # Only do most basic initialization say connect signal
        super().__init__()
        self.setupUi(self)
        self.init_UI()
        
    
    #     # not called for now
    def init_UI(self):
        self.initButton.clicked.connect(self.initButton_slot)
        self.loadButton.clicked.connect(self.loadButton_slot)
        # self.settings = QSettings("config.ini", QSettings.IniFormat)
        # self.baud = self.settings.value("SETUP/BAUD_VALUE")
        # self.comboBox_baud.setCurrentText(self.baud)
        # # self.comboBox_baud.currentIndexChanged.connect(self.combox_baud_cb)
        # # self.loadButton.clicked.connect(self.btn_test_cb)
        
    def init_setting(self, many variables):
        if self.dsp.succeed:    # try "if False:" to test it
            self.settingwidget.comboBox_uart.setCurrentText(str(self.dsp.port))
            self.settingwidget.comboBox_baud.setCurrentText(str(self.dsp.baudrate))
        
            # self.settingwidget.comboBox_uart.setEnabled(False)
            # self.settingwidget.comboBox_baud.setEnabled(False)
            # self.settingwidget.initButton.setEnabled(True)
            # self.settingwidget.loadButton.setEnabled(True)
            # self.settingwidget.spinBox_Biasoffset25_Setting.setEnabled(True)
            # self.settingwidget.spinBox_Biasoffset5_Setting.setEnabled(True)
            # self.settingwidget.spinBox_Biasoffset10_Setting.setEnabled(True)
            # self.settingwidget.spinBox_Isetoff_Setting.setEnabled(True)
            ### This can be written as one function in my setting class called enable
            self.setting.enable(True)
            
            
            
            self.settingwidget.spinBox_Biasoffset25_Setting.setValue(self.dsp.offset[3])
            self.settingwidget.spinBox_Biasoffset5_Setting.setValue(self.dsp.offset[8])
            self.settingwidget.spinBox_Biasoffset10_Setting.setValue(self.dsp.offset[13])
            self.settingwidget.spinBox_Isetoff_Setting.setValue(self.dsp.offset[-1])
        else:
            self.settingwidget.find_port()                                      # ask setting to auto find available ports
    
    


    # auto find available ports and add them into comboBox list
    def find_port(self):
        port_list = [port[0] for port in serial.tools.list_ports.comports()]
        # print(port_list)
        self.comboBox_uart.clear()
        self.comboBox_uart.addItems(port_list)

    def initButton_slot():
        self.initdsp.emit()
    
    def loadButton_slot():
        self.loadoffset.emit()


    # # not called for now
    # def combox_baud_cb(self):
    #     self.baud = self.comboBox_baud.currentText()

    # # not called for now
    # def btn_test_cb(self):
    #     self.settings.setValue("SETUP/BAUD_VALUE", self.baud)
print('hello world')
print(hex([1,2,3,3]))

# Scan | Spectroscopy
self.scan.spc.comboBox_RampCh_General.setCurrentIndex(self.cnfgEX.value("SPECTROSCOPY/RAMP_CHANNEL", type=int))
# bias_flag = '20' if self.bias_dac else 'd'
# bias_ran = self.dsp.dacrange[13]
# bias_mid = 0x80000 if self.bias_dac else 0x8000
# bias_max = 0xfffff if self.bias_dac else 0xffff
# self.scan.spc.spiBox_StepSize_General.setValue(cnv.bv(0x0, bias_flag, bias_ran))
# self.scan.spc.spinBox_Max_General.setValue(cnv.bv(bias_max, bias_flag, bias_ran))
# self.scan.spc.spinBox_Min_General.setValue(cnv.bv(0x0, bias_flag, bias_ran))
print('loading spc1 ', self.cnfgEX.value("SPECTROSCOPY/MIN", type=float), self.scan.spc.spinBox_Min_General.value())
self.scan.spc.spiBox_StepSize_General.setValue(self.cnfgEX.value("SPECTROSCOPY/STEP_SIZE", type=float))
self.scan.spc.step_cnv(True, 0)
self.scan.spc.spinBox_Max_General.setValue(self.cnfgEX.value("SPECTROSCOPY/MAX", type=float))
self.scan.spc.max_cnv(True, 0)
self.scan.spc.spinBox_Min_General.setValue(self.cnfgEX.value("SPECTROSCOPY/MIN", type=float))
self.scan.spc.min_cnv(True, 0)
print('loading spc2 ', self.cnfgEX.value("SPECTROSCOPY/MIN", type=float),
      self.scan.spc.spinBox_Min_General.value())
self.scan.spc.label_DataNum_General.setText(self.cnfgEX.value("SPECTROSCOPY/NUM"))
self.scan.spc.spinBox_Pass_General.setValue(self.cnfgEX.value("SPECTROSCOPY/PASS", type=int))
self.scan.spc.groupBox_Delta_General.setChecked(self.cnfgEX.value("SPECTROSCOPY/DELTA", type=bool))
self.scan.spc.spinBox_Delta_Z.setValue(self.cnfgEX.value("SPECTROSCOPY/DELTA_Z", type=int))
self.scan.spc.scrollBar_Z_Delta.setValue(self.scan.spc.spinBox_Delta_Z.value())
self.scan.spc.spinBox_Bias_Delta.setValue(self.cnfgEX.value("SPECTROSCOPY/DELTA_BIAS", type=float))
self.scan.spc.bias_cnv(True, 0)
self.scan.spc.groupBox_Mapping.setChecked(self.cnfgEX.value("SPECTROSCOPY/MAPPING", type=bool))
# print('loading ', self.cnfgEX.value("SPECTROSCOPY/MAPPING", type=bool))

print('loading spc5 ', self.spinBox_Min_General.value(),
      cnv.bv(self.scrollBar_Max_General.value() - step, bias_flag, self.bias_ran))
self.spinBox_Min_General.setValue(cnv.bv(value, bias_flag, self.bias_ran))
print('loading spc6 ', self.spinBox_Min_General.value())

'''
Line "serial.tools.list_ports.comports()" returns a ListPortInfo object.
And it works like below when I make DSP as COM2 and convertor as COM3.
'''
import serial.tools.list_ports

port_list = list(serial.tools.list_ports.comports())
print(port_list[0])
# COM2 - XDS100 Class USB Serial Port (COM2)
print(port_list[0][0])
# COM2
print(port_list[0][1])
# XDS100 Class USB Serial Port (COM2)
print(port_list[1])
# COM3 - Prolific USB-to-Serial Comm Port (COM3)

'''
The stupid code:
'''
def check_valid_uart():
    port_values = []
    port_list = list(serial.tools.list_ports.comports())        # return a list including all available ports

    if (0 == len(port_list)):                                   # if no port detected
        print("can't find serial port")
        self.comboBox_uart.setCurrentIndex(-1)                  # set combobox item's index as -1
        self.comboBox_uart.clear()                              # clear combobox list
    else:
        for i in range(0, len(port_list)):                      # if port exists
            port_values.append(port_list[i][0])                 # record port names

        for i in range(len(port_list)):                         # find port name in combobox list and return its index
            index = self.comboBox_uart.findText(port_values[i], Qt.MatchFixedString)
            if (index < 0):                                     # if port name is not listed in combobox
                current_text = self.comboBox_uart.currentText() # currently selected item's name in combobox
                self.comboBox_uart.addItem(port_values[i])      # add port name into combobox
                if ("" == current_text):                        # if no combobox item is currently slelected
                    # print("current text is NULL")
                    sleep(0.04)
                    self.comboBox_uart.setCurrentIndex(-1)      # keep the null item's index -1
                else:                                           # if some item is currently selected
                    print("current text is ", current_text)     # print item name
        # remove eut
        count = self.comboBox_uart.count()                      # num of items listed in combobox
        for i in range(count):
            data = self.comboBox_uart.itemText(i)
            is_uart_exist = 0                                   # define a flag to tell existence of port in combobox
            for j in range(len(port_list)):
                if (data == port_values[j]):                    # if port is already listed in combobox
                    is_uart_exist = 1
            if (0 == is_uart_exist):                            # if port is not listed in combobox
                # remove1
                self.comboBox_uart.removeItem(i)                # delete this useless item
                current_data = self.comboBox_uart.currentText() # grap currently selected item name in combobox
                if (current_data == data or current_data == ""):# if the deleted item is still displayed or nothing displayed
                    self.comboBox_uart.clear()                  # clear the combobox list anyway

'''
So we need to specify several points:
    1. When one than one port is available, how to choose DSP? -- Need DSP info to filter.
    2. What if  auto detection failed? -- Manually select one port.
    3. Or just load setting every time? -- Auto detection may work as a debug function.
Reference for "serial.tools.list_ports":
    https://pyserial.readthedocs.io/en/latest/tools.html
'''
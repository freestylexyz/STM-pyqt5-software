from PyQt5.Qt import *
from pyqtgraph import PlotWidget
from PyQt5 import QtCore
import numpy as np
import pyqtgraph as pq

class Window(QWidget):
    def __init__(self):
        super().__init__()
        # 设置下尺寸
        self.resize(600,460)
        # 添加 PlotWidget 控件
        self.plotWidget_1 = PlotWidget(self)
        self.plotWidget_2 = PlotWidget(self)
        # 设置该控件尺寸和相对位置
        self.plotWidget_1.setGeometry(QtCore.QRect(25,25,540,180))
        self.plotWidget_2.setGeometry(QtCore.QRect(25, 230, 540, 180))

        # 仿写 mode1 代码中的数据
        # 生成 300 个正态分布的随机数
        self.data1 = np.random.normal(size=300)
        self.data2 = np.random.normal(size=300)

        self.curve1 = self.plotWidget_1.plot(self.data1, name="mode1")
        self.curve2 = self.plotWidget_2.plot(self.data1, name="mode2")
        self.ptr1 = 0

        # 设定定时器1
        self.timer_1 = pq.QtCore.QTimer()
        # 定时器信号绑定 update_data 函数
        self.timer_1.timeout.connect(self.update_data_1)
        # 定时器间隔50ms，可以理解为 50ms 刷新一次数据
        self.timer_1.start(50)

        # 设定定时器2
        self.timer_2 = pq.QtCore.QTimer()
        # 定时器信号绑定 update_data 函数
        self.timer_2.timeout.connect(self.update_data_2)
        # 定时器间隔50ms，可以理解为 50ms 刷新一次数据
        self.timer_2.start(50)

    # 数据左移1
    def update_data_1(self):
        self.data1[:-1] = self.data1[1:]
        self.data1[-1] = np.random.normal()
        # 数据填充到绘制曲线中
        self.curve1.setData(self.data1)

    # 数据左移2
    def update_data_2(self):
        self.data2[:-1] = self.data2[1:]
        self.data2[-1] = np.random.normal()
        # 数据填充到绘制曲线中
        self.curve2.setData(self.data2)
        # x 轴记录点
        self.ptr1 += 1
        # 重新设定 x 相关的坐标原点
        self.curve2.setPos(self.ptr1,0)


if __name__ == '__main__':
    import sys
    # PyQt5 程序固定写法
    app = QApplication(sys.argv)

    # 将绑定了绘图控件的窗口实例化并展示
    window = Window()
    window.show()

    # PyQt5 程序固定写法
    sys.exit(app.exec())

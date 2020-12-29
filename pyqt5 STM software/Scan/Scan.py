# -*- coding: utf-8 -*-
"""
Created on Wed Dec  2 15:18:34 2020

@author: yaoji
"""

import sys
import io
sys.path.append("../ui/")
sys.path.append("../MainMenu/")
sys.path.append("../Setting/")
sys.path.append("../Model/")
sys.path.append("../TipApproach/")
sys.path.append("../Scan/")
sys.path.append("../Etest/")
from PyQt5.QtWidgets import QApplication , QWidget
from PyQt5.QtGui import QPixmap
from PyQt5.QtCore import pyqtSignal , Qt, QRectF
from PyQt5 import QtCore
from Scan_ui import Ui_Scan
from Spectroscopy import mySpc
from Deposition import myDeposition
from Track import myTrack
from Hop import myHop
from Manipulation import myManipulation
import pyqtgraph as pg
import numpy as np
from symbols import mySymbols

class myScan(QWidget, Ui_Scan):
    close_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.init_UI()
        self.spc = mySpc()
        self.depostion = myDeposition()
        self.track = myTrack()
        self.hop = myHop()
        self.manipulation = myManipulation()
        
        self.mode = 0       # Scan mode: Scan(0), Spectroscopy(1), Deposition(2)
                            # Track(3), Hop(4), Manipulation(5)

    def init_UI(self):
        # self.close_signal.clicked(self.spc.close_signal)
        # self.close_signal.clicked(self.depostion.close_signal)
        # self.close_signal.clicked(self.track.close_signal)
        # self.close_signal.clicked(self.hop.close_signal)
        # self.close_signal.clicked(self.manipulation.close_signal)

        self.view = pg.ViewBox()
        self.graphicsView_Scan.setCentralItem(self.view)

        ## lock the aspect ratio
        self.view.setAspectLocked(True)

        ## Create image item
        self.img = pg.ImageItem(np.zeros((440, 440)))
        self.view.addItem(self.img)

        ## Set initial self.view bounds
        self.view.setRange(QRectF(0, 0, 440, 440))

        ## start drawing with 3x3 brush
        self.symbol = mySymbols()
        self.kernel = self.symbol.cross()
        self.img.setDrawKernel(self.kernel, mask=self.kernel, center=(12, 12), mode='set')
        self.img.setLevels([0, 1])

        # Custom ROI for selecting an image region
        self.roi = pg.ROI([200, 200], [40, 40])

        # handles rotating around center
        self.roi.addRotateHandle([0.5, 1], [0.5, 0.5])

        # handles scaling both vertically and horizontally
        self.roi.addScaleHandle([1, 1], [0.5, 0.5])
        self.roi.addScaleHandle([0, 0], [0.5, 0.5])
        self.roi.addScaleHandle([0, 1], [0.5, 0.5])
        self.roi.addScaleHandle([1, 0], [0.5, 0.5])

        self.view.addItem(self.roi)
        self.roi.setZValue(10)  # make sure ROI is drawn above image

        # Contrast/color control
        self.hist = pg.HistogramLUTItem()
        self.hist.setImageItem(self.img)
        self.graphicsView_Histogram_Scan.addItem(self.hist)

        # # add an image
        # image = QPixmap("../data/scan_example.png").toImage()
        # # print(image.width())
        # channels_count = 4
        # s = image.bits().asstring(image.width() * image.height() * channels_count)
        # self.arr = np.frombuffer(s, dtype = np.uint8).reshape((image.width(), image.height(), channels_count))
        # self.img.setImage(self.arr)
        # self.hist.setLevels(self.arr.min(), self.arr.max())
        #
        # self.img.hoverEvent = self.imageHoverEvent

    # show the position, pixel, and value under the mouse cursor.
    def imageHoverEvent(self, event):
        if event.isExit():
            self.label_Pos_Scan.setText("")
            return
        pos = event.pos()
        i, j = pos.y(), pos.x()
        i = int(np.clip(i, 0, self.arr.shape[0] - 1))
        j = int(np.clip(j, 0, self.arr.shape[1] - 1))
        val = self.arr[i, j]
        ppos = self.img.mapToParent(pos)
        x, y = ppos.x(), ppos.y()
        r = io.StringIO()
        print("pos: (",round(x,2),",",round(y,2),")  pixel: (",i,",",j,")  value:",val, file=r)
        row = r.getvalue()
        self.label_Pos_Scan.setText(row)


    def init_scan(self):
        self.spc.init_spc()
        self.depostion.init_deposition()
        self.track.init_track()
        self.hop.init_hop()
        self.manipulation.init_manipulation()
    
    # Emit close signal
    def closeEvent(self, event):
        if self.mode == 0:
            # !!! pop window to double check
            self.close_signal.emit()
            event.accept()
        else:
            # !!! pop message close other windows
            event.ignore()
        
    def enable_serial(self, enable):
        pass

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myScan()
    window.show()
    sys.exit(app.exec_())

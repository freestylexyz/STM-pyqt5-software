# -*- coding: utf-8 -*-
from PyQt5 import QtCore, QtGui
from pyqtgraph import GraphicsView
from pyqtgraph import GradientEditorItem
from pyqtgraph import OrderedDict
import numpy as np

import weakref
__all__ = ['customGradientWidget']

Gradients = OrderedDict([
    ('thermal', {'ticks': [(0.3333, (185, 0, 0, 255)), (0.6666, (255, 220, 0, 255)), (1, (255, 255, 255, 255)), (0, (0, 0, 0, 255))], 'mode': 'rgb'}),
    ('reverse_thermal', {'ticks': [(0.3333, (255, 220, 0, 255)), (0.6666, (185, 0, 0, 255)), (1, (0, 0, 0, 255)), (0, (255, 255, 255, 255))], 'mode': 'rgb'}),
    ('flame', {'ticks': [(0.2, (7, 0, 220, 255)), (0.5, (236, 0, 134, 255)), (0.8, (246, 246, 0, 255)), (1.0, (255, 255, 255, 255)), (0.0, (0, 0, 0, 255))], 'mode': 'rgb'}),
    ('yellowy', {'ticks': [(0.0, (0, 0, 0, 255)), (0.2328863796753704, (32, 0, 129, 255)), (0.8362738179251941, (255, 255, 0, 255)), (0.5257586450247, (115, 15, 255, 255)), (1.0, (255, 255, 255, 255))], 'mode': 'rgb'} ),
    ('bipolar', {'ticks': [(0.0, (0, 255, 255, 255)), (1.0, (255, 255, 0, 255)), (0.5, (0, 0, 0, 255)), (0.25, (0, 0, 255, 255)), (0.75, (255, 0, 0, 255))], 'mode': 'rgb'}),
    ('spectrum', {'ticks': [(1.0, (255, 0, 255, 255)), (0.0, (255, 0, 0, 255))], 'mode': 'hsv'}),
    ('cyclic', {'ticks': [(0.0, (255, 0, 4, 255)), (1.0, (255, 0, 0, 255))], 'mode': 'hsv'}),
    ('greyclip', {'ticks': [(0.0, (0, 0, 0, 255)), (0.99, (255, 255, 255, 255)), (1.0, (255, 0, 0, 255))], 'mode': 'rgb'}),
    ('grey', {'ticks': [(0.0, (0, 0, 0, 255)), (1.0, (255, 255, 255, 255))], 'mode': 'rgb'}),
    ('reverse_grey', {'ticks': [(1.0, (0, 0, 0, 255)), (0.0, (255, 255, 255, 255))], 'mode': 'rgb'}),
    # Perceptually uniform sequential colormaps from Matplotlib 2.0
    ('viridis', {'ticks': [(0.0, (68, 1, 84, 255)), (0.25, (58, 82, 139, 255)), (0.5, (32, 144, 140, 255)), (0.75, (94, 201, 97, 255)), (1.0, (253, 231, 36, 255))], 'mode': 'rgb'}),
    ('inferno', {'ticks': [(0.0, (0, 0, 3, 255)), (0.25, (87, 15, 109, 255)), (0.5, (187, 55, 84, 255)), (0.75, (249, 142, 8, 255)), (1.0, (252, 254, 164, 255))], 'mode': 'rgb'}),
    ('plasma', {'ticks': [(0.0, (12, 7, 134, 255)), (0.25, (126, 3, 167, 255)), (0.5, (203, 71, 119, 255)), (0.75, (248, 149, 64, 255)), (1.0, (239, 248, 33, 255))], 'mode': 'rgb'}),
    ('magma', {'ticks': [(0.0, (0, 0, 3, 255)), (0.25, (80, 18, 123, 255)), (0.5, (182, 54, 121, 255)), (0.75, (251, 136, 97, 255)), (1.0, (251, 252, 191, 255))], 'mode': 'rgb'}),
])

def addGradientListToDocstring():
    """Decorator to add list of current pre-defined gradients to the end of a function docstring."""
    def dec(fn):
        if fn.__doc__ is not None:
            fn.__doc__ = fn.__doc__ + str(list(Gradients.keys())).strip('[').strip(']')
        return fn
    return dec

class customGradientWidget(GraphicsView):
    """
    Widget displaying an editable color gradient. The user may add, move, recolor,
    or remove colors from the gradient. Additionally, a context menu allows the 
    user to select from pre-defined gradients.
    """
    sigGradientChanged = QtCore.Signal(object)
    sigGradientChangeFinished = QtCore.Signal(object)
    
    def __init__(self, parent=None, orientation='bottom',  *args, **kargs):
        """
        The *orientation* argument may be 'bottom', 'top', 'left', or 'right' 
        indicating whether the gradient is displayed horizontally (top, bottom)
        or vertically (left, right) and on what side of the gradient the editable 
        ticks will appear.
        
        All other arguments are passed to 
        :func:`GradientEditorItem.__init__ <pyqtgraph.GradientEditorItem.__init__>`.
        
        Note: For convenience, this class wraps methods from 
        :class:`GradientEditorItem <pyqtgraph.GradientEditorItem>`.
        """
        GraphicsView.__init__(self, parent, useOpenGL=False, background=None)
        self.maxDim = 31
        kargs['tickPen'] = 'k'
        self.item = GradientEditorItem(*args, **kargs)
        self.item.sigGradientChanged.connect(self.sigGradientChanged)
        self.item.sigGradientChangeFinished.connect(self.sigGradientChangeFinished)
        self.setCentralItem(self.item)
        self.setOrientation(orientation)
        self.setCacheMode(self.CacheNone)
        self.setRenderHints(QtGui.QPainter.Antialiasing | QtGui.QPainter.TextAntialiasing)
        self.setFrameStyle(QtGui.QFrame.NoFrame | QtGui.QFrame.Plain)
        #self.setBackgroundRole(QtGui.QPalette.NoRole)
        #self.setBackgroundBrush(QtGui.QBrush(QtCore.Qt.NoBrush))
        #self.setAutoFillBackground(False)
        #self.setAttribute(QtCore.Qt.WA_PaintOnScreen, False)
        #self.setAttribute(QtCore.Qt.WA_OpaquePaintEvent, True)

    def setOrientation(self, ort):
        """Set the orientation of the widget. May be one of 'bottom', 'top', 
        'left', or 'right'."""
        self.item.setOrientation(ort)
        self.orientation = ort
        self.setMaxDim()
        
    def setMaxDim(self, mx=None):
        if mx is None:
            mx = self.maxDim
        else:
            self.maxDim = mx
            
        if self.orientation in ['bottom', 'top']:
            self.setFixedHeight(mx)
            self.setMaximumWidth(16777215)
        else:
            self.setFixedWidth(mx)
            self.setMaximumHeight(16777215)
        
    def __getattr__(self, attr):
        ### wrap methods from GradientEditorItem
        return getattr(self.item, attr)

    def widgetGroupInterface(self):
        return (self.sigGradientChanged, self.saveState, self.restoreState)

    @addGradientListToDocstring()
    def loadPreset(self, name):
        """
        Load a predefined gradient. Currently defined gradients are: 
        """  ## TODO: provide image with names of defined gradients

        # global Gradients
        self.restoreState(Gradients[name])

    def restoreState(self, state):
        """
        Restore the gradient specified in state.

        ==============  ====================================================================
        **Arguments:**
        state           A dictionary with same structure as those returned by
                        :func:`saveState <pyqtgraph.GradientEditorItem.saveState>`

                        Keys must include:

                            - 'mode': hsv or rgb
                            - 'ticks': a list of tuples (pos, (r,g,b,a))
        ==============  ====================================================================
        """
        ## public

        # Mass edit ticks without graphics update
        signalsBlocked = self.blockSignals(True)

        self.setColorMode(state['mode'])
        for t in list(self.ticks.keys()):
            self.removeTick(t, finish=False)
        for t in state['ticks']:
            c = QtGui.QColor(*t[1])
            self.addTick(t[0], c, finish=False)
        self.showTicks(show=False)

        # Close with graphics update
        self.blockSignals(signalsBlocked)
        self.sigTicksChanged.emit(self)
        self.sigGradientChangeFinished.emit(self)

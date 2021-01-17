# -*- coding: utf-8 -*-
"""
@Date     : 2021/1/6 20:08:55
@Author   : milier00
@FileName : customROI.py
"""
from pyqtgraph import ROI
from pyqtgraph import ViewBox

from PyQt5 import QtCore, QtGui
import numpy as np
#from numpy.linalg import norm

from pyqtgraph import Point
from pyqtgraph import SRTTransform
from math import cos, sin
from pyqtgraph import functions as fn
from pyqtgraph import GraphicsObject
from pyqtgraph import UIGraphicsItem
from pyqtgraph import getConfigOption
from PyQt5.QtCore import Qt


class CrossCenterROI(ROI):

    def __init__(self, pos, size, centered=False, sideScalers=False, **args):
        ROI.__init__(self, pos, size, **args)
        if centered:
            center = [0.5, 0.5]
        else:
            center = [0, 0]

        self.addScaleHandle([1, 1], center)
        if sideScalers:
            self.addScaleHandle([1, 0.5], [center[0], 0.5])
            self.addScaleHandle([0.5, 1], [0.5, center[1]])

    def paint(self, p, opt, widget):
        # Note: don't use self.boundingRect here, because subclasses may need to redefine it.
        r = QtCore.QRectF(0, 0, self.state['size'][0], self.state['size'][1]).normalized()
        p.setRenderHint(QtGui.QPainter.Antialiasing)
        p.setPen(self.currentPen)
        p.translate(r.left(), r.top())
        p.scale(r.width() / 20, r.height() / 20)  # Dan 1/4/2021
        p.drawRect(0, 0, 20, 20)                  # Dan 1/4/2021

    def addHandle(self, info, index=None):
        ## If a Handle was not supplied, create it now
        if 'item' not in info or info['item'] is None:
            h = Handle(self.handleSize, typ=info['type'], pen=self.handlePen,
                       hoverPen=self.handleHoverPen, parent=self)
            info['item'] = h
        else:
            h = info['item']
            if info['pos'] is None:
                info['pos'] = h.pos()
        h.setPos(info['pos'] * self.state['size'])

        ## connect the handle to this ROI
        # iid = len(self.handles)
        h.connectROI(self)
        if index is None:
            self.handles.append(info)
        else:
            self.handles.insert(index, info)

        h.setZValue(self.zValue() + 1)
        self.stateChanged()
        return h

    def addCustomHandle(self, info, index=None):
        ## If a Handle was not supplied, create it now
        if 'item' not in info or info['item'] is None:
            h = CustomHandle(self.handleSize, typ=info['type'], pen=self.handlePen,
                       hoverPen=self.handleHoverPen, parent=self)
            info['item'] = h
        else:
            h = info['item']
            if info['pos'] is None:
                info['pos'] = h.pos()
        h.setPos(info['pos'] * self.state['size'])

        ## connect the handle to this ROI
        # iid = len(self.handles)
        h.connectROI(self)
        if index is None:
            self.handles.append(info)
        else:
            self.handles.insert(index, info)

        h.setZValue(self.zValue() + 1)
        self.stateChanged()
        return h

    def movePoint(self, handle, pos, modifiers=QtCore.Qt.KeyboardModifier(), finish=True, coords='parent'):
        ## called by Handles when they are moved.
        ## pos is the new position of the handle in scene coords, as requested by the handle.

        newState = self.stateCopy()
        index = self.indexOfHandle(handle)
        h = self.handles[index]
        p0 = self.mapToParent(h['pos'] * self.state['size'])
        p1 = Point(pos)

        if coords == 'parent':
            pass
        elif coords == 'scene':
            p1 = self.mapSceneToParent(p1)
        else:
            raise Exception("New point location must be given in either 'parent' or 'scene' coordinates.")

        ## Handles with a 'center' need to know their local position relative to the center point (lp0, lp1)
        if 'center' in h:
            c = h['center']
            cs = c * self.state['size']
            lp0 = self.mapFromParent(p0) - cs
            lp1 = self.mapFromParent(p1) - cs

        if h['type'] == 't':
            snap = True if (modifiers & QtCore.Qt.ControlModifier) else None
            self.translate(p1 - p0, snap=snap, update=False)

        elif h['type'] == 'f':
            newPos = self.mapFromParent(p1)
            h['item'].setPos(newPos)
            h['pos'] = newPos
            self.freeHandleMoved = True

        elif h['type'] == 's':
            ## If a handle and its center have the same x or y value, we can't scale across that axis.
            if h['center'][0] == h['pos'][0]:
                lp1[0] = 0
            if h['center'][1] == h['pos'][1]:
                lp1[1] = 0

            ## snap
            if self.scaleSnap or (modifiers & QtCore.Qt.ControlModifier):
                lp1[0] = round(lp1[0] / self.scaleSnapSize) * self.scaleSnapSize
                lp1[1] = round(lp1[1] / self.scaleSnapSize) * self.scaleSnapSize

            ## preserve aspect ratio (this can override snapping)
            if h['lockAspect'] or (modifiers & QtCore.Qt.AltModifier):
                # arv = Point(self.preMoveState['size']) -
                lp1 = lp1.proj(lp0)

            ## determine scale factors and new size of ROI
            hs = h['pos'] - c
            if hs[0] == 0:
                hs[0] = 1
            if hs[1] == 0:
                hs[1] = 1
            newSize = lp1 / hs

            ## Perform some corrections and limit checks
            if newSize[0] == 0:
                newSize[0] = newState['size'][0]
            if newSize[1] == 0:
                newSize[1] = newState['size'][1]
            if not self.invertible:
                if newSize[0] < 0:
                    newSize[0] = newState['size'][0]
                if newSize[1] < 0:
                    newSize[1] = newState['size'][1]
            if self.aspectLocked:
                newSize[0] = newSize[1]

            ## Move ROI so the center point occupies the same scene location after the scale
            s0 = c * self.state['size']
            s1 = c * newSize
            cc = self.mapToParent(s0 - s1) - self.mapToParent(Point(0, 0))

            ## update state, do more boundary checks
            newState['size'] = newSize
            newState['pos'] = newState['pos'] + cc
            if self.maxBounds is not None:
                r = self.stateRect(newState)
                if not self.maxBounds.contains(r):
                    return

            self.setPos(newState['pos'], update=False)
            self.setSize(newState['size'], update=False)

        elif h['type'] in ['r', 'rf']:
            if h['type'] == 'rf':
                self.freeHandleMoved = True

            if not self.rotatable:
                return
            ## If the handle is directly over its center point, we can't compute an angle.
            try:
                if lp1.length() == 0 or lp0.length() == 0:
                    return
            except OverflowError:
                return

            ## determine new rotation angle, constrained if necessary
            ang = newState['angle'] - lp0.angle(lp1)
            if ang is None:  ## this should never happen..
                return
            if self.rotateSnap or (modifiers & QtCore.Qt.ControlModifier):
                ang = round(ang / self.rotateSnapAngle) * self.rotateSnapAngle

            ## create rotation transform
            tr = QtGui.QTransform()
            tr.rotate(ang)

            ## move ROI so that center point remains stationary after rotate
            cc = self.mapToParent(cs) - (tr.map(cs) + self.state['pos'])
            newState['angle'] = ang
            newState['pos'] = newState['pos'] + cc

            ## check boundaries, update
            if self.maxBounds is not None:
                r = self.stateRect(newState)
                if not self.maxBounds.contains(r):
                    return
            self.setPos(newState['pos'], update=False)
            self.setAngle(ang, update=False)

            ## If this is a free-rotate handle, its distance from the center may change.

            if h['type'] == 'rf':
                h['item'].setPos(self.mapFromScene(p1))  ## changes ROI coordinates of handle
                h['pos'] = self.mapFromParent(p1)

        elif h['type'] == 'sr':
            if h['center'][0] == h['pos'][0]:
                scaleAxis = 1
                nonScaleAxis = 0
            else:
                scaleAxis = 0
                nonScaleAxis = 1

            try:
                if lp1.length() == 0 or lp0.length() == 0:
                    return
            except OverflowError:
                return

            ang = newState['angle'] - lp0.angle(lp1)
            if ang is None:
                return
            if self.rotateSnap or (modifiers & QtCore.Qt.ControlModifier):
                ang = round(ang / self.rotateSnapAngle) * self.rotateSnapAngle

            hs = abs(h['pos'][scaleAxis] - c[scaleAxis])
            newState['size'][scaleAxis] = lp1.length() / hs
            # if self.scaleSnap or (modifiers & QtCore.Qt.ControlModifier):
            if self.scaleSnap:  ## use CTRL only for angular snap here.
                newState['size'][scaleAxis] = round(newState['size'][scaleAxis] / self.snapSize) * self.snapSize
            if newState['size'][scaleAxis] == 0:
                newState['size'][scaleAxis] = 1
            if self.aspectLocked:
                newState['size'][nonScaleAxis] = newState['size'][scaleAxis]

            c1 = c * newState['size']
            tr = QtGui.QTransform()
            tr.rotate(ang)

            cc = self.mapToParent(cs) - (tr.map(c1) + self.state['pos'])
            newState['angle'] = ang
            newState['pos'] = newState['pos'] + cc
            if self.maxBounds is not None:
                r = self.stateRect(newState)
                if not self.maxBounds.contains(r):
                    return

            self.setState(newState, update=False)

        self.stateChanged(finish=finish)

    def indexOfHandle(self, handle):
        """
        Return the index of *handle* in the list of this ROI's handles.
        """
        if isinstance(handle, Handle):
            index = [i for i, info in enumerate(self.handles) if info['item'] is handle]
            if len(index) == 0:
                raise Exception("Cannot return handle index; not attached to this ROI")
            return index[0]
        if isinstance(handle, CustomHandle):
            index = [i for i, info in enumerate(self.handles) if info['item'] is handle]
            if len(index) == 0:
                raise Exception("Cannot return handle index; not attached to this ROI")
            return index[0]
        else:
            return handle



class Handle(UIGraphicsItem):
    """
    Handle represents a single user-interactable point attached to an ROI. They
    are usually created by a call to one of the ROI.add___Handle() methods.

    Handles are represented as a square, diamond, or circle, and are drawn with
    fixed pixel size regardless of the scaling of the view they are displayed in.

    Handles may be dragged to change the position, size, orientation, or other
    properties of the ROI they are attached to.
    """
    types = {  ## defines number of sides, start angle for each handle type
        't': (4, np.pi / 4),
        'f': (4, np.pi / 4),
        's': (4, 0),
        'r': (12, 0),
        'sr': (12, 0),
        'rf': (12, 0),
    }

    sigClicked = QtCore.Signal(object, object)  # self, event
    sigRemoveRequested = QtCore.Signal(object)  # self

    def __init__(self, radius, typ=None, pen=(200, 200, 220),
                 hoverPen=(255, 255, 0), parent=None, deletable=False ):
        self.rois = []
        self.radius = radius
        self.typ = typ
        self.pen = fn.mkPen(pen)
        self.hoverPen = fn.mkPen(hoverPen)
        self.currentPen = self.pen
        self.pen.setWidth(0)
        self.pen.setCosmetic(True)
        self.isMoving = False
        self.sides, self.startAng = self.types[typ]
        self.buildPath()
        self._shape = None
        self.menu = self.buildMenu()

        UIGraphicsItem.__init__(self, parent=parent)
        self.setAcceptedMouseButtons(QtCore.Qt.NoButton)
        self.deletable = deletable
        if deletable:
            self.setAcceptedMouseButtons(QtCore.Qt.RightButton)
        self.setZValue(11)

    def connectROI(self, roi):
        ### roi is the "parent" roi, i is the index of the handle in roi.handles
        self.rois.append(roi)

    def disconnectROI(self, roi):
        self.rois.remove(roi)

    def setDeletable(self, b):
        self.deletable = b
        if b:
            self.setAcceptedMouseButtons(self.acceptedMouseButtons() | QtCore.Qt.RightButton)
        else:
            self.setAcceptedMouseButtons(self.acceptedMouseButtons() & ~QtCore.Qt.RightButton)

    def removeClicked(self):
        self.sigRemoveRequested.emit(self)

    def hoverEvent(self, ev):
        hover = False
        if not ev.isExit():
            if ev.acceptDrags(QtCore.Qt.LeftButton):
                hover = True
            for btn in [QtCore.Qt.LeftButton, QtCore.Qt.RightButton, QtCore.Qt.MidButton]:
                if int(self.acceptedMouseButtons() & btn) > 0 and ev.acceptClicks(btn):
                    hover = True

        if hover:
            self.currentPen = self.hoverPen
        else:
            self.currentPen = self.pen
        self.update()

    def mouseClickEvent(self, ev):
        ## right-click cancels drag
        if ev.button() == QtCore.Qt.RightButton and self.isMoving:
            self.isMoving = False  ## prevents any further motion
            self.movePoint(self.startPos, finish=True)
            ev.accept()
        elif int(ev.button() & self.acceptedMouseButtons()) > 0:
            ev.accept()
            if ev.button() == QtCore.Qt.RightButton and self.deletable:
                self.raiseContextMenu(ev)
            self.sigClicked.emit(self, ev)
        else:
            ev.ignore()

    def buildMenu(self):
        menu = QtGui.QMenu()
        menu.setTitle("Handle")
        self.removeAction = menu.addAction("Remove handle", self.removeClicked)
        return menu

    def getMenu(self):
        return self.menu

    def raiseContextMenu(self, ev):
        # menu = self.scene().addParentContextMenus(self, self.getMenu(), ev)   # Dan 1/17/2021
        menu = self.getMenu()  # Dan 1/17/2021
        ## Make sure it is still ok to remove this handle
        removeAllowed = all([r.checkRemoveHandle(self) for r in self.rois])
        self.removeAction.setEnabled(removeAllowed)
        pos = ev.screenPos()
        menu.popup(QtCore.QPoint(pos.x(), pos.y()))

    def mouseDragEvent(self, ev):
        if ev.button() != QtCore.Qt.LeftButton:
            return
        ev.accept()

        ## Inform ROIs that a drag is happening
        ##  note: the ROI is informed that the handle has moved using ROI.movePoint
        ##  this is for other (more nefarious) purposes.
        # for r in self.roi:
        # r[0].pointDragEvent(r[1], ev)

        if ev.isFinish():
            if self.isMoving:
                for r in self.rois:
                    r.stateChangeFinished()
            self.isMoving = False
            self.currentPen = self.pen
            self.update()
        elif ev.isStart():
            for r in self.rois:
                r.handleMoveStarted()
            self.isMoving = True
            self.startPos = self.scenePos()
            self.cursorOffset = self.scenePos() - ev.buttonDownScenePos()
            self.currentPen = self.hoverPen

        if self.isMoving:  ## note: isMoving may become False in mid-drag due to right-click.
            pos = ev.scenePos() + self.cursorOffset
            self.currentPen = self.hoverPen
            self.movePoint(pos, ev.modifiers(), finish=False)

    def movePoint(self, pos, modifiers=QtCore.Qt.KeyboardModifier(), finish=True):
        for r in self.rois:
            if not r.checkPointMove(self, pos, modifiers):
                return
        # print "point moved; inform %d ROIs" % len(self.roi)
        # A handle can be used by multiple ROIs; tell each to update its handle position
        for r in self.rois:
            r.movePoint(self, pos, modifiers, finish=finish, coords='scene')

    def buildPath(self):
        size = self.radius
        self.path = QtGui.QPainterPath()
        ang = self.startAng
        dt = 2 * np.pi / self.sides
        for i in range(0, self.sides + 1):
            x = size * cos(ang)
            y = size * sin(ang)
            ang += dt
            if i == 0:
                self.path.moveTo(x, y)
            else:
                self.path.lineTo(x, y)


    def paint(self, p, opt, widget):
        p.setRenderHints(p.Antialiasing, True)
        p.setPen(self.currentPen)

        p.drawPath(self.shape())

    def shape(self):
        if self._shape is None:
            s = self.generateShape()
            if s is None:
                return self.path
            self._shape = s
            self.prepareGeometryChange()  ## beware--this can cause the view to adjust, which would immediately invalidate the shape.
        return self._shape

    def boundingRect(self):
        s1 = self.shape()
        return self.shape().boundingRect()

    def generateShape(self):
        dt = self.deviceTransform()

        if dt is None:
            self._shape = self.path
            return None

        v = dt.map(QtCore.QPointF(1, 0)) - dt.map(QtCore.QPointF(0, 0))
        va = np.arctan2(v.y(), v.x())

        dti = fn.invertQTransform(dt)
        devPos = dt.map(QtCore.QPointF(0, 0))
        tr = QtGui.QTransform()
        tr.translate(devPos.x(), devPos.y())
        tr.rotate(va * 180. / 3.1415926)

        return dti.map(tr.map(self.path))

    def viewTransformChanged(self):
        GraphicsObject.viewTransformChanged(self)
        self._shape = None  ## invalidate shape, recompute later if requested.
        self.update()



class CustomHandle(UIGraphicsItem):
    """
    Handle represents a single user-interactable point attached to an ROI. They
    are usually created by a call to one of the ROI.add___Handle() methods.

    Handles are represented as a square, diamond, or circle, and are drawn with
    fixed pixel size regardless of the scaling of the view they are displayed in.

    Handles may be dragged to change the position, size, orientation, or other
    properties of the ROI they are attached to.
    """
    types = {  ## defines number of sides, start angle for each handle type
        't': (4, np.pi / 4),
        'f': (4, np.pi / 4),
        's': (4, 0),
        'r': (12, 0),
        'sr': (12, 0),
        'rf': (12, 0),
    }

    sigClicked = QtCore.Signal(object, object)  # self, event
    sigRemoveRequested = QtCore.Signal(object)  # self

    def __init__(self, radius, typ=None, pen=(200, 200, 220),
                 hoverPen=(255, 255, 0), parent=None, deletable=False ):
        self.rois = []
        self.radius = radius
        self.typ = typ
        self.pen = fn.mkPen(pen)
        self.hoverPen = fn.mkPen(hoverPen)
        self.currentPen = self.pen
        self.pen.setWidth(0)
        self.pen.setCosmetic(True)
        self.isMoving = False
        self.sides, self.startAng = self.types[typ]
        self.custom = False
        self.buildPath()
        self._shape = None
        self.menu = self.buildMenu()

        UIGraphicsItem.__init__(self, parent=parent)
        self.setAcceptedMouseButtons(QtCore.Qt.NoButton)
        self.deletable = deletable
        if deletable:
            self.setAcceptedMouseButtons(QtCore.Qt.RightButton)
        self.setZValue(11)

    def connectROI(self, roi):
        ### roi is the "parent" roi, i is the index of the handle in roi.handles
        self.rois.append(roi)

    def disconnectROI(self, roi):
        self.rois.remove(roi)

    def setDeletable(self, b):
        self.deletable = b
        if b:
            self.setAcceptedMouseButtons(self.acceptedMouseButtons() | QtCore.Qt.RightButton)
        else:
            self.setAcceptedMouseButtons(self.acceptedMouseButtons() & ~QtCore.Qt.RightButton)

    def removeClicked(self):
        self.sigRemoveRequested.emit(self)

    def hoverEvent(self, ev):
        hover = False
        if not ev.isExit():
            if ev.acceptDrags(QtCore.Qt.LeftButton):
                hover = True
            for btn in [QtCore.Qt.LeftButton, QtCore.Qt.RightButton, QtCore.Qt.MidButton]:
                if int(self.acceptedMouseButtons() & btn) > 0 and ev.acceptClicks(btn):
                    hover = True

        if hover:
            self.currentPen = self.hoverPen
        else:
            self.currentPen = self.pen
        self.update()

    def mouseClickEvent(self, ev):
        ## right-click cancels drag
        if ev.button() == QtCore.Qt.RightButton and self.isMoving:
            self.isMoving = False  ## prevents any further motion
            self.movePoint(self.startPos, finish=True)
            ev.accept()
        elif int(ev.button() & self.acceptedMouseButtons()) > 0:
            ev.accept()
            if ev.button() == QtCore.Qt.RightButton and self.deletable:
                self.raiseContextMenu(ev)
            self.sigClicked.emit(self, ev)
        else:
            ev.ignore()

    def buildMenu(self):
        menu = QtGui.QMenu()
        menu.setTitle("Handle")
        self.removeAction = menu.addAction("Remove handle", self.removeClicked)
        return menu

    def getMenu(self):
        return self.menu

    def raiseContextMenu(self, ev):
        # menu = self.scene().addParentContextMenus(self, self.getMenu(), ev)   # Dan 1/17/2021
        menu = self.getMenu()  # Dan 1/17/2021
        ## Make sure it is still ok to remove this handle
        removeAllowed = all([r.checkRemoveHandle(self) for r in self.rois])
        self.removeAction.setEnabled(removeAllowed)
        pos = ev.screenPos()
        menu.popup(QtCore.QPoint(pos.x(), pos.y()))

    def mouseDragEvent(self, ev):
        if ev.button() != QtCore.Qt.LeftButton:
            return
        ev.accept()

        ## Inform ROIs that a drag is happening
        ##  note: the ROI is informed that the handle has moved using ROI.movePoint
        ##  this is for other (more nefarious) purposes.
        # for r in self.roi:
        # r[0].pointDragEvent(r[1], ev)

        if ev.isFinish():
            if self.isMoving:
                for r in self.rois:
                    r.stateChangeFinished()
            self.isMoving = False
            self.currentPen = self.pen
            self.update()
        elif ev.isStart():
            for r in self.rois:
                r.handleMoveStarted()
            self.isMoving = True
            self.startPos = self.scenePos()
            self.cursorOffset = self.scenePos() - ev.buttonDownScenePos()
            self.currentPen = self.hoverPen

        if self.isMoving:  ## note: isMoving may become False in mid-drag due to right-click.
            pos = ev.scenePos() + self.cursorOffset
            self.currentPen = self.hoverPen
            self.movePoint(pos, ev.modifiers(), finish=False)

    def movePoint(self, pos, modifiers=QtCore.Qt.KeyboardModifier(), finish=True):
        for r in self.rois:
            if not r.checkPointMove(self, pos, modifiers):
                return
        # print "point moved; inform %d ROIs" % len(self.roi)
        # A handle can be used by multiple ROIs; tell each to update its handle position
        for r in self.rois:
            r.movePoint(self, pos, modifiers, finish=finish, coords='scene')

    def buildPath(self):
        size = self.radius
        self.path = QtGui.QPainterPath()
        self.path.moveTo(0, size)
        self.path.lineTo(0, -size)
        self.path.moveTo(-size, 0)
        self.path.lineTo(size, 0)

    def paint(self, p, opt, widget):
        p.setRenderHints(p.Antialiasing, True)
        p.setPen(self.currentPen)

        p.drawPath(self.shape())

    def shape(self):
        if self._shape is None:
            s = self.generateShape()
            if s is None:
                return self.path
            self._shape = s
            self.prepareGeometryChange()  ## beware--this can cause the view to adjust, which would immediately invalidate the shape.
        return self._shape

    def boundingRect(self):
        s1 = self.shape()
        return self.shape().boundingRect()

    def generateShape(self):
        dt = self.deviceTransform()

        if dt is None:
            self._shape = self.path
            return None

        v = dt.map(QtCore.QPointF(1, 0)) - dt.map(QtCore.QPointF(0, 0))
        va = np.arctan2(v.y(), v.x())

        dti = fn.invertQTransform(dt)
        devPos = dt.map(QtCore.QPointF(0, 0))
        tr = QtGui.QTransform()
        tr.translate(devPos.x(), devPos.y())
        tr.rotate(va * 180. / 3.1415926)

        return dti.map(tr.map(self.path))

    def viewTransformChanged(self):
        GraphicsObject.viewTransformChanged(self)
        self._shape = None  ## invalidate shape, recompute later if requested.
        self.update()
# -*- coding: utf-8 -*-

from PyQt4.QtGui import QAbstractScrollArea, QTabWidget, QToolButton, QHBoxLayout, QVBoxLayout, QLabel, QWidget, QSpacerItem, QSizePolicy
from PyQt4.QtCore import Qt, QObject
from PyQt4 import QtCore

from kunquat.qt.twbutton import TWButton

class TypewriterView(QAbstractScrollArea):

    def __init__(self, controller, button_layout, parent = None):
        super(self.__class__, self).__init__(parent)
        self._randbut = self.random_button()
        self._buttons = {}
        self._controller = controller
        view = QWidget()
        rows = QVBoxLayout(view)
        rowc = 0
        PAD = 35
        rpads = [PAD, PAD, 2 * PAD, 3 * PAD]
        for buttons in button_layout:
            row = QWidget()
            rowl = QHBoxLayout(row)
            if rowc > 0:
                rowl.addWidget(self.pad(PAD))
            else:
                rowl.addWidget(self._randbut)
            psize = rpads[rowc]
            rowl.addWidget(self.pad(psize))
            colc = 0
            for i in range(buttons):
                coord = (rowc, colc)
                button = TWButton()
                self._buttons[coord] = button
                QObject.connect(button,
                                QtCore.SIGNAL('released()'),
                                lambda c=coord: self._controller.release(c))
                QObject.connect(button,
                                QtCore.SIGNAL('pressed()'),
                                lambda c=coord: self._controller.press(c))

                rowl.addWidget(button)
                colc += 1
            rowl.addStretch(1)
            rows.addWidget(row)
            rowc += 1
        rows.addStretch(1)
        self.setViewport(view)

    def pad(self, psize):
        pad = QLabel()
        pad.setMinimumWidth(psize)
        pad.setMaximumWidth(psize)
        return pad

    def random_button(self):
        randbut = TWButton(color_style='001')
        QObject.connect(randbut,
                        QtCore.SIGNAL('released()'),
                        lambda: self._controller.release_random())
        QObject.connect(randbut,
                        QtCore.SIGNAL('pressed()'),
                        lambda: self._controller.press_random())
        randbut.set_name(u'⚄')
        return randbut

    def update(self):
        self._randbut.set_led(self.model.get_random_led_color())
        for coord, button in self._buttons.items():
                button.set_color(self.model.data(coord, 'color'))
                button.set_name(self.model.data(coord, 'name'))
                button.setEnabled(self.model.data(coord, 'enabled'))
                button.set_led(self.model.get_led_color(coord))

    def keyPressEvent(self, ev):
        self._controller.keyPressEvent(ev)
        self.setFocus()

    def keyReleaseEvent(self, ev):
        self._controller.keyReleaseEvent(ev)
        self.setFocus()

    def setModel(self, model):
        self.model = model
        self.update()


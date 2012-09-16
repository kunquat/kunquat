# -*- coding: utf-8 -*-

from PyQt4.QtGui import QAbstractScrollArea, QTabWidget, QToolButton, QHBoxLayout, QVBoxLayout, QLabel, QWidget, QSpacerItem, QSizePolicy
from PyQt4.QtCore import Qt, QObject
from PyQt4 import QtCore

from kunquat.qt.twbutton import TWButton

class TypewriterView(QAbstractScrollArea):

    def __init__(self, controller, button_layout, parent = None):
        super(self.__class__, self).__init__(parent)
        self.setFocusPolicy(Qt.TabFocus)
        self._randbut = self._random_button()
        self._buttons = {}
        self._controller = controller
        view = QWidget()
        rows = QVBoxLayout(view)
        PAD = 35
        self.PAD = PAD
        self.rpads = [PAD, PAD, 2 * PAD, 3 * PAD]
        for row in self.create_rows(button_layout):
            rows.addWidget(row)
        rows.addStretch(1)
        self.setViewport(view)

    def all_ints(self):
        i = 0
        while True:
            yield i
            i += 1

    def create_rows(self, button_layout):
        for (i, buttons) in zip(self.all_ints(), button_layout):
            yield self.create_row(i, buttons)

    def special_button(self, rowc):
        if rowc == 0:
            return self._randbut
        return self.pad(self.PAD)

    def create_row(self, rowc, buttons):
        row = QWidget()
        rowl = QHBoxLayout(row)
        rowl.addWidget(self.special_button(rowc))
        psize = self.rpads[rowc]
        rowl.addWidget(self.pad(psize))
        for colc in range(buttons):
            coord = (rowc, colc)
            button = self.get_button(coord)
            rowl.addWidget(button)
        rowl.addStretch(1)
        return row

    def get_button(self, coord):
        if not coord in self._buttons:
            button = TWButton()
            self._buttons[coord] = button
            QObject.connect(button,
                            QtCore.SIGNAL('released()'),
                            lambda c=coord: self._controller.release(c))
            QObject.connect(button,
                            QtCore.SIGNAL('pressed()'),
                            lambda c=coord: self._controller.press(c))
        return self._buttons[coord]

    def pad(self, psize):
        pad = QLabel()
        pad.setMinimumWidth(psize)
        pad.setMaximumWidth(psize)
        return pad

    def _random_button(self):
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
        self._randbut.set_name(self.model.get_die())
        for coord, button in self._buttons.items():
                button.set_color(self.model.data(coord, 'color'))
                button.set_name(self.model.data(coord, 'name'))
                button.set_enabled(self.model.data(coord, 'enabled'))
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


from PyQt4.QtGui import QAbstractScrollArea, QTabWidget, QToolButton, QHBoxLayout, QVBoxLayout, QLabel, QWidget, QSpacerItem, QSizePolicy
from PyQt4.QtCore import Qt, QObject
from PyQt4 import QtCore

from kunquat.qt.twbutton import TWButton

class TypewriterView(QAbstractScrollArea):

    def __init__(self, controller, button_layout, parent = None):
        super(self.__class__, self).__init__(parent)
        self._buttons = {}
        self._controller = controller
        view = QWidget()
        rows = QVBoxLayout(view)
        rowc = 0
        for buttons in button_layout:
            row = QWidget()
            rowl = QHBoxLayout(row)
            pad = QLabel()
            pad.setMinimumWidth(rowc * 40)
            pad.setMaximumWidth(rowc * 40)
            rowl.addWidget(pad)
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


    def update(self):
        for coord, button in self._buttons.items():
                button.set_color(self.model.data(coord, 'color'))
                button.set_name(self.model.data(coord, 'name'))
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


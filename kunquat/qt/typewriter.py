from PyQt4.QtGui import QAbstractScrollArea, QTabWidget, QToolButton, QHBoxLayout, QVBoxLayout, QLabel, QWidget, QSpacerItem, QSizePolicy
from PyQt4.QtCore import Qt, QObject
from PyQt4 import QtCore

from kunquat.qt.twbutton import TWButton
from kunquat.qt.typewriter_model import TypewriterModel
from kunquat.qt.typewriter_view import TypewriterView

class Typewriter():
    def __init__(self):
        self._twmodel = TypewriterModel()
        self._twview = TypewriterView(self, self.count_rows())
        self._twmodel.register_view(self._twview)
        self._keymap = self.get_keymap()

    def press(self, coord):
        self._twmodel.set_led_color(coord, 8)

    def release(self, coord):
        self._twmodel.set_led_color(coord, 0)

    def get_view(self):
        return self._twview

    keys = [
    [Qt.Key_1,Qt.Key_2,Qt.Key_3,Qt.Key_4,Qt.Key_5,Qt.Key_6,Qt.Key_7,Qt.Key_8,Qt.Key_9,Qt.Key_0],
    [Qt.Key_Q,Qt.Key_W,Qt.Key_E,Qt.Key_R,Qt.Key_T,Qt.Key_Y,Qt.Key_U,Qt.Key_I,Qt.Key_O,Qt.Key_P],
    [Qt.Key_A,Qt.Key_S,Qt.Key_D,Qt.Key_F,Qt.Key_G,Qt.Key_H,Qt.Key_J],
    [Qt.Key_Z,Qt.Key_X,Qt.Key_C,Qt.Key_V,Qt.Key_B,Qt.Key_N,Qt.Key_M]
    ]

    def get_keymap(self):
        return dict(self.keymap_helper(self.keys))

    def keymap_helper(self, keys):
        indices = range(max(self.count_rows()))
        for row, buttons in zip(indices, keys):
            for but, key in zip(indices, buttons):
                yield (key, (row, but))

    def count_rows(self):
        return [len(row) for row in self.keys]
                
    def keyPressEvent(self, ev):
        if ev.isAutoRepeat() or ev.modifiers() != Qt.NoModifier:
            ev.ignore()
            return
        try:
            coord = self._keymap[ev.key()]
        except KeyError:
            ev.ignore()
            return
        self.press(coord)

    def keyReleaseEvent(self, ev):
        if ev.isAutoRepeat():
            ev.ignore()
            return
        try:
            coord = self._keymap[ev.key()]
        except KeyError:
            ev.ignore()
            return
        self.release(coord)

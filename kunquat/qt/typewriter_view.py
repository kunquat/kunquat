from PyQt4.QtGui import QAbstractScrollArea, QTabWidget, QToolButton, QHBoxLayout, QVBoxLayout, QLabel, QWidget, QSpacerItem, QSizePolicy
from PyQt4.QtCore import Qt, QObject
from PyQt4 import QtCore

from kunquat.qt.twbutton import TWButton

class TypewriterModel():
    def __init__(self):
        self._buttons = {
            (0,  0): {'color': 'gray', 'name': u''},
            (0,  1): {'color': 'dark', 'name': u'C#'},
            (0,  2): {'color': 'dark', 'name': u'D#'},
            (0,  3): {'color': 'gray', 'name': u''},
            (0,  4): {'color': 'dark', 'name': u'F#'},
            (0,  5): {'color': 'dark', 'name': u'G#'},
            (0,  6): {'color': 'dark', 'name': u'A#'},
            (0,  7): {'color': 'gray', 'name': u''},
            (0,  8): {'color': 'dark', 'name': u'C#'},
            (0,  9): {'color': 'dark', 'name': u'D#'},
            (0, 10): {'color': 'gray', 'name': u''},

            (1,  0): {'color': 'light', 'name': u'C'},
            (1,  1): {'color': 'light', 'name': u'D'},
            (1,  2): {'color': 'light', 'name': u'E'},
            (1,  3): {'color': 'light', 'name': u'F'},
            (1,  4): {'color': 'light', 'name': u'G'},
            (1,  5): {'color': 'light', 'name': u'A'},
            (1,  6): {'color': 'light', 'name': u'B'},
            (1,  7): {'color': 'light', 'name': u'C'},
            (1,  8): {'color': 'light', 'name': u'D'},
            (1,  9): {'color': 'light', 'name': u'E'},
            (1, 10): {'color': 'light', 'name': u'F'},

            (2,  0): {'color': 'gray', 'name': u''},
            (2,  1): {'color': 'dark', 'name': u'C#'},
            (2,  2): {'color': 'dark', 'name': u'D#'},
            (2,  3): {'color': 'gray', 'name': u''},
            (2,  4): {'color': 'dark', 'name': u'F#'},
            (2,  5): {'color': 'dark', 'name': u'G#'},
            (2,  6): {'color': 'dark', 'name': u'A#'},
            (2,  7): {'color': 'gray', 'name': u''},
            (2,  8): {'color': 'dark', 'name': u'C#'},
            (2,  9): {'color': 'dark', 'name': u'D#'},
            (2, 10): {'color': 'gray', 'name': u''},

            (3,  0): {'color': 'light', 'name': u'C'},
            (3,  1): {'color': 'light', 'name': u'D'},
            (3,  2): {'color': 'light', 'name': u'E'},
            (3,  3): {'color': 'light', 'name': u'F'},
            (3,  4): {'color': 'light', 'name': u'G'},
            (3,  5): {'color': 'light', 'name': u'A'},
            (3,  6): {'color': 'light', 'name': u'B'},
            (3,  7): {'color': 'light', 'name': u'C'},
            (3,  8): {'color': 'light', 'name': u'D'},
            (3,  9): {'color': 'light', 'name': u'E'},
            (3, 10): {'color': 'light', 'name': u'F'},
        }

    def get_led_color(self, coord):
        DEFAULT = 0
        row, but = coord
        try:
            color = self._buttons[(row,but)]['led']
        except KeyError:
            return DEFAULT
        return color

    def set_led_color(self, coord, color):
        row, but = coord
        self._buttons[(row, but)]['led'] = color

    def data(self, button, role):
        value = self._buttons[button][role]
        return value

class TypewriterView(QAbstractScrollArea):

    keys = [[
    Qt.Key_1,
    Qt.Key_2, 
    Qt.Key_3, 
    Qt.Key_4, 
    Qt.Key_5, 
    Qt.Key_6, 
    Qt.Key_7, 
    Qt.Key_8, 
    Qt.Key_9, 
    Qt.Key_0
    ],[
    Qt.Key_Q, 
    Qt.Key_W, 
    Qt.Key_E, 
    Qt.Key_R, 
    Qt.Key_T, 
    Qt.Key_Y, 
    Qt.Key_U, 
    Qt.Key_I, 
    Qt.Key_O, 
    Qt.Key_P
    ],[
    Qt.Key_A,
    Qt.Key_S, 
    Qt.Key_D, 
    Qt.Key_F, 
    Qt.Key_G, 
    Qt.Key_H, 
    Qt.Key_J
    ],[
    Qt.Key_Z,
    Qt.Key_X,
    Qt.Key_C,
    Qt.Key_V,
    Qt.Key_B,
    Qt.Key_N,
    Qt.Key_M
    ]]

    def __init__(self, parent = None):
        super(self.__class__, self).__init__(parent)
        self.twmodel = TypewriterModel()
        self._keymap = self.get_keymap()
        self._buttons = {}
        view = QWidget()
        rows = QVBoxLayout(view)
        rowc = 0
        for buttons in self.count_rows():
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
                                lambda c=coord: self.release(c))
                QObject.connect(button,
                                QtCore.SIGNAL('pressed()'),
                                lambda c=coord: self.press(c))

                rowl.addWidget(button)
                colc += 1
            rowl.addStretch(1)
            rows.addWidget(row)
            rowc += 1
        rows.addStretch(1)
        self.setViewport(view)
        self.update()

    def count_rows(self):
        return [len(row) for row in self.keys]

    def update(self):
        for coord, button in self._buttons.items():
                button.set_color(self.twmodel.data(coord, 'color'))
                button.set_name(self.twmodel.data(coord, 'name'))
                button.set_led(self.twmodel.get_led_color(coord))

    def press(self, coord):
        self.twmodel.set_led_color(coord, 8)
        self.update()

    def release(self, coord):
        self.twmodel.set_led_color(coord, 0)
        self.update()

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
        self.setFocus()

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
        self.setFocus()

    def setModel(self, twmodel):
        self.model = twmodel

    def get_keymap(self):
        return dict(self.keymap_helper(self.keys))

    def keymap_helper(self, keys):
        indices = range(max(self.count_rows()))
        for row, buttons in zip(indices, keys):
            for but, key in zip(indices, buttons):
                yield (key, (row, but))
                

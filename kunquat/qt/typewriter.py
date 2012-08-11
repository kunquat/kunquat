from PyQt4.QtGui import QAbstractScrollArea, QTabWidget, QToolButton, QHBoxLayout, QVBoxLayout, QLabel, QWidget, QSpacerItem, QSizePolicy
from PyQt4.QtCore import Qt, QObject
from PyQt4 import QtCore

from kunquat.qt.twbutton import TWButton
from kunquat.qt.typewriter_model import TypewriterModel
from kunquat.qt.typewriter_view import TypewriterView
import random

class Typewriter():

    keys = [
    [Qt.Key_1,Qt.Key_2,Qt.Key_3,Qt.Key_4,Qt.Key_5,Qt.Key_6,Qt.Key_7,Qt.Key_8,Qt.Key_9,Qt.Key_0],
    [Qt.Key_Q,Qt.Key_W,Qt.Key_E,Qt.Key_R,Qt.Key_T,Qt.Key_Y,Qt.Key_U,Qt.Key_I,Qt.Key_O,Qt.Key_P],
    [Qt.Key_A,Qt.Key_S,Qt.Key_D,Qt.Key_F,Qt.Key_G,Qt.Key_H,Qt.Key_J],
    [Qt.Key_Z,Qt.Key_X,Qt.Key_C,Qt.Key_V,Qt.Key_B,Qt.Key_N,Qt.Key_M]
    ]

    notes = [
    [None , (1,1), (3,1), None , (6,1), (8,1), (10,1), None , (1,2), (3,2)],
    [(0,1), (2,1), (4,1), (5,1), (7,1), (9,1), (11,1), (0,2), (2,2), (4,2)],
    [None , (1,0), (3,0), None , (6,0), (8,0), (10,0)],
    [(0,0), (2,0), (4,0), (5,0), (7,0), (9,0), (11,0)]
    ]

    def __init__(self, piano):
        self._piano = piano
        self._twmodel = TypewriterModel()
        self._twview = TypewriterView(self, self.count_rows())
        self._twmodel.register_view(self._twview)
        self._keymap = self.get_keymap()
        self._notemap = self.get_notemap()
        self.random_key = Qt.Key_section

    def press(self, coord):
        self._twmodel.set_led_color(coord, 8)
        try:
            note, octave = self._notemap[(coord)]
        except TypeError:
            return
        self._piano.press(note, octave)

    def random(self):
        note_indices = [x[0] for x in self._keymap.itervalues() if x[1] == 0]
        note_indices.sort()
        value = int(random.gauss(len(note_indices) / 2,
                                 len(note_indices)))
        octave = value // len(note_indices)
        note = value % len(note_indices)
        self._piano.press(note, octave)

    def release(self, coord):
        self._twmodel.set_led_color(coord, 0)
        try:
            note, octave = self._notemap[(coord)]
        except TypeError:
            return
        self._piano.release(note, octave)

    def get_view(self):
        return self._twview

    def all_ints(self):
        i = 0
        while True:
            yield i
            i += 1

    def get_keymap(self):
        return dict(self.keymap_helper(self.keys))

    def keymap_helper(self, mapping):
        for row, buttons in zip(self.all_ints(), mapping):
            for but, key in zip(self.all_ints(), buttons):
                yield (key, (row, but))

    def get_notemap(self):
        return dict(self.notemap_helper(self.notes))

    def notemap_helper(self, mapping):
        for row, buttons in zip(self.all_ints(), mapping):
            for but, note in zip(self.all_ints(), buttons):
                yield ((row, but), note)

    def count_rows(self):
        return [len(row) for row in self.keys]
                
    def keyPressEvent(self, ev):
        if ev.isAutoRepeat() or ev.modifiers() != Qt.NoModifier:
            ev.ignore()
            return
        key = ev.key()
        if key == self.random_key:
            self.random()
            return
        try:
            coord = self._keymap[key]
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

from PyQt4.QtGui import QAbstractScrollArea, QTabWidget, QToolButton, QHBoxLayout, QVBoxLayout, QLabel, QWidget, QSpacerItem, QSizePolicy
from PyQt4.QtCore import Qt, QObject
from PyQt4 import QtCore

from kunquat.tracker.sheet.pattern import Pattern
from kunquat.qt.twbutton import TWButton
from kunquat.qt.typewriter_model import TypewriterModel
from kunquat.qt.typewriter_view import TypewriterView
from random import gauss, choice

class Typewriter():

    def __init__(self, p):
        self.p = p
        self._twmodel = TypewriterModel()
        self._twview = TypewriterView(self, self.count_rows())
        self._twmodel.register_view(self._twview)
        self._keymap = self.get_keymap()
        self._notemap = self.get_notemap()
        self.random_key = Qt.Key_section
        self._previous_random = None

    def get_write_cursor(self):
        pattern = self.p._app.focusWidget()
        if isinstance(pattern, Pattern):
            return pattern.get_cursor()
        else:
           return None

    def play(self, note, octave):
        cursor = self.get_write_cursor()
        if cursor == None:
            self._twview.setFocus()
        self.p._piano.press(note, octave, cursor)

    def press(self, coord):
        self._twmodel.set_led_color(coord, 8)
        try:
            note, octave = self._notemap[(coord)]
        except TypeError:
            return
        self.play(note,octave)

    def release(self, coord):
        self._twmodel.set_led_color(coord, 0)
        try:
            note, octave = self._notemap[(coord)]
        except TypeError:
            return
        self.p._piano.release(note, octave)

    def press_random(self):
        self._twmodel.set_random_led_color(8)
        self._twmodel.roll_die()
        note_indices = list(self._keymap.itervalues())
        note_indices.sort()
        #value = int(gauss(len(note_indices) / 2,
        #                         len(note_indices)))
        #octave = value // len(note_indices)
        #note = value % len(note_indices)
        (octave, note) = choice(note_indices)
        self._previous_random = (note, octave)
        self.play(note,octave)

    def release_random(self):
        self._twmodel.set_random_led_color(0)
        (note, octave) = self._previous_random
        self.p._piano.release(note, octave)

    def get_view(self):
        return self._twview

    def all_ints(self):
        i = 0
        while True:
            yield i
            i += 1

    def get_keymap(self):
        return dict(self.keymap_helper(self.p._scale.keys))

    def keymap_helper(self, mapping):
        for row, buttons in zip(self.all_ints(), mapping):
            for but, key in zip(self.all_ints(), buttons):
                yield (key, (row, but))

    def get_notemap(self):
        notes = self.p._scale.knotes
        return dict(self.notemap_helper(notes))

    def notemap_helper(self, mapping):
        for row, buttons in zip(self.all_ints(), mapping):
            for but, note in zip(self.all_ints(), buttons):
                yield ((row, but), note)

    def count_rows(self):
        return [len(row) for row in self.p._scale.keys]
                
    def keyPressEvent(self, ev):
        if ev.modifiers() != Qt.NoModifier:
            ev.ignore()
            return
        if ev.isAutoRepeat():
            ev.ignore()
            return
        key = ev.key()
        if key == self.random_key:
            self.press_random()
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
        key = ev.key()
        if key == self.random_key:
            self.release_random()
            return
        try:
            coord = self._keymap[key]
        except KeyError:
            ev.ignore()
            return
        self.release(coord)


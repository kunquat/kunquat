# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from bisect import bisect_left

keymap = [100, 200, 300, 400, 500, 600, 700, 800]

def closest(x):
    key_count = len(keymap)
    i = bisect_left(keymap, x)
    if i == key_count:
        return keymap[-1]
    elif i == 0:
        return keymap[0]
    else:
         a = keymap[i]
         b = keymap[i - 1]
         if abs(a - x) < abs(b - x):
             return a
         else:
             return b

class TWLed(QFrame):

    def __init__(self):
        super(QFrame, self).__init__()
        self.setMinimumWidth(35)
        self.setMinimumHeight(15)
        self.setMaximumWidth(35)
        self.setMaximumHeight(15)
        self.setFrameStyle(QFrame.Panel | QFrame.Sunken)
        self.setLineWidth(2)

        self._left = QLabel()
        self._left.setMargin(0)
        self._left.setMaximumWidth(10)
        self._center = QLabel()
        self._center.setMargin(0)
        self._right = QLabel()
        self._right.setMargin(0)
        self._right.setMaximumWidth(10)

        h = QHBoxLayout()
        h.addWidget(self._left)
        h.addWidget(self._center)
        h.addWidget(self._right)
        h.setContentsMargins(0,0,0,0)
        h.setSpacing(0)
        self.setLayout(h)

        self.set_leds(0,0,0)

    def set_leds(self, left_on, center_on, right_on):
        led_colors = {
            (0,0,0): ('#400', '#400', '#400'),
            (0,0,1): ('#400', '#400', '#c00'),
            (0,1,0): ('#c00', '#c00', '#c00'),
            (0,1,1): ('#c00', '#c00', '#f00'),
            (1,0,0): ('#c00', '#400', '#400'),
            (1,0,1): ('#c00', '#400', '#c00'),
            (1,1,0): ('#f00', '#c00', '#c00'),
            (1,1,1): ('#f00', '#c00', '#f00')
        }
        bits = (left_on, center_on, right_on)
        styles = ['QLabel { background-color: %s; }' % c for c in led_colors[bits]]
        (left_style, center_style, right_style) = styles
        self._left.setStyleSheet(left_style)
        self._center.setStyleSheet(center_style)
        self._right.setStyleSheet(right_style)

class TypeWriterButton(QPushButton):

    def __init__(self, pitch):
        QPushButton.__init__(self)
        self._pitch = pitch
        self._ui_manager = None

        self._selected_instrument = None
        self.setMinimumWidth(60)
        self.setMinimumHeight(60)
        layout = QVBoxLayout(self)
        led = TWLed()
        self._led = led
        layout.addWidget(led)
        notename = QLabel()
        self._notename = notename
        notename.setAlignment(Qt.AlignCenter)
        layout.addWidget(notename)
        layout.setAlignment(Qt.AlignCenter)
        self.setFocusPolicy(Qt.NoFocus)

        self._notename.setText('%sc' % self._pitch)
        self.setStyleSheet("QLabel { background-color: #ffe; }")
        self._notename.setStyleSheet("QLabel { color: #000; }")

        self.setEnabled(False)
        QObject.connect(self, SIGNAL('clicked()'),
                        self._play_sound)

    def set_ui_model(self, ui_model):
        self._ui_manager = ui_model.get_ui_manager()
        self._ui_manager.register_updater(self.update_selected_instrument)

    def _play_sound(self):
        if self._selected_instrument:
            self._selected_instrument.set_active_note(0, self._pitch)

    def update_selected_instrument(self):
        print 'up'
        old_instrument = self._selected_instrument
        if old_instrument:
            old_instrument.unregister_updater(self.update_leds)
        new_instrument = self._ui_manager.get_selected_instrument()
        new_instrument.register_updater(self.update_leds)
        self._selected_instrument = new_instrument
        self.setEnabled(True)

    def update_leds(self):
        (left_on, center_on, right_on) = 3 * [0]
        notes = self._selected_instrument.get_active_notes()
        for (_, note) in notes:
            if closest(note) == self._pitch:
                if note < self._pitch:
                    left_on = 1
                elif note == self._pitch:
                    center_on = 1
                elif note > self._pitch:
                    right_on = 1
                else:
                    assert False
        self._led.set_leds(left_on, center_on, right_on)


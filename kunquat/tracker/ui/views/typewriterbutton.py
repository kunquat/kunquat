# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2014
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


class TWLight(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._state = 0
        self._colours = [QColor(x, 0, 0) for x in (0x44, 0xcc, 0xff)]
        self._disabled_colour = QColor(0x88, 0x88, 0x88)

    def set_state(self, state):
        if self._state != state:
            self._state = state
            self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        if self.isEnabled():
            colour = self._colours[self._state]
        else:
            colour = self._disabled_colour
        painter.fillRect(event.rect(), colour)


class TWLed(QFrame):

    def __init__(self):
        super(QFrame, self).__init__()
        self.setFixedSize(QSize(35, 15))
        self.setFrameStyle(QFrame.Panel | QFrame.Sunken)
        self.setLineWidth(2)

        self._left = TWLight()
        self._left.setMaximumWidth(10)
        self._center = TWLight()
        self._right = TWLight()
        self._right.setMaximumWidth(10)

        h = QHBoxLayout()
        h.addWidget(self._left)
        h.addWidget(self._center)
        h.addWidget(self._right)
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(0)
        self.setLayout(h)

        self.set_leds(0, 0, 0)

    def set_leds(self, left_on, center_on, right_on):
        self._left.set_state(left_on + center_on)
        self._center.set_state(center_on)
        self._right.set_state(right_on + center_on)


class TypeWriterButton(QPushButton):

    def __init__(self, row, index):
        QPushButton.__init__(self)
        self._updater = None
        self._ui_manager = None
        self._typewriter_manager = None

        self._active_note = None

        self._row = row
        self._index = index
        self._selected_control = None
        self.setFixedSize(QSize(60, 60))
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

        self.setEnabled(False)
        QObject.connect(self, SIGNAL('pressed()'), self._press)
        QObject.connect(self, SIGNAL('released()'), self._release)

    def set_ui_model(self, ui_model):
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._ui_manager = ui_model.get_ui_manager()
        self._typewriter_manager = ui_model.get_typewriter_manager()

        self._button_model = self._typewriter_manager.get_button_model(
                self._row, self._index)
        self._update_properties()

    def _perform_updates(self, signals):
        if 'signal_octave' in signals:
            self._update_properties()
        self._update_leds()

    def _update_properties(self):
        name = self._button_model.get_name()
        if name != None:
            self._notename.setText(name)
            self.setEnabled(True)
        else:
            self._notename.setText('')
            self.setEnabled(False)

    def _press(self):
        self._release()
        self._active_note = self._button_model.start_tracked_note()

    def _release(self):
        if self._active_note:
            self._active_note.set_rest()
            self._active_note = None

    def _update_leds(self):
        led_state = self._button_model.get_led_state() or (False, False, False)
        left_on, center_on, right_on = (int(state) for state in led_state)
        self._led.set_leds(left_on, center_on, right_on)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)



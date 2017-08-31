# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2013-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *


class TWLight(QWidget):

    def __init__(self):
        super().__init__()
        self._state = 0
        self._colours = None
        self._disabled_colour = None
        self.set_default_colours()

    def set_default_colours(self):
        self.set_colours('#f00', '#888')

    def set_colours(self, active_colour, disabled_colour):
        colour = QColor(active_colour)
        ar = colour.red()
        ag = colour.green()
        ab = colour.blue()
        self._colours = [QColor(int(ar * m), int(ag * m), int(ab * m))
                for m in (0.25, 0.75, 1)]

        self._disabled_colour = QColor(disabled_colour)

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
        super().__init__()
        self.setFixedSize(QSize(35, 15))
        self.setFrameStyle(QFrame.Panel | QFrame.Sunken)
        self.setLineWidth(2)

        self._left = TWLight()
        self._left.setMaximumWidth(10)
        self._centre = TWLight()
        self._right = TWLight()
        self._right.setMaximumWidth(10)

        h = QHBoxLayout()
        h.addWidget(self._left)
        h.addWidget(self._centre)
        h.addWidget(self._right)
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(0)
        self.setLayout(h)

        self.set_leds(0, 0, 0)

    def set_default_colours(self):
        for widget in (self._left, self._centre, self._right):
            widget.set_default_colours()

    def set_colours(self, active_colour, disabled_colour):
        for widget in (self._left, self._centre, self._right):
            widget.set_colours(active_colour, disabled_colour)

    def set_leds(self, left_on, centre_on, right_on):
        self._left.set_state(left_on + centre_on)
        self._centre.set_state(centre_on)
        self._right.set_state(right_on + centre_on)


class TypewriterButton(QPushButton):

    def __init__(self, row, index):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._control_mgr = None
        self._typewriter_mgr = None

        self._row = row
        self._index = index
        self._selected_control = None
        self.setFixedSize(QSize(60, 60))
        layout = QVBoxLayout(self)
        led = TWLed()
        self._led = led
        layout.addWidget(led, 0, Qt.AlignHCenter)
        notename = QLabel()
        self._notename = notename
        notename.setAlignment(Qt.AlignCenter)
        layout.addWidget(notename)
        layout.setAlignment(Qt.AlignCenter)

        self.setFocusPolicy(Qt.NoFocus)

        self.setEnabled(False)
        self.pressed.connect(self._press)
        self.released.connect(self._release)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._control_mgr = ui_model.get_control_manager()
        self._typewriter_mgr = ui_model.get_typewriter_manager()

        self._button_model = self._typewriter_mgr.get_button_model(
                self._row, self._index)

        self._update_style()
        self._update_properties()

    def update_led_state(self, led_state):
        self._led.set_leds(*led_state)

    def _perform_updates(self, signals):
        if any(s in signals for s in ['signal_octave', 'signal_notation']):
            self._update_properties()

        if not set(['signal_selection', 'signal_hits']).isdisjoint(signals):
            keymap_mgr = self._ui_model.get_keymap_manager()
            if keymap_mgr.is_hit_keymap_active():
                self._update_properties()

        if 'signal_style_changed' in signals:
            self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        if not style_mgr.is_custom_style_enabled():
            self._led.set_default_colours()
        else:
            self._led.set_colours(
                    style_mgr.get_style_param('active_indicator_colour'),
                    style_mgr.get_style_param('bg_sunken_colour'))

    def _update_properties(self):
        name = self._button_model.get_name()
        if name != None:
            self._notename.setText(name)
            self.setEnabled(True)
        else:
            self._notename.setText('')
            self.setEnabled(False)

    def _press(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        sheet_mgr = self._ui_model.get_sheet_manager()
        control_id = sheet_mgr.get_inferred_active_control_id_at_location(location)

        self._control_mgr.set_control_id_override(control_id)
        self._button_model.start_tracked_note()
        self._control_mgr.set_control_id_override(None)

    def _release(self):
        self._button_model.stop_tracked_note()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)



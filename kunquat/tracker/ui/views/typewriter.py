# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .keyboardmapper import KeyboardMapper
from .typewriterbutton import TypewriterButton
from .updater import Updater


class Typewriter(QFrame, Updater):

    def __init__(self):
        super().__init__()
        self._typewriter_mgr = None
        self._keyboard_mapper = KeyboardMapper()
        self._pad = 35

    def _on_setup(self):
        self.add_to_updaters(self._keyboard_mapper)
        self.register_action('signal_audio_rendered', self._update_button_leds)
        self.register_action('signal_style_changed', self._update_style)

        self._typewriter_mgr = self._ui_model.get_typewriter_manager()
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._pad = style_mgr.get_scaled_size_param('typewriter_padding')

        if not self.layout():
            self.setLayout(self._get_layout())

        rows = self.layout()
        rows.setSpacing(style_mgr.get_scaled_size_param('small_padding'))
        for row_index in range(rows.count()):
            row = rows.itemAt(row_index).layout()
            assert row
            row.setSpacing(style_mgr.get_scaled_size_param('medium_padding'))
            pad = row.itemAt(0).widget()
            assert pad
            pad_px = self._pad * self._typewriter_mgr.get_pad_factor_at_row(row_index)
            pad.setFixedWidth(pad_px)

    def _get_layout(self):
        style_mgr = self._ui_model.get_style_manager()

        rows = QVBoxLayout()
        rows.setContentsMargins(0, 0, 0, 0)
        rows.setSpacing(style_mgr.get_scaled_size_param('small_padding'))

        for row_index in range(self._typewriter_mgr.get_row_count()):
            rows.addLayout(self._get_row(row_index))
        return rows

    def _get_row(self, index):
        style_mgr = self._ui_model.get_style_manager()

        row = QHBoxLayout()
        row.setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

        pad_px = self._pad * self._typewriter_mgr.get_pad_factor_at_row(index)
        row.addWidget(self._get_pad(pad_px))

        for i in range(self._typewriter_mgr.get_button_count_at_row(index)):
            button = TypewriterButton(index, i)
            self.add_to_updaters(button)
            row.addWidget(button)

        row.addStretch(1)
        return row

    def _get_pad(self, psize):
        pad = QWidget()
        pad.setFixedWidth(psize)
        return pad

    def _update_button_leds(self):
        if not self.isVisible():
            return

        led_states = self._typewriter_mgr.get_led_states()
        rows = self.layout()
        for ri in range(rows.count()):
            row = rows.itemAt(ri).layout()
            for ci in range(row.count() - 2): # -2 excludes padding and end stretch
                button = row.itemAt(ci + 1).widget()
                assert isinstance(button, TypewriterButton)
                led_state = tuple(led_states.get((ri, ci), 3 * [False]))
                button.update_led_state(led_state)

    def keyPressEvent(self, event):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        sheet_mgr = self._ui_model.get_sheet_manager()
        control_id = sheet_mgr.get_inferred_active_control_id_at_location(location)

        control_mgr = self._ui_model.get_control_manager()
        control_mgr.set_control_id_override(control_id)
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()
        control_mgr.set_control_id_override(None)

    def keyReleaseEvent(self, event):
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()



# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2014-2016
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

from .keyboardmapper import KeyboardMapper
from .typewriterbutton import TypewriterButton


class Typewriter(QFrame):

    _PAD = 35

    def __init__(self):
        QFrame.__init__(self)
        self._ui_model = None
        self._updater = None
        self._typewriter_manager = None
        self._current_buttons = set()
        self._keyboard_mapper = KeyboardMapper()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._typewriter_manager = ui_model.get_typewriter_manager()
        self._keyboard_mapper.set_ui_model(ui_model)

        self.setLayout(self._get_layout())

    def unregister_updaters(self):
        self._keyboard_mapper.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)
        self._unregister_button_updaters()

    def _perform_updates(self, signals):
        pass

    def _get_layout(self):
        rows = QVBoxLayout()
        rows.setContentsMargins(0, 0, 0, 0)
        rows.setSpacing(2)
        for row_index in xrange(self._typewriter_manager.get_row_count()):
            rows.addLayout(self._get_row(row_index))
        return rows

    def _get_row(self, index):
        row = QHBoxLayout()
        row.setSpacing(4)

        pad_px = self._PAD * self._typewriter_manager.get_pad_factor_at_row(index)
        row.addWidget(self._get_pad(pad_px))

        for i in xrange(self._typewriter_manager.get_button_count_at_row(index)):
            button = TypewriterButton(index, i)
            button.set_ui_model(self._ui_model)
            row.addWidget(button)
            self._current_buttons.add(button)

        row.addStretch(1)
        return row

    def _get_pad(self, psize):
        pad = QWidget()
        pad.setFixedWidth(psize)
        return pad

    def _unregister_button_updaters(self):
        for button in list(self._current_buttons):
            button.unregister_updaters()
            self._current_buttons.remove(button)

    def keyPressEvent(self, event):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        sheet_manager = self._ui_model.get_sheet_manager()
        control_id = sheet_manager.get_inferred_active_control_id_at_location(location)

        control_manager = self._ui_model.get_control_manager()
        control_manager.set_control_id_override(control_id)
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()
        control_manager.set_control_id_override(None)

    def keyReleaseEvent(self, event):
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()


# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2014
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

from typewriterbutton import TypeWriterButton


class TypeWriter(QAbstractScrollArea):

    def __init__(self):
        QAbstractScrollArea.__init__(self)
        self.setFocusPolicy(Qt.TabFocus)
        self._ui_model = None
        self._updater = None
        self._typewriter_manager = None
        self._current_buttons = set()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._typewriter_manager = ui_model.get_typewriter_manager()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._unregister_button_updaters()

    def _update_buttons(self):
        self._unregister_button_updaters()
        view = self._get_view()
        self.setViewport(view)

    def _perform_updates(self, signals):
        if 'signal_octave' in signals:
            self._update_buttons()

    def _get_view(self):
        view = QWidget()
        rows = QVBoxLayout(view)
        PAD = 35
        self.PAD = PAD
        self.rpads = [PAD, PAD, 4 * PAD, 3 * PAD]
        button_layout = [9, 10, 7, 7]
        for row in self.create_rows(button_layout):
            rows.addWidget(row)
        rows.addStretch(1)
        return view

    def create_rows(self, button_layout):
        for (i, buttons) in enumerate(button_layout):
            yield self.create_row(i, buttons)

    def create_row(self, rowc, buttons):
        row = QWidget()
        rowl = QHBoxLayout(row)
        rowl.addWidget(self.special_button(rowc))
        psize = self.rpads[rowc]
        rowl.addWidget(self.pad(psize))
        for colc in range(buttons):
            coordinate = (rowc, colc)
            button = self.get_button(coordinate)
            self._current_buttons.add(button)
            rowl.addWidget(button)
        rowl.addStretch(1)
        return row

    def special_button(self, rowc):
        if rowc == 0:
            self._randbut = TypeWriterButton(None)
            return self._randbut
        return self.pad(self.PAD)

    def pad(self, psize):
        pad = QLabel()
        pad.setMinimumWidth(psize)
        pad.setMaximumWidth(psize)
        return pad

    def get_button(self, coordinate):
        pitch = self._typewriter_manager.get_button_pitch(coordinate)
        button = TypeWriterButton(pitch)
        button.set_ui_model(self._ui_model)
        return button

    def _unregister_button_updaters(self):
        for button in list(self._current_buttons):
            button.unregister_updaters()
            self._current_buttons.remove(button)



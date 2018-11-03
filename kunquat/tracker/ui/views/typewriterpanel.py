# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2018
#          Toni Ruottu, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .hitmaptoggle import HitMapToggle
from .octaveselector import OctaveSelector
from .typewriter import Typewriter
from .notationselect import NotationSelect
from .updater import Updater


class TypewriterPanel(QFrame, Updater):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._notation_select = NotationSelect()
        self._hit_map_toggle = HitMapToggle()
        self._octave_selector = OctaveSelector()
        self._typewriter = Typewriter()

        self.add_to_updaters(
                self._notation_select,
                self._hit_map_toggle,
                self._octave_selector,
                self._typewriter)

        self._top_layout = QHBoxLayout()
        self._top_layout.setContentsMargins(0, 0, 0, 0)
        self._top_layout.setSpacing(8)
        self._top_layout.addWidget(self._notation_select)
        self._top_layout.addWidget(self._hit_map_toggle)
        self._top_layout.addStretch(1)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(6)
        v.addLayout(self._top_layout)
        v.addWidget(self._octave_selector)
        v.addWidget(self._typewriter)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        medium_pad = style_mgr.get_scaled_size_param('medium_padding')
        large_pad = style_mgr.get_scaled_size_param('large_padding')
        self._top_layout.setSpacing(large_pad)
        self.layout().setSpacing(medium_pad)



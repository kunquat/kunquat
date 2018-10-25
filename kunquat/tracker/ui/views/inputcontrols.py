# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017-2018
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
from .iconbutton import IconButton
from .notationselect import NotationSelect
from .octaveselector import OctaveSelector
from .typewriterpanel import TypewriterPanel
from .updater import Updater


class InputControls(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._full_controls = TypewriterPanel()
        self._compact_controls = CompactControls()
        self._switch_button = IconButton()
        self._switch_button.set_sizes(2.2, 0.5)
        self._switch_button.setFocusPolicy(Qt.NoFocus)

        self._controls = ControlLayout()
        self._controls.setContentsMargins(0, 0, 0, 0)
        self._controls.addWidget(self._full_controls)
        self._controls.addWidget(self._compact_controls)

        h = QHBoxLayout()
        h.setContentsMargins(4, 2, 4, 2)
        h.setSpacing(4)
        h.addLayout(self._controls)
        h.addWidget(self._switch_button, 0, Qt.AlignTop)
        self.setLayout(h)

    def _on_setup(self):
        self.add_to_updaters(
                self._full_controls, self._compact_controls, self._switch_button)
        self.register_action('signal_input_control_layout', self._show_controls)
        self.register_action('signal_style_changed', self._update_style)

        self._switch_button.clicked.connect(self._switch_controls)

        self._update_style()
        self._show_controls()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        layout = self.layout()
        small_pad = style_mgr.get_scaled_size_param('small_padding')
        medium_pad = style_mgr.get_scaled_size_param('medium_padding')
        layout.setContentsMargins(medium_pad, small_pad, medium_pad, small_pad)
        layout.setSpacing(medium_pad)

    def _show_controls(self):
        visibility_mgr = self._ui_model.get_visibility_manager()
        icon_bank = self._ui_model.get_icon_bank()
        view_mode = visibility_mgr.get_input_control_view()
        if view_mode == 'full':
            self._controls.setCurrentIndex(0)
            self._switch_button.set_icon('input_compact')
            self._switch_button.setToolTip('Compact input view')
        elif view_mode == 'compact':
            self._controls.setCurrentIndex(1)
            self._switch_button.set_icon('input_full')
            self._switch_button.setToolTip('Full input view')
        else:
            assert False

    def _switch_controls(self):
        visibility_mgr = self._ui_model.get_visibility_manager()
        cur_view_mode = visibility_mgr.get_input_control_view()
        new_view_mode = 'compact' if cur_view_mode != 'compact' else 'full'
        visibility_mgr.set_input_control_view(new_view_mode)
        self._updater.signal_update('signal_input_control_layout')


class ControlLayout(QVBoxLayout):

    def __init__(self):
        super().__init__()

    def addWidget(self, widget):
        widget.hide()
        super().addWidget(widget)

    def setCurrentIndex(self, index):
        for i in range(self.count()):
            self.itemAt(i).widget().setVisible(i == index)


class CompactControls(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._notation_select = NotationSelect()
        self._hit_map_toggle = HitMapToggle()
        self._octave_selector = OctaveSelector()

        self.add_to_updaters(
                self._notation_select, self._hit_map_toggle, self._octave_selector)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(8)
        h.addWidget(self._notation_select)
        h.addWidget(self._hit_map_toggle)
        h.addWidget(self._octave_selector)
        h.addStretch()
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('large_padding'))



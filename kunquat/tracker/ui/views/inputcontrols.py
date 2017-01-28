# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PySide.QtCore import *
from PySide.QtGui import *

from .hitmaptoggle import HitMapToggle
from .notationselect import NotationSelect
from .octaveselector import OctaveSelector
from .typewriterpanel import TypewriterPanel


class InputControls(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self._full_controls = TypewriterPanel()
        self._compact_controls = CompactControls()
        self._switch_button = QPushButton()
        self._switch_button.setStyleSheet('QPushButton { margin: 0; padding: -1px; }')

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

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._full_controls.set_ui_model(ui_model)
        self._compact_controls.set_ui_model(ui_model)

        QObject.connect(self._switch_button, SIGNAL('clicked()'), self._switch_controls)

        self._show_controls()

    def unregister_updaters(self):
        self._compact_controls.unregister_updaters()
        self._full_controls.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_input_control_layout' in signals:
            self._show_controls()

    def _show_controls(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        icon_bank = self._ui_model.get_icon_bank()
        view_mode = visibility_manager.get_input_control_view()
        if view_mode == 'full':
            self._controls.setCurrentIndex(0)
            icon_path = icon_bank.get_icon_path('input_compact')
            self._switch_button.setIcon(QIcon(icon_path))
            self._switch_button.setToolTip('Compact input view')
        elif view_mode == 'compact':
            self._controls.setCurrentIndex(1)
            icon_path = icon_bank.get_icon_path('input_full')
            self._switch_button.setIcon(QIcon(icon_path))
            self._switch_button.setToolTip('Full input view')
        else:
            assert False

    def _switch_controls(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        cur_view_mode = visibility_manager.get_input_control_view()
        new_view_mode = 'compact' if cur_view_mode != 'compact' else 'full'
        visibility_manager.set_input_control_view(new_view_mode)
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


class CompactControls(QWidget):

    def __init__(self):
        super().__init__()

        self._notation_select = NotationSelect()
        self._hit_map_toggle = HitMapToggle()
        self._octave_selector = OctaveSelector()

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(8)
        h.addWidget(self._notation_select)
        h.addWidget(self._hit_map_toggle)
        h.addWidget(self._octave_selector)
        h.addStretch()
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._notation_select.set_ui_model(ui_model)
        self._hit_map_toggle.set_ui_model(ui_model)
        self._octave_selector.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._octave_selector.unregister_updaters()
        self._hit_map_toggle.unregister_updaters()
        self._notation_select.unregister_updaters()



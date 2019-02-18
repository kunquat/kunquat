# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2019
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

from .composition import Composition
from .inputcontrols import InputControls
from .kqtutils import try_open_kqt_module
from .peakmeter import PeakMeter
from .portal import Portal
from .playbackpanel import PlaybackPanel
from .updater import Updater


class MainView(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._portal = Portal()
        self._playback_panel = PlaybackPanel()
        self._composition = Composition()
        self._input_controls = InputControls()
        self._peak_meter = PeakMeter()

        self.add_to_updaters(
                self._portal,
                self._playback_panel,
                self._composition,
                self._input_controls,
                self._peak_meter)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._portal)
        v.addWidget(self._playback_panel)
        v.addSpacing(2)
        v.addWidget(self._composition)
        v.addWidget(self._input_controls)
        v.addSpacing(2)
        v.addWidget(self._peak_meter)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        spacing = style_mgr.get_scaled_size_param('small_padding')
        layout = self.layout()
        for i in range(layout.count()):
            spacer = layout.itemAt(i).spacerItem()
            if spacer:
                spacer.changeSize(2, spacing)

    def keyPressEvent(self, event):
        if event.modifiers() == Qt.ControlModifier:
            if event.key() == Qt.Key_N:
                process_mgr = self._ui_model.get_process_manager()
                process_mgr.new_kunquat()
            elif event.key() == Qt.Key_O:
                try_open_kqt_module(self._ui_model)
            else:
                event.ignore()
        else:
            event.ignore()



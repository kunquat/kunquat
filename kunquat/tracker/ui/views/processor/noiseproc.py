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

from .processorupdater import ProcessorUpdater


class NoiseProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Noise'

    def __init__(self):
        super().__init__()

        v = QVBoxLayout()
        v.addWidget(QLabel('The Noise processor does not support additional parameters.'))
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)



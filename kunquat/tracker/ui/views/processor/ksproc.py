# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .procnumslider import ProcNumSlider
from .processorupdater import ProcessorUpdater
from . import utils


class KsProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Karplus-Strong'

    def __init__(self):
        super().__init__()
        self._damp = DampSlider()

        self.add_to_updaters(self._damp)

        self._sliders_layout = QGridLayout()
        self._sliders_layout.setContentsMargins(0, 0, 0, 0)
        self._sliders_layout.setVerticalSpacing(0)
        self._sliders_layout.addWidget(QLabel('Damp:'), 0, 0)
        self._sliders_layout.addWidget(self._damp, 0, 1)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addLayout(self._sliders_layout)
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        self._sliders_layout.setHorizontalSpacing(
                style_mgr.get_scaled_size_param('large_padding'))

        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))


class DampSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(1, 0.0, 100.0, '')

    def _get_ks_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_update_signal_type(self):
        return 'signal_ks_damp_{}'.format(self._proc_id)

    def _update_value(self):
        ks_params = self._get_ks_params()
        self.set_number(ks_params.get_damp())

    def _value_changed(self, value):
        ks_params = self._get_ks_params()
        ks_params.set_damp(value)
        self._updater.signal_update(self._get_update_signal_type())



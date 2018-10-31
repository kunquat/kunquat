# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2018
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


class BitcrusherProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Bitcrusher'

    def __init__(self):
        super().__init__()

        self._cutoff = CutoffSlider()
        self._resolution = ResolutionSlider()
        self._res_ignore_min = ResIgnoreMinSlider()

        self.add_to_updaters(self._cutoff, self._resolution, self._res_ignore_min)

        self._sliders_layout = QGridLayout()
        self._sliders_layout.setContentsMargins(0, 0, 0, 0)
        self._sliders_layout.setVerticalSpacing(2)
        self._sliders_layout.addWidget(QLabel('Cutoff:'), 0, 0)
        self._sliders_layout.addWidget(self._cutoff, 0, 1)
        self._sliders_layout.addWidget(QLabel('Resolution:'), 1, 0)
        self._sliders_layout.addWidget(self._resolution, 1, 1)
        self._sliders_layout.addWidget(QLabel('Ignore resolution at minimum:'), 2, 0)
        self._sliders_layout.addWidget(self._res_ignore_min, 2, 1)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.addLayout(self._sliders_layout)
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._sliders_layout.setHorizontalSpacing(
                style_mgr.get_scaled_size_param('medium_padding'))
        self._sliders_layout.setVerticalSpacing(
                style_mgr.get_scaled_size_param('small_padding'))

        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)


class BitcrusherSlider(ProcNumSlider):

    def __init__(self, decimals, min_value, max_value):
        super().__init__(decimals, min_value, max_value, title='', width_txt='100.0')

    def _get_bc_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)


class CutoffSlider(BitcrusherSlider):

    def __init__(self):
        super().__init__(2, 0.0, 100.0)

    def _get_update_signal_type(self):
        return 'signal_bc_cutoff_{}'.format(self._proc_id)

    def _update_value(self):
        bc_params = self._get_bc_params()
        self.set_number(bc_params.get_cutoff())

    def _value_changed(self, cutoff):
        bc_params = self._get_bc_params()
        bc_params.set_cutoff(cutoff)
        self._updater.signal_update(self._get_update_signal_type())


class ResolutionSlider(BitcrusherSlider):

    def __init__(self):
        super().__init__(2, 1.0, 32.0)

    def _get_update_signal_type(self):
        return 'signal_bc_resolution_{}'.format(self._proc_id)

    def _update_value(self):
        bc_params = self._get_bc_params()
        self.set_number(bc_params.get_resolution())

    def _value_changed(self, resolution):
        bc_params = self._get_bc_params()
        bc_params.set_resolution(resolution)
        self._updater.signal_update(self._get_update_signal_type())


class ResIgnoreMinSlider(BitcrusherSlider):

    def __init__(self):
        super().__init__(2, 1.0, 32.0)

    def _get_update_signal_type(self):
        return 'signal_bc_res_ignore_min_{}'.format(self._proc_id)

    def _update_value(self):
        bc_params = self._get_bc_params()
        self.set_number(bc_params.get_res_ignore_min())

    def _value_changed(self, res_ignore_min):
        bc_params = self._get_bc_params()
        bc_params.set_res_ignore_min(res_ignore_min)
        self._updater.signal_update(self._get_update_signal_type())



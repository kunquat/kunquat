# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2018
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


class FreeverbProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Freeverb'

    def __init__(self):
        super().__init__()
        self._refl = ReflSlider()
        self._damp = DampSlider()

        self.add_to_updaters(self._refl, self._damp)

        self._sliders_layout = QGridLayout()
        self._sliders_layout.setContentsMargins(0, 0, 0, 0)
        self._sliders_layout.setVerticalSpacing(0)
        self._sliders_layout.addWidget(QLabel('Reflectivity:'), 0, 0)
        self._sliders_layout.addWidget(self._refl, 0, 1)
        self._sliders_layout.addWidget(QLabel('Damp:'), 1, 0)
        self._sliders_layout.addWidget(self._damp, 1, 1)

        v = QVBoxLayout()
        v.addLayout(self._sliders_layout)
        v.addStretch(1)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

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


class FreeverbSlider(ProcNumSlider):

    def __init__(self, decimals, min_value, max_value):
        super().__init__(decimals, min_value, max_value, '')

    def _get_fv_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        freeverb_params = proc.get_type_params()
        return freeverb_params


class ReflSlider(FreeverbSlider):

    def __init__(self):
        super().__init__(1, 0, 200)

    def _get_update_signal_type(self):
        return '_'.join(('signal_freeverb_refl', self._proc_id))

    def _update_value(self):
        fv_params = self._get_fv_params()
        self.set_number(fv_params.get_reflectivity())

    def _value_changed(self, value):
        fv_params = self._get_fv_params()
        fv_params.set_reflectivity(value)
        self._updater.signal_update(self._get_update_signal_type())


class DampSlider(FreeverbSlider):

    def __init__(self):
        super().__init__(1, 0, 100)

    def _get_update_signal_type(self):
        return '_'.join(('signal_freeverb_damp', self._proc_id))

    def _update_value(self):
        fv_params = self._get_fv_params()
        self.set_number(fv_params.get_damp())

    def _value_changed(self, value):
        fv_params = self._get_fv_params()
        fv_params.set_damp(value)
        self._updater.signal_update(self._get_update_signal_type())



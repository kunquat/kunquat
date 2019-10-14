# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.model.procparams.phaserparams import PhaserParams
from .procnumslider import ProcNumSlider
from .processorupdater import ProcessorUpdater
from . import utils
from kunquat.tracker.ui.views.utils import get_scaled_font


class PhaserProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Phaser'

    def __init__(self):
        super().__init__()

        self._stages = StageCountSlider()
        self._cutoff = CutoffSlider()
        self._notch_sep = NotchSeparationSlider()
        self._dry_wet = DryWetSlider()

        self.add_to_updaters(self._stages, self._cutoff, self._notch_sep, self._dry_wet)

        self._sliders_layout = QGridLayout()
        self._sliders_layout.setContentsMargins(0, 0, 0, 0)
        self._sliders_layout.setVerticalSpacing(0)
        self._sliders_layout.addWidget(QLabel('Stages:'), 0, 0)
        self._sliders_layout.addWidget(self._stages, 0, 1)
        self._sliders_layout.addWidget(QLabel('Cutoff:'), 1, 0)
        self._sliders_layout.addWidget(self._cutoff, 1, 1)
        self._sliders_layout.addWidget(QLabel('Notch separation:'), 2, 0)
        self._sliders_layout.addWidget(self._notch_sep, 2, 1)
        self._sliders_layout.addWidget(QLabel('Dry/wet ratio:'), 3, 0)
        self._sliders_layout.addWidget(self._dry_wet, 3, 1)

        v = QVBoxLayout()
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

        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('large_padding'))


class StageCountSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(
                0,
                PhaserParams.get_min_stage_count(),
                PhaserParams.get_max_stage_count(),
                width_txt='100.00')

    def _get_phaser_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_update_signal_type(self):
        return 'signal_phaser_stage_count_{}'.format(self._proc_id)

    def _update_value(self):
        self.set_number(self._get_phaser_params().get_stage_count())

    def _value_changed(self, count):
        self._get_phaser_params().set_stage_count(count)
        self._updater.signal_update(self._get_update_signal_type())


class CutoffSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(2, 0.0, 100.0)

    def _get_phaser_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_update_signal_type(self):
        return 'signal_phaser_cutoff_{}'.format(self._proc_id)

    def _update_value(self):
        self.set_number(self._get_phaser_params().get_cutoff())

    def _value_changed(self, cutoff):
        self._get_phaser_params().set_cutoff(cutoff)
        self._updater.signal_update(self._get_update_signal_type())


class NotchSeparationSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(2, 0.125, 16.0, width_txt='100.00')

    def _get_phaser_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_update_signal_type(self):
        return 'signal_phaser_notch_sep_{}'.format(self._proc_id)

    def _update_value(self):
        self.set_number(self._get_phaser_params().get_notch_separation())

    def _value_changed(self, sep):
        self._get_phaser_params().set_notch_separation(sep)
        self._updater.signal_update(self._get_update_signal_type())


class DryWetSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(3, 0.0, 1.0, width_txt='100.00')

    def _get_phaser_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_update_signal_type(self):
        return 'signal_phaser_dry_wet_{}'.format(self._proc_id)

    def _update_value(self):
        self.set_number(self._get_phaser_params().get_dry_wet_ratio())

    def _value_changed(self, ratio):
        self._get_phaser_params().set_dry_wet_ratio(ratio)
        self._updater.signal_update(self._get_update_signal_type())



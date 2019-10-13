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


class StageCountSlider(QWidget, ProcessorUpdater):

    _WIDTH_TEXT = '100.00'

    def __init__(self):
        super().__init__()

        self._slider = QSlider()
        self._slider.setOrientation(Qt.Horizontal)
        self._slider.setRange(1, 16)

        self._value = QLabel()

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(0)
        h.addWidget(self._slider)
        h.addWidget(self._value)
        self.setLayout(h)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Maximum)

        self._slider.valueChanged.connect(self._on_value_changed)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self.register_action(self._get_update_signal_type(), self._update_value)

        self._update_style()
        self._update_value()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        fm = QFontMetrics(get_scaled_font(style_mgr, 1))
        value_width = fm.boundingRect(self._WIDTH_TEXT).width()
        value_width += style_mgr.get_scaled_size(2)
        self._value.setFixedWidth(value_width)

        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def _get_phaser_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_update_signal_type(self):
        return 'signal_phaser_stages_{}'.format(self._proc_id)

    def _update_value(self):
        phaser_params = self._get_phaser_params()
        stage_count = phaser_params.get_stage_count()

        old_block = self._slider.blockSignals(True)
        self._slider.setValue(stage_count // 2)
        self._slider.blockSignals(old_block)

        self._value.setText(str(stage_count))

    def _on_value_changed(self, scaled_value):
        stage_count = scaled_value * 2
        phaser_params = self._get_phaser_params()
        phaser_params.set_stage_count(stage_count)
        self._updater.signal_update(self._get_update_signal_type())


class CutoffSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(2, 0.0, 100.0)

    def _get_phaser_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_update_signal_type(self):
        return 'signal_phaser_cutoff_{}'.format(self._proc_id)

    def _update_value(self):
        phaser_params = self._get_phaser_params()
        self.set_number(phaser_params.get_cutoff())

    def _value_changed(self, cutoff):
        phaser_params = self._get_phaser_params()
        phaser_params.set_cutoff(cutoff)
        self._updater.signal_update(self._get_update_signal_type())


class NotchSeparationSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(2, 0.125, 16.0, width_txt='100.00')

    def _get_phaser_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_update_signal_type(self):
        return 'signal_phaser_notch_sep_{}'.format(self._proc_id)

    def _update_value(self):
        phaser_params = self._get_phaser_params()
        self.set_number(phaser_params.get_notch_separation())

    def _value_changed(self, sep):
        phaser_params = self._get_phaser_params()
        phaser_params.set_notch_separation(sep)
        self._updater.signal_update(self._get_update_signal_type())


class DryWetSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(3, 0.0, 1.0, width_txt='100.00')

    def _get_phaser_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_update_signal_type(self):
        return 'signal_phaser_dry_wet_{}'.format(self._proc_id)

    def _update_value(self):
        phaser_params = self._get_phaser_params()
        self.set_number(phaser_params.get_dry_wet_ratio())

    def _value_changed(self, ratio):
        phaser_params = self._get_phaser_params()
        phaser_params.set_dry_wet_ratio(ratio)
        self._updater.signal_update(self._get_update_signal_type())



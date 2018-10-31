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

from kunquat.tracker.ui.model.procparams.compressparams import CompressParams
from kunquat.tracker.ui.views.headerline import HeaderLine
from .procnumslider import ProcNumSlider
from .processorupdater import ProcessorUpdater
from . import utils


class CompressProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Compression'

    def __init__(self):
        super().__init__()

        self._attack = Attack()
        self._release = Release()

        self._upward_config = CompressConfig('upward')
        self._downward_config = CompressConfig('downward')

        self.add_to_updaters(
                self._attack, self._release, self._upward_config, self._downward_config)

        self._common_layout = QHBoxLayout()
        self._common_layout.setContentsMargins(0, 0, 0, 0)
        self._common_layout.setSpacing(10)
        self._common_layout.addWidget(self._attack)
        self._common_layout.addWidget(self._release)

        v = QVBoxLayout()
        v.setSpacing(10)
        v.addLayout(self._common_layout)
        v.addWidget(self._upward_config)
        v.addWidget(self._downward_config)
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        self._common_layout.setSpacing(style_mgr.get_scaled_size_param('large_padding'))

        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('large_padding'))


class CompressSlider(ProcNumSlider):

    def __init__(self, decimals, min_value, max_value, title):
        super().__init__(decimals, min_value, max_value, title)

    def _get_compress_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)


class Attack(CompressSlider):

    def __init__(self):
        min_value, max_value = CompressParams.get_react_range()
        super().__init__(0, min_value, max_value, 'Attack:')

    def _get_update_signal_type(self):
        return 'signal_compress_attack_{}'.format(self._proc_id)

    def _update_value(self):
        params = self._get_compress_params()
        self.set_number(params.get_attack())

    def _value_changed(self, value):
        params = self._get_compress_params()
        params.set_attack(value)
        self._updater.signal_update(self._get_update_signal_type())


class Release(CompressSlider):

    def __init__(self):
        min_value, max_value = CompressParams.get_react_range()
        super().__init__(0, min_value, max_value, 'Release:')

    def _get_update_signal_type(self):
        return 'signal_compress_release_{}'.format(self._proc_id)

    def _update_value(self):
        params = self._get_compress_params()
        self.set_number(params.get_release())

    def _value_changed(self, value):
        params = self._get_compress_params()
        params.set_release(value)
        self._updater.signal_update(self._get_update_signal_type())


class CompressConfig(QWidget, ProcessorUpdater):

    def __init__(self, mode):
        super().__init__()
        assert mode in ('upward', 'downward')
        self._mode = mode

        self._enabled = QCheckBox('Enabled')
        self._threshold = Threshold(self._mode)
        self._ratio = Ratio(self._mode)
        self._range = Range(self._mode)

        self._params_layout = QHBoxLayout()
        self._params_layout.setContentsMargins(0, 0, 0, 0)
        self._params_layout.setSpacing(10)
        self._params_layout.addWidget(self._enabled)
        self._params_layout.addWidget(self._threshold)
        self._params_layout.addWidget(self._ratio)
        self._params_layout.addWidget(self._range)

        self._header = HeaderLine('{} compression'.format(mode.capitalize()))

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(4)
        v.addWidget(self._header)
        v.addLayout(self._params_layout)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._threshold.set_au_id(au_id)
        self._ratio.set_au_id(au_id)
        self._range.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._threshold.set_proc_id(proc_id)
        self._ratio.set_proc_id(proc_id)
        self._range.set_proc_id(proc_id)

    def _on_setup(self):
        self.add_to_updaters(self._threshold, self._ratio, self._range)
        self.register_action(self._get_update_signal_type(), self._update_enabled)
        self._enabled.stateChanged.connect(self._change_enabled)

        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
        self._update_enabled()

    def _get_update_signal_type(self):
        return 'signal_compress_enabled_{}'.format(self._proc_id)

    def _get_compress_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._params_layout.setSpacing(style_mgr.get_scaled_size_param('large_padding'))
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def _update_enabled(self):
        params = self._get_compress_params()
        enabled = getattr(params, 'get_{}_enabled'.format(self._mode))()

        old_block = self._enabled.blockSignals(True)
        self._enabled.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        self._enabled.blockSignals(old_block)

        self._threshold.setEnabled(enabled)
        self._ratio.setEnabled(enabled)
        self._range.setEnabled(enabled)

    def _change_enabled(self, state):
        enabled = (state == Qt.Checked)
        params = self._get_compress_params()
        getattr(params, 'set_{}_enabled'.format(self._mode))(enabled)
        self._updater.signal_update(self._get_update_signal_type())


class Threshold(CompressSlider):

    def __init__(self, mode):
        min_value, max_value = CompressParams.get_threshold_range()
        super().__init__(1, min_value, max_value, 'Threshold:')
        self._mode = mode

    def _get_update_signal_type(self):
        return 'signal_compress_threshold_{}'.format(self._proc_id)

    def _update_value(self):
        params = self._get_compress_params()
        self.set_number(getattr(params, 'get_{}_threshold'.format(self._mode))())

    def _value_changed(self, value):
        params = self._get_compress_params()
        if self._mode == 'upward':
            params.set_upward_threshold(value)
            params.set_downward_threshold(max(params.get_downward_threshold(), value))
        else:
            params.set_downward_threshold(value)
            params.set_upward_threshold(min(params.get_upward_threshold(), value))
        self._updater.signal_update(self._get_update_signal_type())


class Ratio(CompressSlider):

    def __init__(self, mode):
        min_value, max_value = CompressParams.get_ratio_range()
        super().__init__(1, min_value, max_value, 'Ratio:')
        self._mode = mode

    def _get_update_signal_type(self):
        return 'signal_compress_{}_ratio_{}'.format(self._mode, self._proc_id)

    def _update_value(self):
        params = self._get_compress_params()
        self.set_number(getattr(params, 'get_{}_ratio'.format(self._mode))())

    def _value_changed(self, value):
        params = self._get_compress_params()
        getattr(params, 'set_{}_ratio'.format(self._mode))(value)
        self._updater.signal_update(self._get_update_signal_type())


class Range(CompressSlider):

    def __init__(self, mode):
        super().__init__(1, 0.0, CompressParams.get_max_range(), 'Range:')
        self._mode = mode

    def _get_update_signal_type(self):
        return 'signal_compress_{}_range_{}'.format(self._mode, self._proc_id)

    def _update_value(self):
        params = self._get_compress_params()
        self.set_number(getattr(params, 'get_{}_range'.format(self._mode))())

    def _value_changed(self, value):
        params = self._get_compress_params()
        getattr(params, 'set_{}_range'.format(self._mode))(value)
        self._updater.signal_update(self._get_update_signal_type())



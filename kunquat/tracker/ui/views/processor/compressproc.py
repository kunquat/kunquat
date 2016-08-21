# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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

from kunquat.tracker.ui.model.procparams.compressparams import CompressParams
from kunquat.tracker.ui.views.headerline import HeaderLine
from .procnumslider import ProcNumSlider
from . import utils


class CompressProc(QWidget):

    @staticmethod
    def get_name():
        return 'Compression'

    def __init__(self):
        super().__init__()

        self._attack = Attack()
        self._release = Release()

        self._upward_config = CompressConfig('upward')
        self._downward_config = CompressConfig('downward')

        rl = QHBoxLayout()
        rl.setContentsMargins(0, 0, 0, 0)
        rl.setSpacing(10)
        rl.addWidget(self._attack)
        rl.addWidget(self._release)

        v = QVBoxLayout()
        v.setSpacing(10)
        v.addLayout(rl)
        v.addWidget(self._upward_config)
        v.addWidget(self._downward_config)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._attack.set_au_id(au_id)
        self._release.set_au_id(au_id)
        self._upward_config.set_au_id(au_id)
        self._downward_config.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._attack.set_proc_id(proc_id)
        self._release.set_proc_id(proc_id)
        self._upward_config.set_proc_id(proc_id)
        self._downward_config.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._attack.set_ui_model(ui_model)
        self._release.set_ui_model(ui_model)
        self._upward_config.set_ui_model(ui_model)
        self._downward_config.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._downward_config.unregister_updaters()
        self._upward_config.unregister_updaters()
        self._release.unregister_updaters()
        self._attack.unregister_updaters()


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
        self._updater.signal_update(set([self._get_update_signal_type()]))


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
        self._updater.signal_update(set([self._get_update_signal_type()]))


class CompressConfig(QWidget):

    def __init__(self, mode):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        assert mode in ('upward', 'downward')
        self._mode = mode

        self._enabled = QCheckBox('Enabled')
        self._threshold = Threshold(self._mode)
        self._ratio = Ratio(self._mode)

        pl = QHBoxLayout()
        pl.setContentsMargins(0, 0, 0, 0)
        pl.setSpacing(10)
        pl.addWidget(self._enabled)
        pl.addWidget(self._threshold)
        pl.addWidget(self._ratio)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(4)
        v.addWidget(HeaderLine('{} compression'.format(mode.capitalize())))
        v.addLayout(pl)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._threshold.set_au_id(au_id)
        self._ratio.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._threshold.set_proc_id(proc_id)
        self._ratio.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._threshold.set_ui_model(ui_model)
        self._ratio.set_ui_model(ui_model)
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._enabled, SIGNAL('stateChanged(int)'), self._change_enabled)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._ratio.unregister_updaters()
        self._threshold.unregister_updaters()

    def _get_update_signal_type(self):
        return 'signal_compress_enabled_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_enabled()

    def _get_compress_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_enabled(self):
        params = self._get_compress_params()
        enabled = getattr(params, 'get_{}_enabled'.format(self._mode))()

        old_block = self._enabled.blockSignals(True)
        self._enabled.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        self._enabled.blockSignals(old_block)

        self._threshold.setEnabled(enabled)
        self._ratio.setEnabled(enabled)

    def _change_enabled(self, state):
        enabled = (state == Qt.Checked)
        params = self._get_compress_params()
        getattr(params, 'set_{}_enabled'.format(self._mode))(enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))


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
        self._updater.signal_update(set([self._get_update_signal_type()]))


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
        self._updater.signal_update(set([self._get_update_signal_type()]))



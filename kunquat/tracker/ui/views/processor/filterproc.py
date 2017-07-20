# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
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


class FilterProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Filter'

    def __init__(self):
        super().__init__()

        self._filter_type = FilterType()
        self._cutoff = CutoffSlider()
        self._resonance = ResonanceSlider()

        self.add_to_updaters(self._filter_type, self._cutoff, self._resonance)

        sliders = QGridLayout()
        sliders.addWidget(QLabel('Cutoff'), 0, 0)
        sliders.addWidget(self._cutoff, 0, 1)
        sliders.addWidget(QLabel('Resonance'), 1, 0)
        sliders.addWidget(self._resonance, 1, 1)

        v = QVBoxLayout()
        v.addWidget(self._filter_type)
        v.addLayout(sliders)
        v.addStretch(1)
        self.setLayout(v)


class FilterType(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        self._lowpass = QRadioButton('Lowpass')
        self._highpass = QRadioButton('Highpass')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(QLabel('Filter type:'))
        v.addWidget(self._lowpass)
        v.addWidget(self._highpass)
        self.setLayout(v)

    def _get_update_signal_type(self):
        return 'signal_filter_type_{}'.format(self._proc_id)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_type)

        QObject.connect(self._lowpass, SIGNAL('clicked()'), self._set_lowpass)
        QObject.connect(self._highpass, SIGNAL('clicked()'), self._set_highpass)

        self._update_type()

    def _get_filter_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        filter_params = proc.get_type_params()
        return filter_params

    def _update_type(self):
        filter_params = self._get_filter_params()
        types = { 'lowpass': self._lowpass, 'highpass': self._highpass }
        filter_type = filter_params.get_type()
        if filter_type in types:
            widget = types[filter_type]
            old_block = widget.blockSignals(True)
            widget.setChecked(True)
            widget.blockSignals(old_block)

    def _set_lowpass(self):
        filter_params = self._get_filter_params()
        filter_params.set_type('lowpass')

    def _set_highpass(self):
        filter_params = self._get_filter_params()
        filter_params.set_type('highpass')


class FilterSlider(ProcNumSlider):

    def __init__(self, decimals, min_value, max_value):
        super().__init__(decimals, min_value, max_value, '')

    def _get_filter_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        filter_params = proc.get_type_params()
        return filter_params


class CutoffSlider(FilterSlider):

    def __init__(self):
        super().__init__(2, 0.0, 100.0)

    def _get_update_signal_type(self):
        return '_'.join(('signal_filter_cutoff', self._proc_id))

    def _update_value(self):
        filter_params = self._get_filter_params()
        self.set_number(filter_params.get_cutoff())

    def _value_changed(self, cutoff):
        filter_params = self._get_filter_params()
        filter_params.set_cutoff(cutoff)
        self._updater.signal_update(self._get_update_signal_type())


class ResonanceSlider(FilterSlider):

    def __init__(self):
        super().__init__(2, 0.0, 100.0)

    def _get_update_signal_type(self):
        return '_'.join(('signal_filter_resonance', self._proc_id))

    def _update_value(self):
        filter_params = self._get_filter_params()
        self.set_number(filter_params.get_resonance())

    def _value_changed(self, resonance):
        filter_params = self._get_filter_params()
        filter_params.set_resonance(resonance)
        self._updater.signal_update(self._get_update_signal_type())



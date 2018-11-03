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


class VolumeProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Volume'

    def __init__(self):
        super().__init__()

        self._volume = VolumeSlider()

        self.add_to_updaters(self._volume)

        v = QVBoxLayout()
        v.setSpacing(2)
        v.addWidget(self._volume, 0, Qt.AlignTop)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))


class VolumeSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(2, -64.0, 24.0, title='Volume')

    def _get_vol_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        vol_params = proc.get_type_params()
        return vol_params

    def _get_update_signal_type(self):
        return '_'.join(('signal_vol_volume', self._proc_id))

    def _update_value(self):
        vol_params = self._get_vol_params()
        self.set_number(vol_params.get_volume())

    def _value_changed(self, volume):
        vol_params = self._get_vol_params()
        vol_params.set_volume(volume)
        self._updater.signal_update(self._get_update_signal_type())



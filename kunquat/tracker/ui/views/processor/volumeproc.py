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

from PySide.QtCore import *
from PySide.QtGui import *

from .procnumslider import ProcNumSlider
from .updatingprocview import UpdatingProcView


class VolumeProc(QWidget, UpdatingProcView):

    @staticmethod
    def get_name():
        return 'Volume'

    def __init__(self):
        super().__init__()

        self._volume = VolumeSlider()

        self.add_updating_child(self._volume)

        v = QVBoxLayout()
        v.setSpacing(10)
        v.addWidget(self._volume, 0, Qt.AlignTop)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)


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



# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from .procnumslider import ProcNumSlider


class VolumeProc(QWidget):

    @staticmethod
    def get_name():
        return 'Volume'

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None

        self._volume = VolumeSlider()

        v = QVBoxLayout()
        v.setSpacing(10)
        v.addWidget(self._volume, 0, Qt.AlignTop)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def set_au_id(self, au_id):
        self._volume.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._volume.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._volume.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._volume.unregister_updaters()


class VolumeSlider(ProcNumSlider):

    def __init__(self):
        ProcNumSlider.__init__(self, 2, -64.0, 24.0, title='Volume')

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
        self._updater.signal_update(set([self._get_update_signal_type()]))



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

from PyQt4.QtCore import *
from PyQt4.QtGui import *


class StreamProc(QWidget):

    @staticmethod
    def get_name():
        return u'Stream'

    def __init__(self):
        QWidget.__init__(self)

        self._init_val_editor = InitValueEditor()

        editors = QGridLayout()
        editors.addWidget(QLabel('Initial value:'), 0, 0)
        editors.addWidget(self._init_val_editor, 0, 1)

        v = QVBoxLayout()
        v.addLayout(editors)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._init_val_editor.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._init_val_editor.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._init_val_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._init_val_editor.unregister_updaters()


class InitValueEditor(QDoubleSpinBox):

    def __init__(self):
        QDoubleSpinBox.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self.setMinimum(-99999)
        self.setMaximum(99999)
        self.setDecimals(5)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('valueChanged(double)'), self._set_default_value)

        self._update_value()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return 'signal_stream_default_value_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_value()

    def _get_stream_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        stream_params = proc.get_type_params()
        return stream_params

    def _update_value(self):
        stream_params = self._get_stream_params()

        def_value = stream_params.get_init_value()
        if def_value != self.value():
            old_block = self.blockSignals(True)
            self.setValue(def_value)
            self.blockSignals(old_block)

    def _set_default_value(self, value):
        stream_params = self._get_stream_params()
        stream_params.set_init_value(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))



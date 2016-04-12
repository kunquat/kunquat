# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
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

from .audio_unit.editor import Editor


class AuWindow(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._au_id = None
        self._updater = None
        self._editor = Editor()

        v = QVBoxLayout()
        v.setMargin(0)
        v.addWidget(self._editor)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._editor.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        assert self._au_id != None
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._editor.set_ui_model(ui_model)
        self._update_title()

    def unregister_updaters(self):
        self._editor.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_controls' in signals:
            self._update_title()

    def _update_title(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        name = au.get_name()
        if name:
            title = '{} – Kunquat Tracker'.format(name)
        else:
            title = 'Kunquat Tracker'
        self.setWindowTitle(title)

    def closeEvent(self, ev):
        ev.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_audio_unit(self._au_id)

    def sizeHint(self):
        return QSize(1024, 768)



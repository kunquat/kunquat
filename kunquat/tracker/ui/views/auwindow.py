# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
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

from .audiounit.editor import Editor
from .audiounit.audiounitupdater import AudioUnitUpdater
from .saverwindow import SaverWindow


class AuWindow(AudioUnitUpdater, SaverWindow):

    def __init__(self):
        super().__init__()
        self._editor = Editor()

        self.add_to_updaters(self._editor)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.addWidget(self._editor)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_controls', self._update_title)
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



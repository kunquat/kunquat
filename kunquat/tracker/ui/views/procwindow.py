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

from kunquat.tracker.ui.qt import *

from .processor.editor import Editor
from .processor.processorupdater import ProcessorUpdater
from .saverwindow import SaverWindow


class ProcWindow(ProcessorUpdater, SaverWindow):

    def __init__(self):
        super().__init__()
        self._editor = Editor()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.addWidget(self._editor)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._editor)
        self.register_action('signal_controls', self._update_title)

        self._update_title()

    def _update_title(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        parts = []
        au_name = au.get_name()
        proc_name = proc.get_name()
        if proc_name:
            parts.append(proc_name)
        if au_name:
            parts.append('[{}]'.format(au_name))

        if parts:
            title = '{} – Kunquat Tracker'.format(' '.join(parts))
        else:
            title = 'Kunquat Tracker'
        self.setWindowTitle(title)

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_processor(self._proc_id)

    def sizeHint(self):
        return QSize(1024, 768)



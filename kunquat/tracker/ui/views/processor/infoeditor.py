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

from .processorupdater import ProcessorUpdater


class InfoEditor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._name = Name()

        self.add_to_updaters(self._name)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._name, 0, Qt.AlignTop)
        self.setLayout(v)


class Name(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._edit = QLineEdit()

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.addWidget(QLabel('Name:'))
        h.addWidget(self._edit)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_controls', self._update_name)
        self._update_name()
        QObject.connect(self._edit, SIGNAL('textEdited(QString)'), self._text_edited)

    def _update_name(self):
        old_block = self._edit.blockSignals(True)
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        vis_name = proc.get_name() or ''
        if vis_name != str(self._edit.text()):
            self._edit.setText(vis_name)
        self._edit.blockSignals(old_block)

    def _text_edited(self, text):
        text = str(text)
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        proc.set_name(text)
        self._updater.signal_update('signal_controls')



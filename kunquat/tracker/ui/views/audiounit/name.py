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

from .audiounitupdater import AudioUnitUpdater


class Name(QWidget, AudioUnitUpdater):

    def __init__(self):
        super().__init__()
        self._edit = QLineEdit()

        self.register_action('signal_controls', self._update_name)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.addWidget(QLabel('Name:'))
        h.addWidget(self._edit)
        self.setLayout(h)

    def _on_setup(self):
        self._update_name()
        QObject.connect(self._edit, SIGNAL('textEdited(QString)'), self._text_edited)

    def _update_name(self):
        old_block = self._edit.blockSignals(True)
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        vis_text = au.get_name() or ''
        if vis_text != str(self._edit.text()):
            self._edit.setText(vis_text)
        self._edit.blockSignals(old_block)

    def _text_edited(self, text):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_name(text)
        self._updater.signal_update('signal_controls')



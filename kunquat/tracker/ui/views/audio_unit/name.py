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


class Name(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._au_id = None
        self._updater = None
        self._edit = QLineEdit()

        h = QHBoxLayout()
        h.addWidget(QLabel('Name:'))
        h.addWidget(self._edit)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_name()
        QObject.connect(self._edit, SIGNAL('textEdited(QString)'), self._text_edited)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_controls' in signals:
            self._update_name()

    def _update_name(self):
        old_block = self._edit.blockSignals(True)
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        vis_text = au.get_name() or ''
        if vis_text != str(self._edit.text()):
            self._edit.setText(vis_text)
        self._edit.blockSignals(old_block)

    def _text_edited(self, text_qstring):
        text = str(text_qstring)
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_name(text)
        self._updater.signal_update(set(['signal_controls']))



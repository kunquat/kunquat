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
        self._message = Message()

        self.add_to_updaters(self._name)
        self.add_to_updaters(self._message)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._name, 0)
        v.addWidget(QLabel('Message:'))
        v.addWidget(self._message)
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


class Message(QTextEdit, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self.setAcceptRichText(False)
        font = QFont('monospace', 10)
        font.setStyleHint(QFont.TypeWriter)
        self.document().setDefaultFont(font)

    def _get_update_signal_type(self):
        return 'signal_proc_message_{}'.format(self._proc_id)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_message)
        QObject.connect(self, SIGNAL('textChanged()'), self._change_message)
        self._update_message()

    def _update_message(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        msg = proc.get_message()

        if msg != self.toPlainText():
            old_block = self.blockSignals(True)
            self.setPlainText(msg)
            self.blockSignals(old_block)

    def _change_message(self):
        text = self.toPlainText()

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        proc.set_message(text)

        self._updater.signal_update(self._get_update_signal_type())



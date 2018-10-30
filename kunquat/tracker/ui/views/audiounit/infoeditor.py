# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .audiounitupdater import AudioUnitUpdater


class InfoEditor(QWidget, AudioUnitUpdater):

    def __init__(self):
        super().__init__()
        self._name = Name()
        self._message = Message()

        self.add_to_updaters(self._name)
        self.add_to_updaters(self._message)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._name)
        v.addWidget(QLabel('Message:'))
        v.addWidget(self._message)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))


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
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()
        self._update_name()
        self._edit.textEdited.connect(self._text_edited)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

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


class Message(QTextEdit, AudioUnitUpdater):

    def __init__(self):
        super().__init__()
        self.setAcceptRichText(False)

    def _get_update_signal_type(self):
        return 'signal_au_message_{}'.format(self._au_id)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_message)
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()
        self.textChanged.connect(self._change_message)
        self._update_message()

    def _update_style(self):
        self.setStyleSheet('QTextEdit { font-family: monospace; }')

    def _update_message(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        msg = au.get_message()

        if msg != self.toPlainText():
            old_block = self.blockSignals(True)
            self.setPlainText(msg)
            self.blockSignals(old_block)

    def _change_message(self):
        text = self.toPlainText()

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_message(text)

        self._updater.signal_update(self._get_update_signal_type())



# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.model.keymapmanager import KeyboardNoteAction
from .updater import Updater
from .utils import get_default_font_info


class HelpView(QWidget, Updater):

    def __init__(self):
        super().__init__()

        self._text = Browser(self._get_help_data)
        self.add_to_updaters(self._text)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._text)
        self.setLayout(v)

    # Protected interface

    def _get_help_data(self):
        raise NotImplementedError


class Browser(QTextBrowser, Updater):

    def __init__(self, get_help_data):
        super().__init__()
        self.setAcceptRichText(True)
        self.setReadOnly(True)
        self.setOpenExternalLinks(True)

        self._get_help_data = get_help_data

        self._pos_norm = 0

        doc = QTextDocument()
        self.setDocument(doc)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)

        self.verticalScrollBar().valueChanged.connect(self._on_position_changed)
        self.verticalScrollBar().rangeChanged.connect(self._on_range_changed)
        self.textChanged.connect(self._update_relative_pos)

        self._update_style()
        self.moveCursor(QTextCursor.Start)

    def _fill_help_vars(self, help_fmt):
        kwargs = {
            'actionkey': _ActionKeyHelper(self._ui_model.get_keymap_manager()),
        }

        doc = help_fmt.format(**kwargs)
        return doc

    def _on_range_changed(self, new_min, new_max):
        self._update_relative_pos()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        _, font_size = get_default_font_info(style_mgr)
        style_sheet = style_mgr.get_help_style(font_size)

        help_doc = self._fill_help_vars(self._get_help_data())

        self.document().setDefaultStyleSheet(style_sheet)
        self.document().setHtml(help_doc)

    def _update_relative_pos(self):
        scrollbar = self.verticalScrollBar()
        scroll_max = scrollbar.maximum()
        if scroll_max > 0:
            end_pos = scrollbar.pageStep() + scroll_max
            new_pos = int(self._pos_norm * end_pos)
        else:
            new_pos = 0
        scrollbar.setValue(new_pos)

    def _on_position_changed(self, pos):
        scrollbar = self.verticalScrollBar()
        scroll_max = scrollbar.maximum()
        if scroll_max > 0:
            end_pos = scroll_max + scrollbar.pageStep()
            self._pos_norm = pos / end_pos


class _ActionKeyHelper():

    def __init__(self, keymap_mgr):
        self._keymap_mgr = keymap_mgr

    def __getitem__(self, name):
        if name.startswith('note'):
            action = None
            loc_str = name[4:]
            parts = loc_str.split('_')
            if len(parts) == 2:
                row_str, index_str = parts
                try:
                    row = int(row_str)
                    index = int(index_str)
                    action = KeyboardNoteAction(row, index)
                except ValueError:
                    pass
        else:
            action = name

        key_name = self._keymap_mgr.get_key_name(
                self._keymap_mgr.get_action_location(action))
        if key_name:
            return '<span class="key">{}</span>'.format(key_name)

        return '‽'



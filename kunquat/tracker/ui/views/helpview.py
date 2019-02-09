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

        self._get_help_data = get_help_data

        doc = QTextDocument()
        self.setDocument(doc)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        _, font_size = get_default_font_info(style_mgr)
        style_sheet = style_mgr.get_help_style(font_size)

        help_data = self._get_help_data()

        self.document().setDefaultStyleSheet(style_sheet)
        self.document().setHtml(help_data)

        # TODO: try to show the same top row when font size changes
        self.moveCursor(QTextCursor.Start)



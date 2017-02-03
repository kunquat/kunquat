# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2014
#          Tomi Jylhä-Ollila, Finland 2015-2017
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

from .chdefaultseditor import ChDefaultsEditor
from .orderlisteditor import OrderlistEditor
from .songeditor import SongEditor
from .updater import Updater


class SongsChannelsWindow(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._orderlist_editor = OrderlistEditor()
        self._song_editor = SongEditor()
        self._ch_defaults_editor = ChDefaultsEditor()

        self.add_updating_child(
                self._orderlist_editor, self._song_editor, self._ch_defaults_editor)

        self.setWindowTitle('Songs & channels')

        h = QHBoxLayout()
        h.setContentsMargins(4, 4, 4, 4)
        h.setSpacing(4)
        h.addWidget(self._orderlist_editor)
        h.addWidget(self._song_editor)
        h.addWidget(self._ch_defaults_editor)
        self.setLayout(h)

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_songs_channels()

    def sizeHint(self):
        return QSize(1024, 600)



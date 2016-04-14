# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2014
#          Tomi Jylhä-Ollila, Finland 2015-2016
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

from .chdefaultseditor import ChDefaultsEditor
from .orderlisteditor import OrderlistEditor
from .songeditor import SongEditor


class SongsChannelsWindow(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None

        self._orderlist_editor = OrderlistEditor()
        self._song_editor = SongEditor()
        self._ch_defaults_editor = ChDefaultsEditor()

        self.setWindowTitle('Songs & channels')

        h = QHBoxLayout()
        h.setMargin(4)
        h.setSpacing(4)
        h.addWidget(self._orderlist_editor)
        h.addWidget(self._song_editor)
        h.addWidget(self._ch_defaults_editor)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._orderlist_editor.set_ui_model(ui_model)
        self._song_editor.set_ui_model(ui_model)
        self._ch_defaults_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._ch_defaults_editor.unregister_updaters()
        self._song_editor.unregister_updaters()
        self._orderlist_editor.unregister_updaters()

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_songs_channels()

    def sizeHint(self):
        return QSize(1024, 600)



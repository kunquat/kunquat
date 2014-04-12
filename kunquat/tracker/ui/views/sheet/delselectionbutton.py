# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
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


class DelSelectionButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setText('Del')
        self.setToolTip('Delete selection (Delete)')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_selection' in signals:
            self._update_enabled()

    def _update_enabled(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()

        has_trigger = False

        module = self._ui_model.get_module()
        album = module.get_album()

        if album and album.get_track_count() > 0:
            cur_song = album.get_song_by_track(location.get_track())
            cur_pattern = cur_song.get_pattern_instance(
                    location.get_system()).get_pattern()
            cur_column = cur_pattern.get_column(location.get_col_num())
            has_trigger = cur_column.has_trigger(
                    location.get_row_ts(), location.get_trigger_index())

        self.setEnabled(has_trigger)



# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .eventlist import EventList
from .updater import Updater


class EventListWindow(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._event_list = EventList()

        self.add_to_updaters(self._event_list)

        self.setWindowTitle('Event log')

        v = QVBoxLayout()
        v.addWidget(self._event_list)
        self.setLayout(v)

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_event_log()

    def sizeHint(self):
        return QSize(600, 768)



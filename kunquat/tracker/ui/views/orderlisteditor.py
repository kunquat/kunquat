# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
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

from kunquat.tracker.ui.model.patterninstance import PatternInstance
from kunquat.tracker.ui.model.song import Song
from orderlist import Orderlist


class OrderlistEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None
        self._album = None
        self._orderlist_manager = None
        self._orderlist = Orderlist()
        self._toolbar = OrderlistToolBar()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._orderlist)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        module = ui_model.get_module()
        self._album = module.get_album()
        self._orderlist_manager = ui_model.get_orderlist_manager()
        self._orderlist.set_ui_model(ui_model)
        self._toolbar.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._toolbar.unregister_updaters()
        self._orderlist.unregister_updaters()

    def _handle_insert(self):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, PatternInstance):
            track_num, system_num = self._album.get_pattern_instance_location(selection)
            new_pattern_num = self._album.get_new_pattern_num()
            self._album.insert_pattern_instance(
                    track_num, system_num, new_pattern_num, 0)
            self._orderlist_manager.set_orderlist_selection((track_num, system_num))
            self._updater.signal_update(set(['signal_order_list']))

    def keyPressEvent(self, event):
        if event.modifiers() == Qt.NoModifier:
            if event.key() == Qt.Key_Insert:
                self._handle_insert()
                return
        event.ignore()


class OrderlistToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._updater = None

        self._add_button = QToolButton()
        self._add_button.setText('Add pattern')
        self._add_button.setToolTip('Add pattern (Insert)')

        self.addWidget(self._add_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        module = ui_model.get_module()
        self._album = module.get_album()
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self._add_button, SIGNAL('clicked()'), self._pattern_added)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        # TODO: disable add button if no song is selected or song is full
        pass

    def _pattern_added(self):
        track_num = 0
        song = self._album.get_song_by_track(track_num)
        pattern_num = self._album.get_new_pattern_num()
        self._album.insert_pattern_instance(
                track_num, song.get_system_count(), pattern_num, 0)
        self._updater.signal_update(set(['signal_order_list']))



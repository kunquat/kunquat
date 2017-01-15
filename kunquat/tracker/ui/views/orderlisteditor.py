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

from kunquat.tracker.ui.model.patterninstance import PatternInstance
from kunquat.tracker.ui.model.song import Song
from .headerline import HeaderLine
from .orderlist import Orderlist


class OrderlistEditor(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._album = None
        self._orderlist_manager = None
        self._orderlist = Orderlist()
        self._toolbar = OrderlistToolBar(self._orderlist)

        self._waiting_for_update = False

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Order list'))
        v.addWidget(self._toolbar)
        v.addWidget(self._orderlist)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        module = ui_model.get_module()
        self._album = module.get_album()
        self._orderlist_manager = ui_model.get_orderlist_manager()
        self._orderlist.set_ui_model(ui_model)
        self._toolbar.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._toolbar.unregister_updaters()
        self._orderlist.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_order_list' in signals:
            self._waiting_for_update = False

    def _handle_insert_at(self, offset):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, PatternInstance):
            track_num, system_num = self._album.get_pattern_instance_location(selection)
            new_pattern_num = self._album.get_new_pattern_num()
            self._album.insert_pattern_instance(
                    track_num, system_num + offset, new_pattern_num, 0)
            self._orderlist_manager.set_orderlist_selection(
                    (track_num, system_num + offset))
            self._updater.signal_update(set(['signal_order_list']))
            self._waiting_for_update = True

    def _handle_delete(self):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, PatternInstance):
            track_num, system_num = self._album.get_pattern_instance_location(selection)
            song = self._album.get_song_by_track(track_num)
            self._album.remove_pattern_instance(track_num, system_num)
            self._orderlist_manager.set_orderlist_selection(
                    (track_num, min(system_num, song.get_system_count() - 1)))
            self._updater.signal_update(set(['signal_order_list']))
            self._waiting_for_update = True

    def _handle_move_pattern_instance(self, offset):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, PatternInstance):
            track_num, system_num = self._album.get_pattern_instance_location(selection)
            song = self._album.get_song_by_track(track_num)
            new_system_num = system_num + offset
            if 0 <= new_system_num < song.get_system_count():
                self._album.move_pattern_instance(
                        track_num, system_num, track_num, new_system_num)
                self._orderlist_manager.set_orderlist_selection(
                        (track_num, new_system_num))
                self._updater.signal_update(set(['signal_order_list']))
                self._waiting_for_update = True

    def keyPressEvent(self, event):
        if self._waiting_for_update:
            event.ignore()
            return

        if event.modifiers() == Qt.NoModifier:
            if event.key() == Qt.Key_Insert:
                self._handle_insert_at(0)
                return
            elif event.key() == Qt.Key_N:
                self._handle_insert_at(1)
                return
            elif event.key() == Qt.Key_Delete:
                self._handle_delete()
                return
        elif event.modifiers() == Qt.ShiftModifier:
            if event.key() == Qt.Key_Up:
                self._handle_move_pattern_instance(-1)
                return
            elif event.key() == Qt.Key_Down:
                self._handle_move_pattern_instance(1)
                return
        event.ignore()


class OrderlistToolBar(QToolBar):

    def __init__(self, orderlist):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._orderlist_manager = None

        self._orderlist = orderlist
        self._selection = None

        def create_button(text):
            button = QToolButton()
            button.setText(text)
            button.setToolTip(text)
            button.setEnabled(False)
            return button

        self._new_pat_button        = create_button('New pattern')
        self._remove_pat_button     = create_button('Remove pattern')
        self._reuse_pat_button      = create_button('Reuse pattern')
        self._new_song_button       = create_button('New song')
        self._remove_song_button    = create_button('Remove song')

        self.addWidget(self._new_pat_button)
        self.addWidget(self._remove_pat_button)
        self.addWidget(self._reuse_pat_button)
        self.addSeparator()
        self.addWidget(self._new_song_button)
        self.addWidget(self._remove_song_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        module = ui_model.get_module()
        self._album = module.get_album()
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._orderlist_manager = ui_model.get_orderlist_manager()

        icon_bank = ui_model.get_icon_bank()
        def set_icon(button, icon_name):
            icon_path = icon_bank.get_icon_path(icon_name)
            button.setIcon(QIcon(icon_path))

        set_icon(self._new_pat_button, 'new_pattern')
        set_icon(self._remove_pat_button, 'remove_pattern')
        set_icon(self._reuse_pat_button, 'reuse_pattern')
        set_icon(self._new_song_button, 'new_song')
        set_icon(self._remove_song_button, 'remove_song')

        self._update_buttons_enabled(None)

        QObject.connect(self._new_pat_button, SIGNAL('clicked()'), self._pattern_added)
        QObject.connect(
                self._remove_pat_button, SIGNAL('clicked()'), self._pattern_removed)
        QObject.connect(
                self._reuse_pat_button, SIGNAL('clicked()'), self._pattern_reused)
        QObject.connect(
                self._new_song_button, SIGNAL('clicked()'), self._song_added)
        QObject.connect(
                self._remove_song_button, SIGNAL('clicked()'), self._song_removed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        selection = self._orderlist.get_selected_object()

        if selection != self._selection:
            self._update_buttons_enabled(selection)

    def _update_buttons_enabled(self, selection):
        self._selection = selection

        if isinstance(selection, PatternInstance):
            self._new_pat_button.setEnabled(True)
            pinst_loc = self._album.get_pattern_instance_location(selection)
            self._reuse_pat_button.setEnabled(bool(pinst_loc))
            self._remove_pat_button.setEnabled(bool(pinst_loc))
            self._new_song_button.setEnabled(False)
        else:
            self._new_pat_button.setEnabled(False)
            self._reuse_pat_button.setEnabled(False)
            self._remove_pat_button.setEnabled(False)
            self._new_song_button.setEnabled(True)

        if isinstance(selection, Song):
            self._new_pat_button.setEnabled(True)
            self._new_song_button.setEnabled(True)
            self._remove_song_button.setEnabled(True)
        else:
            self._remove_song_button.setEnabled(False)

    def _pattern_added(self):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, PatternInstance):
            track_num, system_num = self._album.get_pattern_instance_location(selection)
        elif isinstance(selection, Song):
            track_num = selection.get_containing_track_number()
            song = self._album.get_song_by_track(track_num)
            system_num = song.get_system_count()
        else:
            return

        pattern_num = self._album.get_new_pattern_num()
        self._album.insert_pattern_instance(
                track_num, system_num, pattern_num, 0)
        self._orderlist_manager.set_orderlist_selection((track_num, system_num))
        self._updater.signal_update(set(['signal_order_list']))

    def _pattern_removed(self):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, PatternInstance):
            track_num, system_num = self._album.get_pattern_instance_location(selection)
            song = self._album.get_song_by_track(track_num)
            self._album.remove_pattern_instance(track_num, system_num)
            self._orderlist_manager.set_orderlist_selection(
                    (track_num, min(system_num, song.get_system_count() - 1)))
            self._updater.signal_update(set(['signal_order_list']))

    def _pattern_reused(self):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, PatternInstance):
            track_num, system_num = self._album.get_pattern_instance_location(selection)
            song = self._album.get_song_by_track(track_num)
            pinst = song.get_pattern_instance(system_num)
            pattern_num = pinst.get_pattern_num()
            instance_num = self._album.get_new_pattern_instance_num(pattern_num)
            self._album.insert_pattern_instance(
                    track_num, system_num + 1, pattern_num, instance_num)
            self._orderlist_manager.set_orderlist_selection(
                    (track_num, system_num + 1))
            self._updater.signal_update(set(['signal_order_list']))

    def _song_added(self):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, Song):
            track_num = selection.get_containing_track_number()
        else:
            track_num = self._album.get_track_count()
        self._album.insert_song(track_num)
        self._updater.signal_update(set(['signal_order_list']))

    def _song_removed(self):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, Song):
            track_num = selection.get_containing_track_number()
            self._album.remove_song(track_num)
            self._updater.signal_update(set(['signal_order_list']))



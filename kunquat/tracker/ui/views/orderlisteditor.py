# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.model.patterninstance import PatternInstance
from kunquat.tracker.ui.model.song import Song
from .headerline import HeaderLine
from .iconbutton import IconButton
from .orderlist import Orderlist
from .updater import Updater


class OrderlistEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._album = None
        self._orderlist_mgr = None
        self._orderlist = Orderlist()
        self._toolbar = OrderlistToolBar(self._orderlist)

        self._waiting_for_update = False

        self._header = HeaderLine('Order list')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header)
        v.addWidget(self._toolbar)
        v.addWidget(self._orderlist)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._orderlist, self._toolbar)
        self.register_action('signal_order_list', self._acknowledge_update)
        self.register_action('signal_style_changed', self._update_style)

        module = self._ui_model.get_module()
        self._album = module.get_album()
        self._orderlist_mgr = self._ui_model.get_orderlist_manager()

        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._header.update_style(style_mgr)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _acknowledge_update(self):
        self._waiting_for_update = False

    def _handle_insert_at(self, offset):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, PatternInstance):
            track_num, system_num = self._album.get_pattern_instance_location(selection)
            new_pattern_num = self._album.get_new_pattern_num()
            self._album.insert_pattern_instance(
                    track_num, system_num + offset, new_pattern_num, 0)
            self._orderlist_mgr.set_orderlist_selection((track_num, system_num + offset))
            self._updater.signal_update('signal_order_list')
            self._waiting_for_update = True

    def _handle_delete(self):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, PatternInstance):
            track_num, system_num = self._album.get_pattern_instance_location(selection)
            song = self._album.get_song_by_track(track_num)
            self._album.remove_pattern_instance(track_num, system_num)
            self._orderlist_mgr.set_orderlist_selection(
                    (track_num, min(system_num, song.get_system_count() - 1)))
            self._updater.signal_update('signal_order_list')
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
                self._orderlist_mgr.set_orderlist_selection((track_num, new_system_num))
                self._updater.signal_update('signal_order_list')
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
        self._orderlist_mgr = None

        self._orderlist = orderlist
        self._selection = None

        def create_button(text):
            button = IconButton(flat=True)
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
        self._orderlist_mgr = ui_model.get_orderlist_manager()

        def setup_button(button, icon_name):
            button.set_ui_model(self._ui_model)
            button.set_icon(icon_name)

        setup_button(self._new_pat_button, 'new_pattern')
        setup_button(self._remove_pat_button, 'remove_pattern')
        setup_button(self._reuse_pat_button, 'reuse_pattern')
        setup_button(self._new_song_button, 'new_song')
        setup_button(self._remove_song_button, 'remove_song')

        self._update_buttons_enabled(None)

        self._new_pat_button.clicked.connect(self._pattern_added)
        self._remove_pat_button.clicked.connect(self._pattern_removed)
        self._reuse_pat_button.clicked.connect(self._pattern_reused)
        self._new_song_button.clicked.connect(self._song_added)
        self._remove_song_button.clicked.connect(self._song_removed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

        self._remove_song_button.unregister_updaters()
        self._new_song_button.unregister_updaters()
        self._reuse_pat_button.unregister_updaters()
        self._remove_pat_button.unregister_updaters()
        self._new_pat_button.unregister_updaters()

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
        self._album.insert_pattern_instance(track_num, system_num, pattern_num, 0)
        self._orderlist_mgr.set_orderlist_selection((track_num, system_num))
        self._updater.signal_update('signal_order_list')

    def _pattern_removed(self):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, PatternInstance):
            track_num, system_num = self._album.get_pattern_instance_location(selection)
            song = self._album.get_song_by_track(track_num)
            self._album.remove_pattern_instance(track_num, system_num)
            self._orderlist_mgr.set_orderlist_selection(
                    (track_num, min(system_num, song.get_system_count() - 1)))
            self._updater.signal_update('signal_order_list')

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
            self._orderlist_mgr.set_orderlist_selection((track_num, system_num + 1))
            self._updater.signal_update('signal_order_list')

    def _song_added(self):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, Song):
            track_num = selection.get_containing_track_number()
        else:
            track_num = self._album.get_track_count()
        self._album.insert_song(track_num)
        self._updater.signal_update('signal_order_list')

    def _song_removed(self):
        selection = self._orderlist.get_selected_object()
        if isinstance(selection, Song):
            track_num = selection.get_containing_track_number()
            self._album.remove_song(track_num)
            self._updater.signal_update('signal_order_list')



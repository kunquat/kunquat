# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2014
#          Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import json

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.tracker.ui.model.patterninstance import PatternInstance
from kunquat.tracker.ui.model.song import Song


class AlbumTreeModelNode():

    def __init__(self, payload, parent=None):
        self._payload = payload
        self._parent = parent
        self._children = []

    def add_child(self, child):
        self._children.append(child)

    def get_children(self):
        return self._children

    def is_song_node(self):
        return isinstance(self._payload, Song)

    def is_pattern_instance_node(self):
        return isinstance(self._payload, PatternInstance)

    def get_payload(self):
        return self._payload

    def get_parent(self):
        return self._parent


class AlbumTreeModel(QAbstractItemModel):

    def __init__(self):
        super().__init__()

        # We store the nodes because of reference handling issues
        self._songs = []

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._module = self._ui_model.get_module()
        self._album = self._module.get_album()
        self._make_nodes()

    def _make_nodes(self):
        for track_num in range(self._album.get_track_count()):
            song = self._album.get_song_by_track(track_num)
            song_node = AlbumTreeModelNode(song)
            for system_num in range(song.get_system_count()):
                pat_instance = song.get_pattern_instance(system_num)
                pat_inst_node = AlbumTreeModelNode(pat_instance, song_node)
                song_node.add_child(pat_inst_node)
            self._songs.append(song_node)

    def get_song_index(self, track_num):
        return self.createIndex(track_num, 0, self._songs[track_num])

    def get_pattern_index(self, track_num, system_num):
        song_node = self._songs[track_num]
        return self.createIndex(system_num, 0, song_node.get_children()[system_num])

    # Qt interface

    def columnCount(self, _):
        return 1

    def rowCount(self, parent):
        if not parent.isValid():
            # album, count tracks
            track_count = len(self._songs)
            return track_count
        node = parent.internalPointer()
        if node.is_song_node():
            # track, count systems
            return len(node.get_children())
        elif node.is_pattern_instance_node():
            # system, no children
            return 0
        else:
            assert False

    def index(self, row, col, parent):
        if not parent.isValid():
            # album, row indicates track
            if row >= len(self._songs):
                return QModelIndex()
            node = self._songs[row]
        else:
            # song, row indicates system
            parent_node = parent.internalPointer()
            assert parent_node.is_song_node()
            children = parent_node.get_children()
            if row >= len(children):
                return QModelIndex()
            node = children[row]
        return self.createIndex(row, col, node)

    def parent(self, index):
        if not index.isValid():
            # album, has no parent
            invalid = QModelIndex()
            return invalid
        node = index.internalPointer()
        if node.is_song_node():
            # song, album is the parent
            invalid = QModelIndex()
            return invalid
        elif node.is_pattern_instance_node():
            # pattern instance, some song is the parent
            parent_node = node.get_parent()
            assert parent_node.is_song_node()
            track = parent_node.get_children().index(node)
            return self.createIndex(track, 0, parent_node)
        else:
            assert False

    def data(self, index, role):
        node = index.internalPointer()
        if node.is_song_node():
            if role == Qt.DisplayRole:
                song = node.get_payload()
                song_name = song.get_name() or 'song {}'.format(song.get_number())
                return song_name
        elif node.is_pattern_instance_node():
            if role == Qt.DisplayRole:
                pattern_instance = node.get_payload()
                return pattern_instance.get_name()
        else:
            assert False

    def supportedDropActions(self):
        return Qt.MoveAction

    def flags(self, index):
        default_flags = super().flags(index)
        if not index.isValid():
            return default_flags
        node = index.internalPointer()
        if node.is_song_node() or node.is_pattern_instance_node():
            return Qt.ItemIsDragEnabled | Qt.ItemIsDropEnabled | default_flags
        else:
            assert False

    def mimeTypes(self):
        return ['application/json']

    def _get_item(self, index):
        assert index.isValid()
        node = index.internalPointer()
        if node.is_song_node():
            song = node.get_payload()
            track_num = song.get_containing_track_number()
            return ('song', track_num)
        elif node.is_pattern_instance_node():
            pinst = node.get_payload()
            track_num, system_num = self._album.get_pattern_instance_location(pinst)
            return ('pinst', (track_num, system_num))
        else:
            assert False

    def mimeData(self, index_list):
        items = [self._get_item(index) for index in index_list]
        serialised = json.dumps(items)
        mimedata = QMimeData()
        mimedata.setData('application/json', serialised)
        return mimedata

    def dropMimeData(self, mimedata, action, row, col, parent):
        if action != Qt.MoveAction:
            return False
        if not mimedata.hasFormat('application/json'):
            return False

        data = mimedata.data('application/json')
        items = json.loads(str(data))
        assert len(items) == 1
        item = items[0]

        if item[0] == 'song':
            if row == -1:
                song_index = parent
                if not song_index.isValid():
                    return False
                song_node = song_index.internalPointer()
                if not song_node.is_song_node():
                    return False
                song = song_node.get_payload()
                to_track_num = song.get_containing_track_number()
            else:
                to_track_num = row

            from_track_num = item[1]

            self._album.move_song(from_track_num, to_track_num)
            self._updater.signal_update(set(['signal_order_list']))
            return True

        elif item[0] == 'pinst':
            if not parent.isValid():
                return False

            # Find target track and system
            if row == -1:
                pinst_index = parent
                song_index = pinst_index.parent()
                if not song_index.isValid():
                    return False
                pinst_node = pinst_index.internalPointer()
                if not pinst_node.is_pattern_instance_node():
                    return False
                pinst = pinst_node.get_payload()
                to_track_num, to_system_num = self._album.get_pattern_instance_location(
                        pinst)
            else:
                song_index = parent
                song_node = song_index.internalPointer()
                song = song_node.get_payload()
                to_track_num = song.get_containing_track_number()
                to_system_num = row

            from_track_num, from_system_num = item[1]

            self._album.move_pattern_instance(
                    from_track_num, from_system_num, to_track_num, to_system_num)
            self._updater.signal_update(set(['signal_order_list']))
            return True


class AlbumTree(QTreeView):

    def __init__(self):
        super().__init__()
        self.setSelectionMode(QAbstractItemView.SingleSelection)
        self.setDragEnabled(True)
        self.setAcceptDrops(True)
        self.setDropIndicatorShown(True)

    def keyPressEvent(self, event):
        if Qt.Key_A <= event.key() <= Qt.Key_Z:
            event.ignore()
        elif ((event.modifiers() == Qt.ShiftModifier) and
                (event.key() in (Qt.Key_Up, Qt.Key_Down))):
            event.ignore()
        else:
            super().keyPressEvent(event)


class Orderlist(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._album = None
        self._orderlist_manager = None
        self._album_tree_model = None

        self._album_tree = AlbumTree()
        self._album_tree.setHeaderHidden(True)
        self._album_tree.setRootIsDecorated(True)

        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        layout.addWidget(self._album_tree)

        self.setLayout(layout)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        module = ui_model.get_module()
        self._album = module.get_album()
        self._orderlist_manager = ui_model.get_orderlist_manager()
        self._update_model()

    def _update_model(self):
        self._album_tree_model = AlbumTreeModel()
        self._album_tree_model.set_ui_model(self._ui_model)
        self._album_tree.setModel(self._album_tree_model)
        self._album_tree.expandAll()

        # Fix selection
        selection = self._orderlist_manager.get_orderlist_selection()
        if selection != None:
            if type(selection) == tuple:
                track_num, system_num = selection
                if track_num >= self._album.get_track_count():
                    system_num = 0
                    track_num = self._album.get_track_count() - 1
                if track_num < 0:
                    self._orderlist_manager.set_orderlist_selection(None)
                else:
                    song = self._album.get_song_by_track(track_num)
                    system_num = min(system_num, song.get_system_count() - 1)
                    if system_num < 0:
                        self._orderlist_manager.set_orderlist_selection(None)
                    else:
                        self._orderlist_manager.set_orderlist_selection(
                                (track_num, system_num))
                        index = self._album_tree_model.get_pattern_index(
                                track_num, system_num)
                        self._album_tree.setCurrentIndex(index)
            else:
                track_num = selection
                track_num = min(track_num, self._album.get_track_count() - 1)
                if track_num < 0:
                    self._orderlist_manager.set_orderlist_selection(None)
                else:
                    index = self._album_tree_model.get_song_index(track_num)
                    self._album_tree.setCurrentIndex(index)

        QObject.connect(
            self._album_tree.selectionModel(),
            SIGNAL('currentChanged(const QModelIndex&, const QModelIndex&)'),
            self._change_selection)

    def get_selected_object(self):
        selection_model = self._album_tree.selectionModel()
        index = selection_model.currentIndex()
        if not index.isValid():
            return None

        node = index.internalPointer()
        if not node:
            return None

        obj = node.get_payload()
        return obj

    def _perform_updates(self, signals):
        if 'signal_order_list' in signals:
            self._update_model()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _change_selection(self, current, previous):
        if not current.isValid():
            return

        node = current.internalPointer()
        if node.is_pattern_instance_node():
            node = node.get_parent()
        if node.is_song_node():
            song = node.get_payload()
            album = self._ui_model.get_module().get_album()
            album.set_selected_track_num(song.get_containing_track_number())
            self._updater.signal_update(set(['signal_song']))



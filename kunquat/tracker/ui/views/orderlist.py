# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2014
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

class AlbumTreeModelNode(object):

    def __init__(self, payload, parent=None):
       self._payload = payload
       self._parent = parent

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
        QAbstractItemModel.__init__(self)

        # we store the nodes because PyQT fails reference handling
        self._nodes = []

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._module = self._ui_model.get_module()
        self._album = self._module.get_album()

    # override
    def columnCount(self, _):
        return 1

    # override
    def rowCount(self, parent):
        if not parent.isValid():
            # album, count tracks
            track_count = self._album.get_track_count()
            return track_count
        node = parent.internalPointer()
        if node.is_song_node():
            # track, count systems
            song = node.get_payload()
            return song.get_system_count()
        elif node.is_pattern_instance_node():
            # system, no children
            return 0
        else:
            assert False

    # override
    def index(self, row, col, parent):
        if not parent.isValid():
            # album, row indicates track
            payload = self._album.get_song_by_track(row)
            node = AlbumTreeModelNode(payload)
        else:
            # song, row indicates system
            parent_node = parent.internalPointer()
            assert parent_node.is_song_node()
            song = parent_node.get_payload()
            payload = song.get_pattern_instance(row)
            node = AlbumTreeModelNode(payload, parent_node)
        self._nodes.append(node)
        return self.createIndex(row, col, node)

    # override
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
            song = parent_node.get_payload()
            track = song.get_containing_track_number()
            return self.createIndex(track, 0, parent_node)
        else:
            assert False

    # override
    def data(self, index, role):
        node = index.internalPointer()
        if node.is_song_node():
            if role == Qt.DisplayRole:
                song = node.get_payload()
                song_name = song.get_name()
                return song_name
        elif node.is_pattern_instance_node():
            if role == Qt.DisplayRole:
                pattern_instance = node.get_payload()
                return pattern_instance.get_name()
        else:
            assert False


class AlbumTree(QTreeView):

    def __init__(self):
        QTreeView.__init__(self)

class Orderlist(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None
        self._album_tree_model = None

        self._album_tree = AlbumTree()
        self._album_tree.setHeaderHidden(True)
        self._album_tree.setRootIsDecorated(True)

        layout = QVBoxLayout()
        layout.setMargin(0)
        layout.setSpacing(0)
        layout.addWidget(self._album_tree)

        self.setLayout(layout)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._album_tree_model = AlbumTreeModel()
        self._album_tree_model.set_ui_model(ui_model)
        self._album_tree.setModel(self._album_tree_model)
        self._album_tree.expandAll()

    def _perform_updates(self, signals):
        pass

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

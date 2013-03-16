# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2012
#          Tomi Jylhä-Ollila, Finland 2010-2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division
from __future__ import print_function

from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import Qt

import kunquat.tracker.kqt_limits as lim
import json

class AlbumTree(QtGui.QTreeView):

    def __init__(self, song_view):
        QtGui.QTreeView.__init__(self)
        self.song_view = song_view

    def currentChanged(self, new_index, old_index):
        QtGui.QTreeView.currentChanged(self, new_index, old_index)
        return self.song_view.currentChanged(new_index, old_index)

    def dragEnterEvent(self, e):
        QtGui.QTreeView.dragEnterEvent(self, e)
        self.song_view.drag_enter(e)

class Subsongs(QtGui.QWidget):

    def __init__(self, p, section):
        QtGui.QWidget.__init__(self)
        self.p = p
        self._section_manager = section

        song_list = AlbumTree(self)
        song_list.setHeaderHidden(True)
        song_list.setRootIsDecorated(True)
        song_list.setDragDropMode(QtGui.QAbstractItemView.InternalMove)
        song_list.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        song_list.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self._song_list = song_list

        layout = QtGui.QVBoxLayout()
        layout.setMargin(0)
        layout.setSpacing(0)
        layout.addWidget(song_list)

        self.setLayout(layout)

    def select_track(self, track):
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('subsongParams(int)'),
                            track)
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('subsongChanged(int)'),
                            track)

    def init(self):
        self.update()
        self._song_list.expandAll()

    def deal_with(self, node):
        if isinstance(node, Song_node):
            song = node.song
            track = self.p.project._composition.get_track_by_song(song)
            self.select_track(track)
        elif isinstance(node, Pattern_instance_node):
            parent = node.parent
            song = parent.song
            track = self.p.project._composition.get_track_by_song(song)
            system = node.system
            self._section_manager.set(track, system)
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('subsongChanged(int)'),
                                track)
            track = self.p.project._composition.get_track_by_song(song)
            self.select_track(track)
        else:
            assert False

    def currentChanged(self, new_index, old_index):
        node = new_index.internalPointer()
        self.deal_with(node)

    def drag_enter(self, e):
        data = e.mimeData()
        if not data.hasFormat('application/json'):
            e.ignore()
            return False
        serials = data.data('application/json')
        nodes = json.loads(str(serials))
        assert len(nodes) == 1
        node = nodes[0]
        (node_type, _) = node
        if node_type == 'pi':
            self._model.setSongDrag(False)
        elif node_type == 'song':
            self._model.setSongDrag(True)

    def update_model(self):
        # this is expected to be lighter,
        # but does not work at the moment
        #self._model.update()
        self.update()

    def update(self):
        project = self.p.project
        songs = self.p.project._composition.song_ids()
        self._model = OrderList(self.p, self)
        self._song_list.setModel(self._model)

class Pattern_instance_node(object):
    def __init__(self, parent, system):
        self.parent = parent
        self.system = system

    @property
    def pattern_instance(self):
        song = self.parent.song
        system = self.system
        pattern_instance = song.get_pattern_instance(system)
        return pattern_instance

class Song_node(object):
    def __init__(self, song):
        self.song = song

class OrderList(QtCore.QAbstractItemModel):

    def __init__(self, p, view):
        QtCore.QAbstractItemModel.__init__(self)
        self.p = p
        self.rubbish = []
        self.songdrag = False
        self.view = view

    def columnCount(self, _):
        return 1

    def _song_count(self):
        count = self.p.project._composition.song_count()
        return count

    def _system_count(self, song_node):
        song = song_node.song
        count = song.system_count()
        return count

    def rowCount(self, parent):
        if not parent.isValid():
            return self._song_count()
        node = parent.internalPointer()
        if isinstance(node, Pattern_instance_node):
            return 0
        elif isinstance(node, Song_node):
            return self._system_count(node)
        else:
            assert False

    def _song_index(self, row, col):
        track = row
        song = self.p.project._composition.get_song_by_track(track)
        sno = Song_node(song)
        self.rubbish.append(sno)
        return self.createIndex(row, col, sno)

    def _pattern_instance_index(self, row, col, parent):
        parent = parent.internalPointer()
        system = row
        pi = Pattern_instance_node(parent, system)
        self.rubbish.append(pi)
        return self.createIndex(row, col, pi)

    def index(self, row, col, parent):
        if not parent.isValid():
            return self._song_index(row, col)
        return self._pattern_instance_index(row, col, parent)

    def parent(self, index):
        if not index.isValid():
            return QtCore.QModelIndex()
        node = index.internalPointer()
        if isinstance(node, Song_node):
            return QtCore.QModelIndex()
        elif isinstance(node, Pattern_instance_node):
            parent = node.parent
            song = parent.song
            track = self.p.project._composition.get_track_by_song(song)
            return self.createIndex(track, 0, parent)
        else:
            assert False

    def _song_data(self, index, role):
        if role == QtCore.Qt.DisplayRole:
            song_node = index.internalPointer()
            song = song_node.song
            song_name = song.get_name()
            return song_name

    def _pattern_instance_data(self, index, role):
        if role == QtCore.Qt.DisplayRole:
            pino = index.internalPointer()
            parent = pino.parent
            return pino.pattern_instance.get_name()

    def data(self, index, role):
        node = index.internalPointer()
        if isinstance(node, Song_node):
            return self._song_data(index, role)
        elif isinstance(node, Pattern_instance_node):
            return self._pattern_instance_data(index, role)
        else:
            assert False

    def setSongDrag(self, b):
        self.songdrag = b

    def _root_flags(self):
        default = Qt.ItemIsEnabled | Qt.ItemIsSelectable | Qt.ItemIsDragEnabled
        drop_target = default | Qt.ItemIsDropEnabled
        if self.songdrag:
            flagval = drop_target
        else:
            flagval = default
        flagses = Qt.ItemFlags(flagval)
        return flagses

    def _song_flags(self):
        default = Qt.ItemIsEnabled | Qt.ItemIsSelectable | Qt.ItemIsDragEnabled
        drop_target = default | Qt.ItemIsDropEnabled
        if self.songdrag:
            flagval = default
        else: 
            flagval = drop_target
        flagses = Qt.ItemFlags(flagval)
        return flagses

    def _pattern_instance_flags(self):
        default = Qt.ItemIsEnabled | Qt.ItemIsSelectable | Qt.ItemIsDragEnabled
        flagses = Qt.ItemFlags(default)
        return flagses

    def flags(self, index):
        if not index.isValid():
            return self._root_flags()
        parent = index.parent()
        if parent.isValid():
            return self._pattern_instance_flags()
        return self._song_flags()

    def supportedDropActions(self):
        return QtCore.Qt.MoveAction

    def mimeTypes(self):
        return ['application/json']

    def _item_pair(self, index):
        assert index.isValid()
        node = index.internalPointer()
        if isinstance(node, Song_node):
            sno = node
            song = sno.song
            track = self.p.project._composition.get_track_by_song(song)
            return ('song', track)
        elif isinstance(node, Pattern_instance_node):
            pino = node
            parent_sno = pino.parent
            parent_song = parent_sno.song
            track = self.p.project._composition.get_track_by_song(parent_song)
            system = pino.system
            return ('pi', (track, system))
        else:
            assert False

    def mimeData(self, indexes):
        serials = [self._item_pair(i) for i in indexes]
        data = json.dumps(serials)
        mimedata = QtCore.QMimeData()
        mimedata.setData('application/json', data)
        return mimedata

    def dropMimeData(self, data, action, row, column, parent):
        if not action == QtCore.Qt.MoveAction:
            return False
        if not data.hasFormat('application/json'):
            return False
        #self.emit(QtCore.SIGNAL("layoutAboutToBeChanged()"))
        serials = data.data('application/json')
        nodes = json.loads(str(serials))
        assert len(nodes) == 1
        node = nodes[0]
        (node_type, node_data) = node
        composition = self.p.project._composition
        if node_type == 'song':
            assert not parent.isValid()
            track = node_data
            if row < 0:
                target = composition.song_count()
            else:
                target = row
            composition.move_track(track, target)
        elif node_type == 'pi':
            assert parent.isValid()
            global_system = tuple(node_data)
            parent_sno = parent.internalPointer()
            parent_song = parent_sno.song
            target_track = self.p.project._composition.get_track_by_song(parent_song)
            if row < 0:
                target_row = parent_song.system_count()
            else:
                target_row = row
            global_target = (target_track, target_row)
            composition.move_system(global_system, global_target)
        else:
           assert False
        return True

    def update(self):
        #self.view.update()
        self.emit(QtCore.SIGNAL("layoutChanged()"))


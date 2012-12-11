# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2012
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
        self._song_list = song_list

        layout = QtGui.QVBoxLayout()
        layout.setMargin(0)
        layout.setSpacing(0)
        layout.addWidget(song_list)

        self.setLayout(layout)

    def init(self):
        self.update()

    def deal_with(self, node):
        if isinstance(node, Song_node):
            track = node.track
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('subsongParams(int)'),
                                track)
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('subsongChanged(int)'),
                                track)
        elif isinstance(node, Pattern_instance_node):
            parent = node.parent
            song = parent.song
            track = parent.track
            pattern = node.pattern_instance.pattern
            self._section_manager.set(track, pattern)
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('subsongChanged(int)'),
                                track)
            self.deal_with(parent)
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
        '''
        if item_type == 'system':
            self.pattern_drag()
        elif item_type == 'song':
            self.song_drag()
        '''

    def update(self):
        project = self.p.project
        songs = self.p.project._composition.song_ids()
        model = OrderList(self.p)

        self._song_list.setModel(model)
        self._song_list.expandAll()

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
    def __init__(self, track, song):
        self.track = track
        self.song = song

class OrderList(QtCore.QAbstractItemModel):

    def __init__(self, p):
        QtCore.QAbstractItemModel.__init__(self)
        self.p = p
        self.rubbish = []

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
        somo = Song_node(track, song)
        self.rubbish.append(somo)
        return self.createIndex(row, col, somo)

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
            return self.createIndex(parent.track, 0, parent)
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
            pimo = index.internalPointer()
            parent = pimo.parent
            song = parent.song
            return pimo.pattern_instance.get_name(song)

    def data(self, index, role):
        node = index.internalPointer()
        if isinstance(node, Song_node):
            return self._song_data(index, role)
        elif isinstance(node, Pattern_instance_node):
            return self._pattern_instance_data(index, role)
        else:
            assert False

    def flags(self, index):
        flagval = QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsDragEnabled | QtCore.Qt.ItemIsDropEnabled | QtCore.Qt.ItemIsEnabled
        flagses = QtCore.Qt.ItemFlags(flagval)
        return flagses

    def supportedDropActions(self):
        return QtCore.Qt.MoveAction

    def mimeTypes(self):
        return ['application/json']

    def _item_pair(self, index):
        node = index.internalPointer()
        if isinstance(node, Song_node):
            songmo = node
            return songmo.song.get_ref()
        elif isinstance(node, Pattern_instance_node):
            pimo = node
            return pimo.pattern_instance.get_ref()
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
        serials = data.data('application/json')
        print('%s dropped at %s %s' % (serials, row, parent) )
        return True


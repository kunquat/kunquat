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

class AlbumTree(QtGui.QTreeView):

    def __init__(self, song_view):
        QtGui.QTreeView.__init__(self)
        self.song_view = song_view

    def currentChanged(self, new_index, old_index):
        return self.song_view.currentChanged(new_index, old_index)

    def dragEnterEvent(self, e):
        QtGui.QTreeView.dragEnterEvent(self, e)
        self.song_view.drag_enter(e)

class PIMo(object):
    def __init__(self, song_number, songmo):
        self.song_number = song_number
        self.songmo = songmo

class SongMo(object):
    def __init__(self, song_number, song):
        self.song_number = song_number
        self.song = song

class Subsongs(QtGui.QWidget):
    '''
    comp_signal = QtCore.pyqtSignal(name='compositionParams')
    subsong_params = QtCore.pyqtSignal(int, name='subsongParams')
    subsong_changed = QtCore.pyqtSignal(int, name='subsongChanged')
    '''

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

    @property
    def _songs(self):
        song_count = self._root.rowCount()
        songs = set()
        for i in range(song_count):
            song = self._root.child(i, 0)
            songs.add(song)
        return songs


    def data_from_item(self, item):
        data = item.data()
        obj = data.toPyObject()
        items = obj.items()
        pyitems = [(str(a), b) for (a, b) in items]
        pydict = dict(pyitems)
        return pydict

    def deal_with(self, item):
        item_data = self.data_from_item(item)
        item_id = item_data['type']
	parts = item_id.split('_')
        item_type = parts[0]
        if item_type == 'song':
            song_number = int(parts[1])
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('subsongParams(int)'),
                                song_number)
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('subsongChanged(int)'),
                                song_number)
        elif item_type == 'system':
            pattern_number = int(parts[1])
            song_item = item.parent()
            song_data = self.data_from_item(song_item)
            song_id = song_data['type']
	    song_id_parts = song_id.split('_')
            song_type = song_id_parts[0]
            song_number = int(song_id_parts[1])
            self._section_manager.set(song_number, pattern_number)
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('subsongChanged(int)'),
                                song_number)
            parent = item.parent()
            self.deal_with(parent)
        else:
            assert False

    def currentChanged(self, new_index, old_index):
        model = new_index.model()
	item = model.itemFromIndex(new_index)
        self.deal_with(item)

    def subscript(self, number):
        nums = [int(i) for i in str(number)]
        subs = [unichr(0x2080 + i) for i in nums]
        return u''.join(subs)

    def pattern_instance_name(self, patterns, pattern_instance):
        pattern, instance = pattern_instance
        pname = u'pattern {0}'.format(pattern)
        if len([i for i in patterns if i == pattern]) > 1:
             piname = pname + self.subscript(instance)
        else:
             piname = pname
        return piname

    def create_systems(self, order_list):
        patterns = [pattern for pattern, _ in order_list]
        for system_number, pattern_instance in enumerate(order_list):
            piname = self.pattern_instance_name(patterns, pattern_instance)
            pattern_item = QtGui.QStandardItem(piname)
            pattern_item.setEditable(False)
            pattern_item.setDropEnabled(False)
            system_id = u'system_{0}'.format(system_number)
            pattern_item.setData({'type':system_id})
            yield pattern_item

    def create_songs(self, song_ids):
        for song_number, song_id in enumerate(song_ids):
            song = self.p.project._composition.get_song(song_id)
            song_name = song.get_name()
            song_name = 'song {0}'.format(song_number)
            st = '{1}'.format(song_number, song_name)
            song_item = QtGui.QStandardItem(st)
            song_item.setDragEnabled(True)
            song_item.setEditable(False)
            song_item.setData({'type':song_id})
            order_list = song.get_order_list()
            for pattern_item in self.create_systems(order_list):
                song_item.appendRow(pattern_item)
            yield song_item

    def song_drag(self):
        self._root.setDropEnabled(True)
        for song in self._songs:
            song.setDropEnabled(False)

    def pattern_drag(self):
        self._root.setDropEnabled(False)
        for song in self._songs:
            song.setDropEnabled(True)

    def drag_enter(self, e):
        mt = 'application/x-qabstractitemmodeldatalist'
        md = e.mimeData()
        if not md.hasFormat(mt): # invalid type
            e.ignore()
        model = QtGui.QStandardItemModel()
        model.dropMimeData(md, QtCore.Qt.CopyAction, 0,0, QtCore.QModelIndex())
        root = model.invisibleRootItem()
        rc = root.rowCount()
        if rc != 1: # multi drag
            e.ignore()
        item = root.child(0,0)
        item_data = self.data_from_item(item)
        item_id = item_data['type']
	parts = item_id.split('_')
        item_type = parts[0]
        if item_type == 'system':
            self.pattern_drag()
        elif item_type == 'song':
            self.song_drag()

    def update(self):
        project = self.p.project
        songs = self.p.project._composition.song_ids()
        model = OrderList(self.p)

        #root = model.invisibleRootItem()
        #self._root = root
        #self._model = model
        #for song_item in self.create_songs(songs):
        #    root.appendRow(song_item)
        self._song_list.setModel(model)
        self._song_list.expandAll()

class OrderList(QtCore.QAbstractItemModel):

    def __init__(self, p):
        QtCore.QAbstractItemModel.__init__(self)
        self.p = p
        self.rubbish = []

    def columnCount(self, _):
        return 1

    def _song_count(self):
        songs = self.p.project._composition.song_ids()
        count = len(songs)
        return count

    def _pattern_instance_count(self, parent):
        song_number = parent.row()
        song = self.p.project._composition.get_song('song_%02d' % song_number)
        order_list = song.get_order_list()
        count = len(order_list)
        return count

    def rowCount(self, parent):
        if not parent.isValid():
            return self._song_count()
        maybe_pimo = parent.internalPointer()
        if isinstance(maybe_pimo, PIMo):
            return 0
        return self._pattern_instance_count(parent)

    def _song_index(self, row, col, parent):
        song_number = row
        song = self.p.project._composition.get_song('song_%02d' % song_number)
        somo = SongMo(song_number, song)
        self.rubbish.append(somo)
        return self.createIndex(row, col, somo)

    def _pattern_instance_index(self, row, col, parent):
        song_number = parent.row()
        songmo = parent.internalPointer()
        pi = PIMo(song_number, songmo)
        self.rubbish.append(pi)
        return self.createIndex(row, col, pi)

    def index(self, row, col, parent):
        if not parent.isValid():
            return self._song_index(row, col, parent)
        return self._pattern_instance_index(row, col, parent)

    def parent(self, index):
        if not index.isValid():
            return QtCore.QModelIndex()
        maybe_songmo = index.internalPointer()
        if isinstance(maybe_songmo, SongMo):
            return QtCore.QModelIndex()
        pi = index.internalPointer()
        return self.createIndex(pi.song_number, 0, pi.songmo)

    def _song_data(self, index, role):
        if role == QtCore.Qt.DisplayRole:
            songmo = index.internalPointer()
            song = songmo.song
            song_name = song.get_name()
            return song_name

    def _pattern_instance_data(self, index, role):
        if role == QtCore.Qt.DisplayRole:
            return u'lol'

    def data(self, index, role):
        parent = index.parent()
        maybe_songmo = index.internalPointer()
        if isinstance(maybe_songmo, SongMo):
            return self._song_data(index, role)
        return self._pattern_instance_data(index, role)



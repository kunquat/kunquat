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
    def __init__(self, song_number, songmo, system):
        self.song_number = song_number
        self.songmo = songmo
        self.system = system

class SongMo(object):
    def __init__(self, song_number, song):
        self.song_number = song_number
        self.song = song

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

        if isinstance(node, SongMo):
            song_number = node.song_number
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('subsongParams(int)'),
                                song_number)
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('subsongChanged(int)'),
                                song_number)
        elif isinstance(node, PIMo):

            songmo = node.songmo
            song = songmo.song
            system = node.system
            order_list = song.get_order_list()
            patterns = [i for (i, _) in order_list]
            pattern_instance = order_list[system]
            pattern, instance = pattern_instance
            song_number = songmo.song_number
            self._section_manager.set(song_number, pattern)
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('subsongChanged(int)'),
                                song_number)
            parent = songmo
            self.deal_with(parent)
        else:
            assert False

    def currentChanged(self, new_index, old_index):
        node = new_index.internalPointer()
        self.deal_with(node)

    def update(self):
        project = self.p.project
        songs = self.p.project._composition.song_ids()
        model = OrderList(self.p)

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
        pi = PIMo(song_number, songmo, row)
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

    def subscript(self, number):
        nums = [int(i) for i in str(number)]
        subs = [unichr(0x2080 + i) for i in nums]
        return u''.join(subs)

    def pattern_instance_name(self, pimo):
        song = pimo.songmo.song
        system = pimo.system
        order_list = song.get_order_list()
        patterns = [i for (i, _) in order_list]
        pattern_instance = order_list[system]
        pattern, instance = pattern_instance
        pname = u'pattern {0}'.format(pattern)
        if len([i for i in patterns if i == pattern]) > 1:
             piname = pname + self.subscript(instance)
        else:
             piname = pname
        return piname

    def _pattern_instance_data(self, index, role):
        if role == QtCore.Qt.DisplayRole:
            pimo = index.internalPointer()
            return self.pattern_instance_name(pimo)

    def data(self, index, role):
        parent = index.parent()
        maybe_songmo = index.internalPointer()
        if isinstance(maybe_songmo, SongMo):
            return self._song_data(index, role)
        return self._pattern_instance_data(index, role)



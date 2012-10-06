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

    def __init__(self):
        QtGui.QTreeView.__init__(self)

    def currentChanged(self, new_index, old_index):
        model = new_index.model()
	item = model.itemFromIndex(new_index)
        data = item.data()
        obj = data.toPyObject()
        items = obj.items()
        pyitems = [(str(a), b) for (a, b) in items]
        pydict = dict(pyitems)
        item_type = pydict['type']
        print(item_type)
        '''
        if self._section_signal:
            print('signal loop')
            return
        QtGui.QTreeView.currentChanged(self, new_index, old_index)
        item = self._model.itemFromIndex(new_index)
        if not item:
            return
        parent = item.parent()
        if not parent:
            QtCore.QObject.emit(self, QtCore.SIGNAL('compositionParams()'))
            if self._cur_subsong != -1:
                self._cur_subsong = -1
                QtCore.QObject.emit(self,
                                    QtCore.SIGNAL('subsongChanged(int)'),
                                    self._cur_subsong)
            return
        child = item.child(0)
        if child or not parent.parent():
            if item.row() >= len(self._slists):
                assert item.row() == len(self._slists)
                return
            QtCore.QObject.emit(self, QtCore.SIGNAL('subsongParams(int)'),
                                item.row())
            if self._cur_subsong != item.row():
                self._cur_subsong = item.row()
                QtCore.QObject.emit(self,
                                    QtCore.SIGNAL('subsongChanged(int)'),
                                    self._cur_subsong)
            return
        self._section_signal = True
        self._section_manager.set(parent.row(), item.row())
        self._section_signal = False
        if self._cur_subsong != parent.row():
            self._cur_subsong = parent.row()
            QtCore.QObject.emit(self, QtCore.SIGNAL('subsongChanged(int)'),
                                self._cur_subsong)
    '''


class Subsongs(QtGui.QWidget):
    '''
    comp_signal = QtCore.pyqtSignal(name='compositionParams')
    subsong_params = QtCore.pyqtSignal(int, name='subsongParams')
    subsong_changed = QtCore.pyqtSignal(int, name='subsongChanged')
    '''

    def __init__(self, p):
        QtGui.QWidget.__init__(self)
        self.p = p

        song_list = AlbumTree()
        song_list.setHeaderHidden(True)
        song_list.setRootIsDecorated(False)
        #song_list.setDragDropMode(QtGui.QAbstractItemView.InternalMove)
        #song_list.connect(self.currentChanged)
        self._song_list = song_list

        but_layout = QtGui.QVBoxLayout()
        buttons = QtGui.QWidget()
        buttons.setLayout(but_layout)
        new_song = QtGui.QPushButton('new song')
        new_song = QtGui.QPushButton('delete song')
        duplicate_section = QtGui.QPushButton('duplicate section')
        reuse_section = QtGui.QPushButton('reuse section')
        rm_section = QtGui.QPushButton('remove section')
        new_section = QtGui.QPushButton('new section')
        but_layout.addWidget(new_song)
        but_layout.addWidget(duplicate_section)
        but_layout.addWidget(reuse_section)
        but_layout.addWidget(new_section)
        but_layout.setMargin(0)
        but_layout.setSpacing(0)

        layout = QtGui.QVBoxLayout()
        #layout.setMargin(0)
        layout.setSpacing(0)
        layout.addWidget(song_list)
        layout.addWidget(buttons)

        self.setLayout(layout)

    def init(self):
        self.update()

    def create_systems(self, order_list):
        for i2, pattern in enumerate(order_list):
            system_number = i2 + 1
            ptt = 'System {1}'.format(system_number, pattern)
            pname = str(system_number) + ': ' + str(pattern)
            pattern_item = QtGui.QStandardItem(ptt)
            #pattern_item.setToolTip(ptt)
            pattern_item.setEditable(True)
            pattern_item.setData({'type':'system'})
            yield pattern_item

    def create_songs(self, song_ids):
        for i, song_id in enumerate(song_ids):
            song_number = i + 1
            song = self.p.project._composition.get_song(song_id)
            song_name = song.get_name()
            name = '{0}: {1}'.format(song_number, song_name)
            stt = 'Song {0}: {1}'.format(song_number, song_name)
            song_item = QtGui.QStandardItem(stt)
            #song_item.setToolTip(stt)
            song_item.setEditable(False)
            song_item.setData({'type':'song'})
            order_list = song.get_order_list()
            for pattern_item in self.create_systems(order_list):
                song_item.appendRow(pattern_item)
            yield song_item

    def create_trash(self):
        trash_item = QtGui.QStandardItem('Trash')
        trash_item.setEditable(False)
        trash_item.setData({'type':'trash'})
        trash_ids = self.p.project._composition.left_over_patterns()
        trash = [int(i.split('_')[1]) for i in trash_ids]
        for system_item in self.create_systems(sorted(trash)):
            trash_item.appendRow(system_item)
        return trash_item

    def update(self):
        project = self.p.project
        album_item = QtGui.QStandardItem('untitled album')
        album_item.setEditable(False)
        album_item.setData({'type':'album'})
        songs = self.p.project._composition.song_ids()
        for song_item in self.create_songs(songs):
            album_item.appendRow(song_item)
        trash = self.create_trash()
        album_item.appendRow(trash)
        model = QtGui.QStandardItemModel()
        root = model.invisibleRootItem()
        root.appendRow(album_item)
        self._song_list.setModel(model)
        self._song_list.expandAll()






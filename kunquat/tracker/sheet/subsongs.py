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

    def data_from_item(self, item):
        data = item.data()
        obj = data.toPyObject()
        items = obj.items()
        pyitems = [(str(a), b) for (a, b) in items]
        pydict = dict(pyitems)
        return pydict

    def show_album(self):
        QtCore.QObject.emit(self, QtCore.SIGNAL('compositionParams()'))
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('subsongChanged(int)'),
                            -1)

    def show_trash(self):
        self.show_album()

    def currentChanged(self, new_index, old_index):
        model = new_index.model()
	item = model.itemFromIndex(new_index)
        item_data = self.data_from_item(item)
        item_id = item_data['type']
	parts = item_id.split('_')
        item_type = parts[0]
        if item_type == 'album':
            self.show_album()
        elif item_type == 'song':
            song_number = int(parts[1])
            QtCore.QObject.emit(self, QtCore.SIGNAL('subsongParams(int)'),
                                song_number)
            QtCore.QObject.emit(self,
                                QtCore.SIGNAL('subsongChanged(int)'),
                                song_number)
        elif item_type == 'trash':
            self.show_trash()
        elif item_type == 'section':
            pattern_number = int(parts[1])
            song_item = item.parent()
            song_data = self.data_from_item(song_item)
            song_id = song_data['type']
	    song_id_parts = song_id.split('_')
            song_type = song_id_parts[0]
            if song_type == 'trash':
                self.show_trash()
            else:
                song_number = int(song_id_parts[1]) 
                self._section_manager.set(song_number, pattern_number)
                QtCore.QObject.emit(self,
                                    QtCore.SIGNAL('subsongChanged(int)'),
                                    song_number)
        else:
            assert False

    def create_systems(self, order_list):
        for i2, pattern in enumerate(order_list):
            system_number = i2 + 1
            pattern_id = 'section_{0}'.format(i2)
            ptt = 'System {1}'.format(system_number, pattern)
            pname = str(system_number) + ': ' + str(pattern)
            pattern_item = QtGui.QStandardItem(ptt)
            #pattern_item.setToolTip(ptt)
            pattern_item.setEditable(True)
            pattern_item.setData({'type':pattern_id})
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
            song_item.setData({'type':song_id})
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






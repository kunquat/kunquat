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


class Subsongs(QtGui.QWidget):

    def __init__(self, p):
        QtGui.QWidget.__init__(self)
        self.p = p

        song_list = QtGui.QTreeView()
        song_list.setHeaderHidden(True)
        song_list.setRootIsDecorated(False)
        #song_list.setDragDropMode(QtGui.QAbstractItemView.InternalMove)
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

    def update(self):
        project = self.p.project
        album = QtGui.QStandardItem('untitled album')
        album.setEditable(False)
        model = QtGui.QStandardItemModel()
        root = model.invisibleRootItem()
        root.appendRow(album)
        songs = self.p.project._composition.song_ids()
        for i, song_id in enumerate(songs):
            song_number = i + 1
            song = self.p.project._composition.get_song(song_id)
            song_name = song.get_name()
            name = '{0}: {1}'.format(song_number, song_name)
            stt = 'Song {0}: {1}'.format(song_number, song_name)
            song_item = QtGui.QStandardItem(stt)
            #song_item.setToolTip(stt)
            song_item.setEditable(False)
            album.appendRow(song_item)
            order_list = song.get_order_list()
            for i2, pattern in enumerate(order_list):
                system_number = i2 + 1
                ptt = 'System {1}'.format(system_number, pattern)
                pname = str(system_number) + ': ' + str(pattern)
                pattern_item = QtGui.QStandardItem(ptt)
                #pattern_item.setToolTip(ptt)
                pattern_item.setEditable(True)
                song_item.appendRow(pattern_item)
        self._song_list.setModel(model)
        self._song_list.expandAll()






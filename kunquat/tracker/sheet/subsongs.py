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


class Subsongs(QtGui.QTreeView):

    comp_signal = QtCore.pyqtSignal(name='compositionParams')
    subsong_params = QtCore.pyqtSignal(int, name='subsongParams')
    subsong_changed = QtCore.pyqtSignal(int, name='subsongChanged')

    def __init__(self, project, section, parent=None):
        QtGui.QTreeView.__init__(self, parent)
        self._cur_subsong = -1
        self._section_signal = False
        self._parent = parent
        self.set_project(project)
        section.connect(self.section_changed)
        self._section_manager = section
        self.setHeaderHidden(True)
        self.sync()
        QtCore.QObject.connect(self,
                           QtCore.SIGNAL('doubleClicked(const QModelIndex&)'),
                           self.create_new_node)

    def currentChanged(self, new_index, old_index):
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

    def keyPressEvent(self, ev):
        select_model = self.selectionModel()
        selections = select_model.selectedIndexes()
        if not selections:
            index = self._model.index(0, 0)
            select_model.select(index, QtGui.QItemSelectionModel.Select)
            select_model.setCurrentIndex(index,
                                         QtGui.QItemSelectionModel.Select)
            selections = select_model.selectedIndexes()
        assert len(selections) == 1
        selection = selections[0]
        item = self._model.itemFromIndex(selection)
        parent = item.parent()
        child = item.child(0)
        section_number = -1
        subsong_number = -1
        if parent and parent != self._model.itemFromIndex(
                                        self._model.index(0, 0)):
            section_number = selection.row()
            subsong_number = selection.parent().row()
        if parent and not parent.parent(): # subsong
            if ev.key() == QtCore.Qt.Key_Return:
                if item.row() < len(self._slists):
                    return
                assert item.row() == len(self._slists)
                self.create_new_node(self._model.indexFromItem(item))
                return
            elif ev.key() == QtCore.Qt.Key_Insert:
                if item.row() >= len(self._slists):
                    assert item.row() == len(self._slists)
                    return
                if len(self._slists) >= lim.SONGS_MAX:
                    assert len(self._slists) == lim.SONGS_MAX
                    return
                subsong_number = item.row()
                self._project.start_group(
                        'Insert subsong {0:d}'.format(subsong_number))
                try:
                    key_format = 'song_{0:02x}/p_song.json'
                    for i in xrange(len(self._slists), subsong_number, -1):
                        #print('move', i - 1, 'to', i)
                        self._project[key_format.format(i)] = \
                                self._project[key_format.format(i - 1)]
                    ss_info = {'patterns': []}
                    self._project[key_format.format(subsong_number)] = ss_info
                finally:
                    self._project.end_group()
                new_ss = QtGui.QStandardItem('Song {0}'.format(
                                                        subsong_number))
                item.parent().insertRow(subsong_number, new_ss)
                add_section = QtGui.QStandardItem('New section...')
                add_section.setEditable(False)
                add_section.setFont(QtGui.QFont('Decorative', italic=True))
                new_ss.appendRow(add_section)
                self._slists[subsong_number:subsong_number] = [[]]
                parent = item.parent()
                for i in xrange(subsong_number + 1, len(self._slists)):
                    parent.child(i).setText('Song {0}'.format(i))
                select_model = self.selectionModel()
                index = self._model.indexFromItem(new_ss)
                selection_mode = QtGui.QItemSelectionModel.Select
                select_model.clear()
                select_model.select(index, selection_mode)
                select_model.setCurrentIndex(index, selection_mode)
                self.expand(index)
                if len(self._slists) >= lim.SONGS_MAX:
                    assert len(self._slists) == lim.SONGS_MAX
                    item.parent().removeRow(lim.SONGS_MAX)
                return
            elif ev.key() == QtCore.Qt.Key_Delete:
                if item.row() >= len(self._slists):
                    assert item.row() == len(self._slists)
                    return
                subsong_number = item.row()
                self._project.start_group(
                        'Remove song {0:d}'.format(subsong_number))
                try:
                    key_format = 'song_{0:02x}/p_song.json'
                    #print('remove', subsong_number)
                    self._project[key_format.format(subsong_number)] = None
                    for i in xrange(subsong_number + 1, len(self._slists)):
                        #print('move', i, 'to', i - 1)
                        self._project[key_format.format(i - 1)] = \
                                self._project[key_format.format(i)]
                        self._project[key_format.format(i)] = None
                finally:
                    self._project.end_group()
                parent = item.parent()
                parent.removeRow(subsong_number)
                self._slists[subsong_number:subsong_number + 1] = []
                for i in xrange(subsong_number, len(self._slists)):
                    parent.child(i).setText('Song {0}'.format(i))
                if len(self._slists) == lim.SONGS_MAX - 1:
                    add_ss = QtGui.QStandardItem('New song...')
                    add_ss.setEditable(False)
                    add_ss.setFont(QtGui.QFont('Decorative', italic=True))
                    parent.appendRow(add_ss)
                # TODO: update subsong jumps in pattern data!
                return
        if subsong_number >= 0 and section_number >= 0: # section
            if ev.key() == QtCore.Qt.Key_Return:
                if child or not parent:
                    return
                if len(self._slists) <= subsong_number:
                    return
                if len(self._slists[subsong_number]) != section_number:
                    sel = select_model.selection()
                    region = self.visualRegionForSelection(sel)
                    area = region.boundingRect()
                    if area.isNull():
                        return
                    center = area.center()
                    me = QtGui.QMouseEvent(QtCore.QEvent.MouseButtonDblClick,
                                           center, QtCore.Qt.LeftButton,
                                           QtCore.Qt.NoButton,
                                           QtCore.Qt.NoModifier)
                    self.mouseDoubleClickEvent(me)
                    self.mouseDoubleClickEvent(me)
                    return
                assert len(self._slists[subsong_number]) < lim.SECTIONS_MAX
                self.create_new_node(self._model.indexFromItem(item))
                return
            elif ev.key() in (QtCore.Qt.Key_Insert, QtCore.Qt.Key_Delete):
                if child or not parent:
                    return
                if len(self._slists) <= subsong_number or \
                        len(self._slists[subsong_number]) <= section_number:
                    return
                if ev.key() == QtCore.Qt.Key_Insert:
                    if len(self._slists[subsong_number]) >= lim.SECTIONS_MAX:
                        return
                else:
                    if len(self._slists[subsong_number]) <= 0:
                        return
                pattern_number = int(item.text())
                path = 'song_{0:02x}/p_song.json'.format(subsong_number)
                ss_info = self._project[path]
                slist = ss_info['patterns']
                if ev.key() == QtCore.Qt.Key_Insert:
                    slist[section_number:section_number] = [pattern_number]
                else:
                    slist[section_number:section_number + 1] = []
                self._project[path] = ss_info
                self._slists[subsong_number] = slist
                if ev.key() == QtCore.Qt.Key_Insert:
                    pattern_item = QtGui.QStandardItem(str(pattern_number))
                    pattern_item.setEditable(True)
                    item.parent().insertRow(section_number, pattern_item)
                    if len(slist) >= lim.SECTIONS_MAX:
                        assert len(slist) == lim.SECTIONS_MAX
                        item.parent().removeRow(lim.SECTIONS_MAX)
                else:
                    if len(slist) == lim.SECTIONS_MAX - 1:
                        add_item = QtGui.QStandardItem('New section...')
                        add_item.setEditable(False)
                        add_item.setFont(QtGui.QFont('Decorative',
                                                     italic=True))
                        item.parent().appendRow(add_item)
                    item.parent().removeRow(section_number)
                self._section_manager.set(subsong_number, section_number)
                return
            elif ev.key() in (QtCore.Qt.Key_Plus, QtCore.Qt.Key_Minus):
                if child or not parent:
                    return
                if len(self._slists) <= subsong_number or \
                        len(self._slists[subsong_number]) <= section_number:
                    return
                pattern_number = int(item.text())
                if ev.key() == QtCore.Qt.Key_Plus:
                    if pattern_number >= lim.PATTERNS_MAX - 1:
                        return
                else:
                    if pattern_number <= 0:
                        return
                path = 'song_{0:02x}/p_song.json'.format(subsong_number)
                ss_info = self._project[path]
                slist = ss_info['patterns']
                if ev.key() == QtCore.Qt.Key_Plus:
                    slist[section_number] += 1
                else:
                    slist[section_number] -= 1
                self._project[path] = ss_info
                #self._slists[subsong_number] = slist
                #item.setText(str(slist[section_number]))
                self._section_manager.set(subsong_number, section_number)
                return
        QtGui.QTreeView.keyPressEvent(self, ev)

    def modified(self, item):
        subsong, section = self.resolve_location(item)
        if subsong >= 0:
            path = 'song_{0:02x}/p_song.json'.format(subsong)
            if section >= 0:
                if section >= len(self._slists[subsong]):
                    return
                ss_info = self._project[path]
                try:
                    pat = int(item.text())
                    if pat < 0 or pat >= lim.PATTERNS_MAX:
                        raise ValueError
                except ValueError:
                    item.setText(str(self._slists[subsong][section]))
                    return
                slist = ss_info['patterns']
                assert slist
                if pat == slist[section]:
                    return
                slist[section] = pat
                self._project[path] = ss_info
                self._slists[subsong][section] = pat
                self._section_manager.set(subsong, section)
            else:
                pass

    def resolve_location(self, item):
        subsong = -1
        section = -1
        if item.parent():
            num = item.row()
            if item.parent().parent():
                subsong = item.parent().row()
                section = num
            else:
                subsong = num
        return subsong, section

    def create_new_node(self, index):
        item = self._model.itemFromIndex(index)
        subsong, section = self.resolve_location(item)
        if subsong >= 0:
            path = 'song_{0:02x}/p_song.json'.format(subsong)
            if section >= 0:
                if section == len(self._slists[subsong]):
                    pattern_number = 0
                    ss_info = self._project[path]
                    if ss_info and 'patterns' in ss_info:
                        slist = ss_info['patterns']
                    else:
                        if ss_info == None:
                            ss_info = {}
                        ss_info['patterns'] = []
                        slist = ss_info['patterns']
                    if slist:
                        pattern_number = slist[-1]
                    slist.append(pattern_number)
                    self._project[path] = ss_info
                    """
                    self._slists[subsong] = slist
                    if len(slist) < lim.SECTIONS_MAX:
                        add_item = QtGui.QStandardItem('New section...')
                        add_item.setEditable(False)
                        add_item.setFont(QtGui.QFont('Decorative', italic=True))
                        item.parent().appendRow(add_item)
                    item.setText(str(slist[section]))
                    item.setEditable(True)
                    item.setFont(QtGui.QFont())
                    self._section_manager.set(subsong, section)
                    """
            else:
                if subsong == len(self._slists):
                    ss_info = self._project[path]
                    if ss_info == None:
                        ss_info = { 'patterns': [] }
                    if 'patterns' in ss_info:
                        self._slists.append(ss_info['patterns'])
                    else:
                        self._slists.append([])
                    parent = item.parent()
                    self._project[path] = ss_info
                    """
                    if len(self._slists) < lim.SONGS_MAX:
                        add_ss = QtGui.QStandardItem('New song...')
                        add_ss.setEditable(False)
                        add_ss.setFont(QtGui.QFont('Decorative', italic=True))
                        parent.appendRow(add_ss)
                    item.setText('Song {0}'.format(subsong))
                    item.setFont(QtGui.QFont())
                    if len(self._slists[-1]) < lim.SONGS_MAX:
                        add_section = QtGui.QStandardItem('New section...')
                        add_section.setEditable(False)
                        add_section.setFont(QtGui.QFont('Decorative',
                                                        italic=True))
                        item.appendRow(add_section)
                    self.expand(self._model.indexFromItem(item))
                    """
                    QtCore.QObject.emit(self, QtCore.SIGNAL('subsongParams(int)'),
                                        subsong)
                    QtCore.QObject.emit(self, QtCore.SIGNAL('subsongChanged(int)'),
                                        subsong)

    def section_changed(self, *args):
        subsong, section = args
        select_model = self.selectionModel()
        index = self._model.item(0).child(subsong).child(section).index()
        select_model.clearSelection()
        select_model.select(index, QtGui.QItemSelectionModel.Select)
        select_model.setCurrentIndex(index, QtGui.QItemSelectionModel.Select)

    def set_project(self, project):
        self._project = project

    def sync(self):
        self._slists = []
        for num in xrange(lim.SONGS_MAX):
            path = 'song_{0:02x}/p_song.json'.format(num)
            subsong = self._project[path]
            if subsong == None:
                break
            if 'patterns' in subsong:
                self._slists.append(subsong['patterns'])
            else:
                self._slists.append([])

        cur_subsong = -1
        cur_section = -1
        select_model = self.selectionModel()
        if select_model:
            selected = select_model.selectedIndexes()
            if selected:
                assert len(selected) == 1
                selected = selected[0]
                if selected.isValid() and selected.parent().isValid():
                    if not selected.parent().parent().isValid(): # subsong
                        cur_subsong = selected.row()
                    else: # section
                        cur_subsong = selected.parent().row()
                        cur_section = selected.row()

        self._model = QtGui.QStandardItemModel(self._parent)
        QtCore.QObject.connect(self._model,
                               QtCore.SIGNAL('itemChanged(QStandardItem*)'),
                               self.modified)
        root = self._model.invisibleRootItem()
        composition = QtGui.QStandardItem('Composition')
        composition.setEditable(False)
        root.appendRow(composition)
        for index, slist in enumerate(self._slists):
            subsong_item = QtGui.QStandardItem('Song {0}'.format(index))
            subsong_item.setEditable(False)
            composition.appendRow(subsong_item)
            for section, pattern in enumerate(slist):
                pattern_item = QtGui.QStandardItem(str(pattern))
                pattern_item.setEditable(True)
                subsong_item.appendRow(pattern_item)
            if len(slist) < lim.SECTIONS_MAX:
                add_item = QtGui.QStandardItem('New section...')
                add_item.setEditable(False)
                add_item.setFont(QtGui.QFont('Decorative', italic=True))
                subsong_item.appendRow(add_item)
        if len(self._slists) < lim.SONGS_MAX:
            add_item = QtGui.QStandardItem('New song...')
            add_item.setEditable(False)
            add_item.setFont(QtGui.QFont('Decorative', italic=True))
            composition.appendRow(add_item)
        self.setModel(self._model)
        self.expandAll()

        item = self._model.item(0)
        if cur_subsong >= 0:
            child = item.child(cur_subsong)
            item = child if child else item
            if cur_section >= 0 and child:
                child = item.child(cur_section)
                item = child if child else item
        select_model = self.selectionModel()
        assert select_model
        index = self._model.indexFromItem(item)
        select_model.select(index, QtGui.QItemSelectionModel.Select)
        select_model.setCurrentIndex(index, QtGui.QItemSelectionModel.Select)



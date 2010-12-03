# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
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

import kqt_limits as lim


class Subsongs(QtGui.QTreeView):

    def __init__(self, project, section, parent=None):
        QtGui.QTreeView.__init__(self, parent)
        self.set_project(project)
        section.connect(self.section_changed)
        self.section_manager = section
        """
        self.model = QtGui.QStandardItemModel(parent)
        parent_item = self.model.invisibleRootItem()
        item = QtGui.QStandardItem('lol')
        parent_item.appendRow(item)
        parent_item.appendRow(QtGui.QStandardItem('one'))
        item.appendRow(QtGui.QStandardItem('two'))
        self.setModel(self.model)
        """

    def currentChanged(self, new_index, old_index):
        print((old_index.row(), old_index.column(),
                    old_index.parent() == QtCore.QModelIndex()),
              (new_index.row(), new_index.column(),
                    new_index.parent() == QtCore.QModelIndex()))

    def section_changed(self, *args):
        print('ss:', args)

    def set_project(self, project):
        self.project = project

    def sync(self):
        self.ss = [None] * lim.SUBSONGS_MAX
        for num in xrange(lim.SUBSONGS_MAX):
            path = 'subs_{0:02x}/p_subsong.json'.format(num)
            self.ss[num] = self.project[path]



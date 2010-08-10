#!/usr/bin/env python
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

import sys

from PyQt4 import QtCore, QtGui

from sheet import Sheet


class KqtEditor(QtGui.QMainWindow):

    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        # FIXME: size and title
        self.resize(400, 300)
        self.setWindowTitle('Kunquat')

        self.statusBar().showMessage('[status]')

        self.central = QtGui.QWidget(self)
        self.setCentralWidget(self.central)
        top_layout = QtGui.QVBoxLayout(self.central)
        top_layout.setMargin(0)
        top_layout.setSpacing(0)

        top_control = self.create_top_control()

        tabs = QtGui.QTabWidget()
        sheet = Sheet()
        tabs.addTab(sheet, 'Sheet')
        
        top_layout.addWidget(top_control)
        top_layout.addWidget(tabs)

    def create_separator(self):
        separator = QtGui.QFrame()
        separator.setFrameShape(QtGui.QFrame.VLine)
        separator.setFrameShadow(QtGui.QFrame.Sunken)
        return separator

    def create_top_control(self):
        top_control = QtGui.QWidget()
        layout = QtGui.QHBoxLayout(top_control)
        layout.setMargin(5)
        layout.setSpacing(5)
        icon_prefix = ':/trolltech/styles/commonstyle/images/'

        new_project = QtGui.QToolButton()
        new_project.setText('New Project')
        new_project.setIcon(QtGui.QIcon(QtGui.QPixmap(icon_prefix +
                                                      'file-32.png')))
        new_project.setAutoRaise(True)

        open_project = QtGui.QToolButton()
        open_project.setText('Open Project')
        open_project.setIcon(QtGui.QIcon(QtGui.QPixmap(icon_prefix +
                                             'standardbutton-open-32.png')))
        open_project.setAutoRaise(True)

        save_project = QtGui.QToolButton()
        save_project.setText('Save Project')
        save_project.setIcon(QtGui.QIcon(QtGui.QPixmap(icon_prefix +
                                             'standardbutton-save-32.png')))
        save_project.setAutoRaise(True)

        play = QtGui.QToolButton()
        play.setText('Play')
        play.setAutoRaise(True)

        stop = QtGui.QToolButton()
        stop.setText('Stop')
        stop.setAutoRaise(True)

        seek_back = QtGui.QToolButton()
        seek_back.setText('Seek backwards')
        seek_back.setAutoRaise(True)

        seek_for = QtGui.QToolButton()
        seek_for.setText('Seek forwards')
        seek_for.setAutoRaise(True)

        pos_display = QtGui.QLabel('[position display]')

        subsong_select = QtGui.QLabel('[subsong select]')

        tempo_factor = QtGui.QLabel('[tempo factor]')

        instrument = QtGui.QLabel('[instrument]')

        octave = QtGui.QLabel('[octave]')

        layout.addWidget(new_project)
        layout.addWidget(open_project)
        layout.addWidget(save_project)
        layout.addWidget(self.create_separator())

        layout.addWidget(play)
        layout.addWidget(stop)
        layout.addWidget(seek_back)
        layout.addWidget(seek_for)
        layout.addWidget(self.create_separator())

        layout.addWidget(pos_display)
        layout.addWidget(subsong_select)
        layout.addWidget(tempo_factor)
        layout.addWidget(self.create_separator())

        layout.addWidget(instrument)
        layout.addWidget(octave)
        return top_control


if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    editor = KqtEditor()
    editor.show()
    sys.exit(app.exec_())



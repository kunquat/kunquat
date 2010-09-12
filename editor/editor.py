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

from __future__ import division
from __future__ import print_function
from itertools import count
import math
import random
import sys
import time

from kunquat.extras import pulseaudio
from PyQt4 import QtCore, QtGui

import project
from sheet import Sheet


PROGRAM_NAME = 'Kunquat'
PROGRAM_VERSION = '0.0.0'


def sine():
    phase = 0
    shift = (2 * math.pi * 440) / 48000
    while True:
        yield math.sin(phase)
        phase = (phase + shift) % (2 * math.pi)


class KqtEditor(QtGui.QMainWindow):

    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        self.project = project.Project(0)
        self.handle = self.project.handle
        self.set_appearance()
        self.pa = pulseaudio.Poll(PROGRAM_NAME, 'Monitor')
        self.mix_timer = QtCore.QTimer(self)
        QtCore.QObject.connect(self.mix_timer, QtCore.SIGNAL('timeout()'),
                               self.mix)
        self.bufs = (None, None)

        """
        self.pa_debug_timer = QtCore.QTimer(self)
        QtCore.QObject.connect(self.pa_debug_timer, QtCore.SIGNAL('timeout()'),
                               self.print_pa_state)
        self.pa_debug_timer.start(1)
        """

    def mix(self):
        if not self.bufs[0]:
            self.bufs = self.handle.mix()
            if not self.bufs[0]:
                self.mix_timer.stop()
                return
        if self.pa.try_write(*self.bufs):
            self.bufs = self.handle.mix()

    def print_pa_state(self):
        print('Context: {0}, Stream: {1}, Error: {2}'.format(
                  self.pa.context_state(), self.pa.stream_state(),
                  self.pa.error()), end='\r')

    def play(self):
        self.mix_timer.start(0)

    def stop(self):
        self.mix_timer.stop()

    def set_appearance(self):
        # FIXME: size and title
        self.resize(400, 300)
        self.setWindowTitle(PROGRAM_NAME)

        self.statusBar().showMessage('[status]')

        self.central = QtGui.QWidget(self)
        self.setCentralWidget(self.central)
        top_layout = QtGui.QVBoxLayout(self.central)
        top_layout.setMargin(0)
        top_layout.setSpacing(0)

        top_control = self.create_top_control()

        tabs = QtGui.QTabWidget()
        sheet = Sheet(self.project)
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
        QtCore.QObject.connect(play, QtCore.SIGNAL('clicked()'),
                               self.play)

        stop = QtGui.QToolButton()
        stop.setText('Stop')
        stop.setAutoRaise(True)
        QtCore.QObject.connect(stop, QtCore.SIGNAL('clicked()'),
                               self.stop)

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



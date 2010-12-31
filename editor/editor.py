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

from instruments import Instruments
import kqt_limits as lim
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


class Playback(QtCore.QObject):

    _play_sub = QtCore.pyqtSignal(int, name='playSubsong')
    _play_pat = QtCore.pyqtSignal(int, name='playPattern')
    _play_event = QtCore.pyqtSignal(int, str, name='playEvent')
    _stop = QtCore.pyqtSignal(name='stop')

    def __init__(self, parent=None):
        QtCore.QObject.__init__(self, parent)

    def connect(self, play_sub, play_pat, play_event, stop):
        """Connects the playback control signals to functions."""
        self._play_sub.connect(play_sub)
        self._play_pat.connect(play_pat)
        self._play_event.connect(play_event)
        self._stop.connect(stop)

    def stop(self):
        """Stops playback."""
        QtCore.QObject.emit(self, QtCore.SIGNAL('stop()'))

    def play_subsong(self, subsong):
        """Plays a subsong."""
        QtCore.QObject.emit(self, QtCore.SIGNAL('playSubsong(int)'), subsong)

    def play_pattern(self, pattern):
        """Plays a pattern repeatedly."""
        QtCore.QObject.emit(self, QtCore.SIGNAL('playPattern(int)'), pattern)

    def play_event(self, channel, event):
        """Plays a single event."""
        self._play_event.emit(channel, event)
        #QtCore.QObject.emit(self, QtCore.SIGNAL('playEvent(int, str)'),
        #                    channel, event)


class KqtEditor(QtGui.QMainWindow):

    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        self._playback = Playback()
        self._playback.connect(self.play_subsong, self.play_pattern,
                               self.play_event, self.stop)
        self.project = project.Project(0)
        self.handle = self.project.handle
        self.set_appearance()
        self.pa = pulseaudio.Poll(PROGRAM_NAME, 'Monitor')
        self.mix_timer = QtCore.QTimer(self)
        QtCore.QObject.connect(self.mix_timer, QtCore.SIGNAL('timeout()'),
                               self.mix)
        self.bufs = (None, None)
        self.playing = False
        self.mix_timer.start(1)
        self._cur_subsong = -1
        self._cur_pattern = 0

        """
        self.pa_debug_timer = QtCore.QTimer(self)
        QtCore.QObject.connect(self.pa_debug_timer, QtCore.SIGNAL('timeout()'),
                               self.print_pa_state)
        self.pa_debug_timer.start(1)
        """

    def keyPressEvent(self, ev):
        if ev.modifiers() == QtCore.Qt.ControlModifier:
            if ev.key() == QtCore.Qt.Key_Z:
                self.project.undo()
                self.sync()
            if ev.key() == QtCore.Qt.Key_Y:
                self.project.redo()
                self.sync()
            return
        if ev.modifiers() == QtCore.Qt.ShiftModifier:
            if ev.key() == QtCore.Qt.Key_PageUp:
                self._instrument.setValue(self._instrument.value() - 1)
            elif ev.key() == QtCore.Qt.Key_PageDown:
                self._instrument.setValue(self._instrument.value() + 1)
            return
        if ev.key() == QtCore.Qt.Key_F5:
            self._playback.play_subsong(self._cur_subsong)
        elif ev.key() == QtCore.Qt.Key_F6:
            self._playback.play_pattern(self._cur_pattern)
        elif ev.key() == QtCore.Qt.Key_F8:
            self._playback.stop()
        elif ev.key() == QtCore.Qt.Key_Less:
            self._octave.setValue(self._octave.value() - 1)
        elif ev.key() == QtCore.Qt.Key_Greater:
            self._octave.setValue(self._octave.value() + 1)

    def mix(self):
        if self.playing:
            if not self.bufs[0]:
                self.bufs = self.handle.mix()
                if not self.bufs[0]:
                    self.stop()
                    return
            if self.pa.try_write(*self.bufs):
                self.bufs = self.handle.mix()
        else:
            self.pa.iterate()

    def pattern_changed(self, num):
        self._cur_pattern = num

    def subsong_changed(self, num):
        self._cur_subsong = num

    def print_pa_state(self):
        print('Context: {0}, Stream: {1}, Error: {2}'.format(
                  self.pa.context_state(), self.pa.stream_state(),
                  self.pa.error()), end='\r')

    def play(self):
        self.playing = True

    def stop(self):
        self.playing = False
        self.handle.nanoseconds = 0

    def play_subsong(self, subsong):
        self.handle.nanoseconds = 0
        self.handle.subsong = subsong
        self.playing = True

    def play_pattern(self, pattern):
        self.handle.nanoseconds = 0
        self.playing = True
        self.handle.trigger(-1, '[">pattern", [{0}]'.format(pattern))

    def play_event(self, *args):
        channel, event = args
        event = str(event)
        if not self.playing:
            self.playing = True
            self.handle.trigger(-1, '[">pause", []]')
        self.handle.trigger(channel, event)

    def save(self):
        self.project.save()

    def export_composition(self):
        path = QtGui.QFileDialog.getSaveFileName(
                caption='Export Kunquat composition',
                filter='Kunquat compositions (*.kqt *.kqt.gz *.kqt.bz2)')
        if path:
            self.project.export_kqt(str(path))

    def import_composition(self):
        path = QtGui.QFileDialog.getOpenFileName(
                caption='Import Kunquat composition',
                filter='Kunquat compositions (*.kqt *.kqt.gz *.kqt.bz2)')
        if path:
            self.stop()
            self.project.import_kqt(str(path))
            self.sync()

    def clear(self):
        self.stop()
        self.project.clear()
        self.sync()

    def sync(self):
        self._sheet.sync()
        self._instruments.sync()

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
        self._sheet = Sheet(self.project, self._playback,
                            self.subsong_changed, self.pattern_changed,
                            self._octave, self._instrument)
        tabs.addTab(self._sheet, 'Sheet')
        self._instruments = Instruments(self.project,
                                        self._instrument)
        tabs.addTab(self._instruments, 'Instruments')

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
        new_project.setText('Clear Project')
        new_project.setIcon(QtGui.QIcon(QtGui.QPixmap(icon_prefix +
                                                      'file-32.png')))
        new_project.setAutoRaise(True)
        QtCore.QObject.connect(new_project,
                               QtCore.SIGNAL('clicked()'),
                               self.clear)

        open_project = QtGui.QToolButton()
        open_project.setText('Import Composition')
        open_project.setIcon(QtGui.QIcon(QtGui.QPixmap(icon_prefix +
                                             'standardbutton-open-32.png')))
        open_project.setAutoRaise(True)
        QtCore.QObject.connect(open_project,
                               QtCore.SIGNAL('clicked()'),
                               self.import_composition)

        save_project = QtGui.QToolButton()
        save_project.setText('Save Project')
        save_project.setIcon(QtGui.QIcon(QtGui.QPixmap(icon_prefix +
                                             'standardbutton-save-32.png')))
        save_project.setAutoRaise(True)
        QtCore.QObject.connect(save_project, QtCore.SIGNAL('clicked()'),
                               self.save)

        export = QtGui.QToolButton()
        export.setText('Export')
        export.setAutoRaise(True)
        QtCore.QObject.connect(export, QtCore.SIGNAL('clicked()'),
                               self.export_composition)

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

        self._instrument = QtGui.QSpinBox()
        self._instrument.setMinimum(0)
        self._instrument.setMaximum(lim.INSTRUMENTS_MAX - 1)
        self._instrument.setValue(0)
        self._instrument.setToolTip('Instrument')

        self._octave = QtGui.QSpinBox()
        self._octave.setMinimum(lim.SCALE_OCTAVE_FIRST)
        self._octave.setMaximum(lim.SCALE_OCTAVE_LAST)
        self._octave.setValue(4)
        self._octave.setToolTip('Base octave')

        layout.addWidget(new_project)
        layout.addWidget(open_project)
        layout.addWidget(save_project)
        layout.addWidget(export)
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

        layout.addWidget(self._instrument)
        layout.addWidget(self._octave)
        return top_control

    def __del__(self):
        self.mix_timer.stop()
        #QtGui.QMainWindow.__del__(self)


if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    editor = KqtEditor()
    editor.show()
    sys.exit(app.exec_())



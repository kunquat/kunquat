#!/usr/bin/env python
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
from itertools import count
import json
import math
import os.path
import random
import sys
import time

from kunquat.qt import TypewriterView
from kunquat.extras import pulseaudio
from kunquat.tracker.env import Env
from PyQt4 import QtCore, QtGui

from connections import Connections
from effects import Effects
from env import Env
from instruments import Instruments
import keymap
import kqt_limits as lim
import note_input as ni
from peak_meter import PeakMeter
import project
import scale
from sheet import Sheet
import timestamp as ts
from playback import Playback
from statusbar import Statusbar
from toolbar import Toolbar

PROGRAM_NAME = 'Kunquat Tracker'
PROGRAM_VERSION = '0.5.4'


def sine():
    phase = 0
    shift = (2 * math.pi * 440) / 48000
    while True:
        yield math.sin(phase)
        phase = (phase + shift) % (2 * math.pi)


class KqtEditor(QtGui.QMainWindow):

    def __init__(self, args):
        QtGui.QMainWindow.__init__(self)

        try:
            file_path = str(args[1])
        except IndexError:
            file_path = ''

        self.p = self
        self._playback = Playback(self.p)
        self._toolbar = Toolbar(self.p)
        self._status = Statusbar(self.p)
        self.project = project.Project(self, file_path)
        self.handle = self.project.handle
        self._note_input = ni.NoteInput()
        self._scale = scale.Scale({
                          'ref_pitch': 440 * 2**(3/12),
                          'octave_ratio': ['/', [2, 1]],
                          'notes': list(zip(('C', 'C#', 'D', 'D#', 'E', 'F',
                                             'F#', 'G', 'G#', 'A', 'A#', 'B'),
                              (['c', cents] for cents in range(0, 1200, 100))))
                          })
        self.set_appearance()
        self._keys = keymap.KeyMap('Global keys', {
                (QtCore.Qt.Key_Z, QtCore.Qt.ControlModifier):
                        (self._undo, None),
                (QtCore.Qt.Key_Y, QtCore.Qt.ControlModifier):
                        (self._redo, None),
                (QtCore.Qt.Key_Up, QtCore.Qt.ShiftModifier):
                        (self._prev_ins, None),
                (QtCore.Qt.Key_Down, QtCore.Qt.ShiftModifier):
                        (self._next_ins, None),
                (QtCore.Qt.Key_F5, QtCore.Qt.NoModifier):
                        (self._playback._play_subsong, None),
                (QtCore.Qt.Key_F5, QtCore.Qt.ShiftModifier):
                        (lambda x: self.play_subsong(self._cur_subsong, True),
                            None),
                (QtCore.Qt.Key_F6, QtCore.Qt.NoModifier):
                        (self._play_pattern, None),
                (QtCore.Qt.Key_F6, QtCore.Qt.ShiftModifier):
                        (lambda x: self.play_pattern(self._cur_pattern, True),
                            None),
                (QtCore.Qt.Key_F7, QtCore.Qt.NoModifier):
                        (self._play_from, None),
                (QtCore.Qt.Key_F7, QtCore.Qt.ShiftModifier):
                        (lambda x: self.play_from(self._cur_subsong,
                                        self._cur_section,
                                        self._cur_pattern_offset[0],
                                        self._cur_pattern_offset[1], True),
                                    None),
                (QtCore.Qt.Key_F8, QtCore.Qt.NoModifier):
                        (self._stop, None),
                (QtCore.Qt.Key_Less, None):
                        (self._octave_down, None),
                (QtCore.Qt.Key_Greater, None):
                        (self._octave_up, None),
                })
        self.pa = pulseaudio.Poll(PROGRAM_NAME, 'Monitor')
        self.mix_timer = QtCore.QTimer(self)
        QtCore.QObject.connect(self.mix_timer, QtCore.SIGNAL('timeout()'),
                               self.mix)
        self.bufs = (None, None)
        self.mix_timer.start(2)
        self._focus_backup = None
        #self._infinite = False
        self.sync()
        QtCore.QObject.connect(self.project, QtCore.SIGNAL('sync()'),
                               self.sync)
        QtCore.QObject.connect(self, QtCore.SIGNAL('destroyed(QObject*)'),
                               self._finalise)

        """
        self.pa_debug_timer = QtCore.QTimer(self)
        QtCore.QObject.connect(self.pa_debug_timer, QtCore.SIGNAL('timeout()'),
                               self.print_pa_state)
        self.pa_debug_timer.start(1)
        """

    def _finalise(self, obj):
        fw = self.focusWidget()
        fw.clearFocus()

    def _undo(self, ev):
        self.project.undo()

    def _redo(self, ev):
        self.project.redo()

    def _prev_ins(self, ev):
        self._instrument.setValue(self._instrument.value() - 1)

    def _next_ins(self, ev):
        self._instrument.setValue(self._instrument.value() + 1)

    def _play_pattern(self, ev):
        self._playback.play_pattern(self._cur_pattern)

    def _play_from(self, ev):
        self._playback.play_from(self._cur_subsong, self._cur_section,
                                 self._cur_pattern_offset[0],
                                 self._cur_pattern_offset[1])

    def _stop(self, ev):
        self._playback.stop()

    def _octave_down(self, ev):
        self._octave.setValue(self._octave.value() - 1)

    def _octave_up(self, ev):
        self._octave.setValue(self._octave.value() + 1)

    def keyPressEvent(self, ev):
        if self._status.in_progress():
            return
        self._keys.call(ev)
        return

    def mix(self):
        if self._playback.playing:
            if not self.bufs[0]:
                self.handle.fire(0, ['qlocation', None])
                self.bufs = self.handle.mix()
                if not self.bufs[0]:
                    self.stop()
                    return
            for ch, event in self.handle.treceive():
                self.project.tfire(ch, event)
            if self.pa.try_write(*self.bufs):
                dB = [float('-inf')] * 2
                abs_max = [0] * 2
                for ch in (0, 1):
                    min_val = min(self.bufs[ch])
                    max_val = max(self.bufs[ch])
                    abs_max[ch] = max(abs(min_val), abs(max_val))
                    amp = (max_val - min_val) / 2
                    if amp > 0:
                        dB[ch] = math.log(amp, 2) * 6
                self._peak_meter.set_peaks(dB[0], dB[1],
                                           abs_max[0], abs_max[1],
                                           len(self.bufs[0]))
                self.handle.fire(0, ['qlocation', None])
                self.bufs = self.handle.mix()
        else:
            self.pa.iterate()

    def pattern_changed(self, num):
        self._cur_pattern = num

    def section_changed(self, subsong, section):
        self._cur_subsong = subsong
        self._cur_section = section

    def pattern_offset_changed(self, beats, rem):
        self._cur_pattern_offset = ts.Timestamp(beats, rem)

    def print_pa_state(self):
        print('Context: {0}, Stream: {1}, Error: {2}'.format(
                  self.pa.context_state(), self.pa.stream_state(),
                  self.pa.error()), end='\r')

    def play_event(self, *args):
        channel, event = args
        event = str(event)
        if not self.playing:
            self.playing = True
            self.handle.fire(0, ['Ipause', None])
        self.handle.fire(channel, json.loads(event))

    def save(self):
        self.project.save()

    def export_composition(self):
        path = QtGui.QFileDialog.getSaveFileName(
                caption='Export Kunquat composition',
                filter='Kunquat compositions (*.kqt *.kqt.gz *.kqt.bz2)')
        if path:
            self.project.export_kqt(str(path))

    def show_sheet(self):
        sheet = self._sheet
        #sheet = self._sheetbox
        visibility = sheet.isVisible()
        sheet.setVisible(not visibility)

    def environment_window(self):
        self._env.show()

    def instrument_window(self):
        self._instrumentconf.show()

    def sync(self):
        self._sheet.sync()
        self._instruments.sync()
        self._effects.sync()
        self._connections.sync()
        self._env.sync()

    def busy(self, busy_set):
        if busy_set:
            self._focus_backup = QtGui.QApplication.focusWidget()
        self._toolbar.setEnabled(not busy_set)
        self._sheet.setEnabled(not busy_set)
        #self._sheetbox.setEnabled(not busy_set)
        if not busy_set:
            if self._focus_backup:
                self._focus_backup.setFocus()

    def set_appearance(self):
        # FIXME: size and title
        self.resize(400, 300)
        self.setWindowTitle(PROGRAM_NAME)

        #self.statusBar().showMessage('[status]')

        self.central = QtGui.QWidget(self)
        self.setCentralWidget(self.central)
        top_layout = QtGui.QVBoxLayout(self.central)
        top_layout.setMargin(0)
        top_layout.setSpacing(0)
        playback_bar = self._playback.get_view()

        self._instrumentconf = QtGui.QTabWidget()
        self._sheet = Sheet(self.project, self._playback,
                            self._playback.subsong_changed, self.section_changed,
                            self.pattern_changed, self.pattern_offset_changed,
                            self._toolbar._octave, self._toolbar._instrument, playback_bar)
        #self._sheetbox = QtGui.QTabWidget()
        #self._sheetbox.addTab(self._sheet, 'Sheet')
        #self._sheetbox.tabBar().setVisible(False)

        self._instruments = Instruments(self.project,
                                        self._toolbar._instrument,
                                        self._playback,
                                        self._note_input,
                                        self._scale,
                                        self._toolbar._octave)
        self._instrumentconf.addTab(self._instruments, 'Instruments')
        self._effects = Effects(self.project, '')
        self._instrumentconf.addTab(self._effects, 'Effects')
        self._connections = Connections(self.project, 'p_connections.json')
        self._instrumentconf.addTab(self._connections, 'Connections')
        self._env = Env(self.project)
        #self._tabs.addTab(self._env, 'Environment')

        self._peak_meter = PeakMeter(-96, 0, self.handle.mixing_rate)

        QtCore.QObject.connect(self.project,
                               QtCore.SIGNAL('startTask(int)'),
                               self._status.start_task)
        QtCore.QObject.connect(self.project,
                               QtCore.SIGNAL('step(QString)'),
                               self._status.step)
        QtCore.QObject.connect(self.project,
                               QtCore.SIGNAL('endTask()'),
                               self._status.end_task)
        QtCore.QObject.connect(self.project,
                               QtCore.SIGNAL('endTask()'),
                               self.sync)

        instruarea = TypewriterView()
        instrumentpanel = QtGui.QWidget(self)
        instrumentlayout = QtGui.QVBoxLayout(instrumentpanel)
        instrumentlayout.addWidget(self._toolbar.get_view())
        instrumentlayout.addWidget(instruarea)
        instrumentlayout.setMargin(0)
        instrumentlayout.setSpacing(0)

        div = QtGui.QSplitter(self)
        self._div = div
        div.setOrientation(QtCore.Qt.Vertical)
        div.addWidget(self._sheet)
        div.addWidget(instrumentpanel)
        self._sheet.hide()
        #self._sheetbox.hide()

        top_layout.addWidget(div)
        #top_layout.addWidget(self._sheetbox)
        top_layout.addWidget(self._peak_meter)
        top_layout.addWidget(self._status.get_view())

    def __del__(self):
        self.mix_timer.stop()
        #QtGui.QMainWindow.__del__(self)

def main():
    app = QtGui.QApplication(sys.argv)
    editor = KqtEditor(app.arguments())
    editor.show()
    sys.exit(app.exec_())



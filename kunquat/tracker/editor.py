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

from kunquat.qt import Typewriter
from kunquat.extras import pulseaudio
from kunquat.tracker.env import Env
from kunquat.tracker.env import State
from PyQt4 import QtCore, QtGui

from connections import Connections
from effects import Effects
from env import Env
from instruments import Instruments
from instruments import InstEditor
import keymap
import kqt_limits as lim
import note_input as ni
from peak_meter import PeakMeter
import project
from scale import Scale
from sheet import Sheet
import timestamp as ts
from playback import Playback
from statusbar import Statusbar
from toolbar import Toolbar
from piano import Piano

PROGRAM_NAME = 'Kunquat Tracker'
PROGRAM_VERSION = '0.5.4'


def sine():
    phase = 0
    shift = (2 * math.pi * 440) / 48000
    while True:
        yield math.sin(phase)
        phase = (phase + shift) % (2 * math.pi)


class KqtEditor(QtGui.QMainWindow):

    def __init__(self, args, app):
        QtGui.QMainWindow.__init__(self)
        self._app = app
        self.windows = []

        try:
            self._file_path = str(args[1])
        except IndexError:
            self._file_path = ''

        self.p = self

        chromatic_knotes = [
        [(1,1), (3,1), None , (6,1), (8,1), (10,1), None , (1,2), (3,2)],
        [(0,1), (2,1), (4,1), (5,1), (7,1), (9,1), (11,1), (0,2), (2,2), (4,2)],
        [None , (1,0), (3,0), None , (6,0), (8,0), (10,0)],
        [(0,0), (2,0), (4,0), (5,0), (7,0), (9,0), (11,0)]
        ]
 
        chromatic_buttons = {
        (0,  0): {'color': 'dark', 'enabled': True },
        (0,  1): {'color': 'dark', 'enabled': True },
        (0,  2): {'color': None  , 'enabled': False},
        (0,  3): {'color': 'dark', 'enabled': True },
        (0,  4): {'color': 'dark', 'enabled': True },
        (0,  5): {'color': 'dark', 'enabled': True },
        (0,  6): {'color': None  , 'enabled': False},
        (0,  7): {'color': 'dark', 'enabled': True },
        (0,  8): {'color': 'dark', 'enabled': True },
        (0,  9): {'color': None  , 'enabled': False},
        (0, 10): {'color': None  , 'enabled': True },
 
        (1,  0): {'color': 'light', 'enabled': True },
        (1,  1): {'color': 'light', 'enabled': True },
        (1,  2): {'color': 'light', 'enabled': True },
        (1,  3): {'color': 'light', 'enabled': True },
        (1,  4): {'color': 'light', 'enabled': True },
        (1,  5): {'color': 'light', 'enabled': True },
        (1,  6): {'color': 'light', 'enabled': True },
        (1,  7): {'color': 'light', 'enabled': True },
        (1,  8): {'color': 'light', 'enabled': True },
        (1,  9): {'color': 'light', 'enabled': True },
        (1, 10): {'color': 'light', 'enabled': True },
 
        (2,  0): {'color': None  , 'enabled': False},
        (2,  1): {'color': 'dark', 'enabled': True },
        (2,  2): {'color': 'dark', 'enabled': True },
        (2,  3): {'color': None  , 'enabled': False},
        (2,  4): {'color': 'dark', 'enabled': True },
        (2,  5): {'color': 'dark', 'enabled': True },
        (2,  6): {'color': 'dark', 'enabled': True },
        (2,  7): {'color': None  , 'enabled': False},
        (2,  8): {'color': 'dark', 'enabled': True },
        (2,  9): {'color': 'dark', 'enabled': True },
        (2, 10): {'color': None  , 'enabled': False},
 
        (3,  0): {'color': 'light', 'enabled': True },
        (3,  1): {'color': 'light', 'enabled': True },
        (3,  2): {'color': 'light', 'enabled': True },
        (3,  3): {'color': 'light', 'enabled': True },
        (3,  4): {'color': 'light', 'enabled': True },
        (3,  5): {'color': 'light', 'enabled': True },
        (3,  6): {'color': 'light', 'enabled': True },
        (3,  7): {'color': 'light', 'enabled': True },
        (3,  8): {'color': 'light', 'enabled': True },
        (3,  9): {'color': 'light', 'enabled': True },
        (3, 10): {'color': 'light', 'enabled': True },
        }


        chromatic = Scale({
                          'ref_pitch': 440 * 2**(3/12),
                          'octave_ratio': ['/', [2, 1]],
                          'notes': list(zip(('C', 'C#', 'D', 'D#', 'E', 'F',
                                             'F#', 'G', 'G#', 'A', 'A#', 'B'),
                              (['c', cents] for cents in range(0, 1200, 100)))),
                          'knotes': chromatic_knotes,
                          'buttons': chromatic_buttons,
                          'name': 'chromatic'
                          })
        slendro_intervals = [0,
                     0 + 245,
                     0 + 245 + 262,
                     0 + 245 + 262 + 228,
                     0 + 245 + 262 + 228 + 240]
                     #0 + 245 + 262 + 228 + 240 + 230]

        slendro_knotes = [
        [(1,2), (3,2), None , (1,3), (3,3), None , (1,4), (3,4), None],
        [(0,2), (2,2), (4,2), (0,3), (2,3), (4,3), (0,4), (2,4), (4,4), (0,5)],
        [None , (1,0), (3,0), None , (1,1), (3,1), None],
        [(0,0), (2,0), (4,0), (0,1), (2,1), (4,1), None]
        ]
 
        slendro_buttons = {
        (0,  0): {'color': 'light', 'enabled': True },
        (0,  1): {'color': 'light', 'enabled': True },
        (0,  2): {'color': None   , 'enabled': False},
        (0,  3): {'color': 'light', 'enabled': True },
        (0,  4): {'color': 'light', 'enabled': True },
        (0,  5): {'color': None   , 'enabled': False},
        (0,  6): {'color': 'light', 'enabled': True },
        (0,  7): {'color': 'light', 'enabled': True },
        (0,  8): {'color': None   , 'enabled': False},
        (0,  9): {'color': 'light', 'enabled': True },
        (0, 10): {'color': 'light', 'enabled': True },
 
        (1,  0): {'color': 'light', 'enabled': True },
        (1,  1): {'color': 'light', 'enabled': True },
        (1,  2): {'color': 'light', 'enabled': True },
        (1,  3): {'color': 'light', 'enabled': True },
        (1,  4): {'color': 'light', 'enabled': True },
        (1,  5): {'color': 'light', 'enabled': True },
        (1,  6): {'color': 'light', 'enabled': True },
        (1,  7): {'color': 'light', 'enabled': True },
        (1,  8): {'color': 'light', 'enabled': True },
        (1,  9): {'color': 'light', 'enabled': True },
        (1, 10): {'color': 'light', 'enabled': True },
 
        (2,  0): {'color': None   , 'enabled': False},
        (2,  1): {'color': 'light', 'enabled': True },
        (2,  2): {'color': 'light', 'enabled': True },
        (2,  3): {'color': None   , 'enabled': False},
        (2,  4): {'color': 'light', 'enabled': True },
        (2,  5): {'color': 'light', 'enabled': True },
        (2,  6): {'color': None   , 'enabled': False},
 
        (3,  0): {'color': 'light', 'enabled': True },
        (3,  1): {'color': 'light', 'enabled': True },
        (3,  2): {'color': 'light', 'enabled': True },
        (3,  3): {'color': 'light', 'enabled': True },
        (3,  4): {'color': 'light', 'enabled': True },
        (3,  5): {'color': 'light', 'enabled': True },
        (3,  6): {'color': None   , 'enabled': False},
        }
        slendro = Scale({
                          'ref_pitch': 440 * 2**(3/12),
                          'octave_ratio': ['c', 1205],
                          'notes': list(zip(('ji', 'ro', 'lu', 'ma', 'nam'),
                              (['c', cents] for cents in slendro_intervals))),
                          'knotes': slendro_knotes,
                          'buttons': slendro_buttons,
                          'name': 'slendro'
                          })
        self._scales = [chromatic, slendro]
        self._scale = self._scales[0]


        self._playback = Playback(self.p)
        self._status = Statusbar(self.p)
        self._toolbar = Toolbar(self.p)
        self.project = project.Project(self.p, self)
        self._note_input = ni.NoteInput()
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
                        (lambda x: self._playback.play_subsong(self._playback._cur_subsong,
                                                               True),
                            None),
                (QtCore.Qt.Key_F6, QtCore.Qt.NoModifier):
                        (self._play_pattern, None),
                (QtCore.Qt.Key_F6, QtCore.Qt.ShiftModifier):
                        (lambda x: self._playback.play_pattern(self._playback._cur_pattern,
                                                               True),
                            None),
                (QtCore.Qt.Key_F7, QtCore.Qt.NoModifier):
                        (self._play_from, None),
                (QtCore.Qt.Key_F7, QtCore.Qt.ShiftModifier):
                        (lambda x: self._playback.play_from(
                                        self._playback._cur_subsong,
                                        self._playback._cur_section,
                                        self._playback._cur_pattern_offset[0],
                                        self._playback._cur_pattern_offset[1], True),
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
        self.bufs = (None, None)
        self._focus_backup = None
        #self._infinite = False

        self.central = QtGui.QWidget(self)

        self._piano = Piano(self)
        self._tw = Typewriter(self)

        self._instrumentconf = QtGui.QTabWidget()
        self._instrumentconf.setWindowTitle(u'Instrument Configuration')

        self._instruments = Instruments(self.p,
                                        self._tw,
                                        self._piano,
                                        self.project,
                                        self._toolbar._instrument,
                                        self._playback,
                                        self._note_input,
                                        self._scale,
                                        self._toolbar._octave)
        self._effects = Effects(self.project, '')
        self._connections = Connections(self.project, 'p_connections.json')
        self._env = Env(self.project)
        self._state = State(self.project)

    def init(self):
        self.project.init(self._file_path)
        self.handle = self.project.handle

        self._state.init()

        self._peak_meter = PeakMeter(-96, 0, self.handle.mixing_rate)

        playback_bar = self._playback.get_view()
        self._sheet = Sheet(self.project, self._playback,
                            self._playback.subsong_changed, self.section_changed,
                            self.pattern_changed, self.pattern_offset_changed,
                            self._toolbar._octave, self._toolbar._instrument, self._tw, playback_bar)
        #self._sheetbox = QtGui.QTabWidget()
        #self._sheetbox.addTab(self._sheet, 'Sheet')
        #self._sheetbox.tabBar().setVisible(False)

        self._sheet.init()
        self._instruments.init()
        self._effects.init()
        self._connections.init()
        self._env.init()

        self.set_appearance()
        QtCore.QObject.connect(self.mix_timer, QtCore.SIGNAL('timeout()'),
                               self.mix)
        self.mix_timer.start(2)


        self.sync()
        QtCore.QObject.connect(self.project, QtCore.SIGNAL('sync()'),
                               self.sync)
        QtCore.QObject.connect(self, QtCore.SIGNAL('destroyed(QObject*)'),
                               self._finalise)

        self._cur_subsong = 0
        self._cur_section = 0
        self._cur_pattern = 0
        self._cur_pattern_offset = ts.Timestamp()

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
        self._toolbar._octave.setValue(self._toolbar._octave.value() - 1)

    def _octave_up(self, ev):
        self._toolbar._octave.setValue(self._toolbar._octave.value() + 1)

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
                    self._playback.stop()
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

    def state_window(self):
        self._state.show()

    def instruments_window(self):
        self._instrumentconf.show()

    def instrument_window(self, ins_id):
        ie = InstEditor(self.p, self.project, ins_id)
        ie.init()
        self.windows.append(ie) # FIXME: Do we need to collect trash?
        ie.show()
        #ie.sync() FIXME: Do we need to call this?

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
        self.resize(800, 450)
        self.setWindowTitle(PROGRAM_NAME)
        #self.setWindowIcon(QtGui.QIcon('kunquat.svg'))

        #self.statusBar().showMessage('[status]')

        self.setCentralWidget(self.central)

        self._instrumentconf.addTab(self._instruments, 'Instruments')
        self._instrumentconf.addTab(self._effects, 'Effects')
        self._instrumentconf.addTab(self._connections, 'Connections')
        #self._tabs.addTab(self._env, 'Environment')

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

        instruarea = self._tw.get_view()
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

        top_layout = QtGui.QVBoxLayout(self.central)
        top_layout.setMargin(0)
        top_layout.setSpacing(0)
        top_layout.addWidget(div)
        #top_layout.addWidget(self._sheetbox)
        top_layout.addWidget(self._peak_meter)
        top_layout.addWidget(self._status.get_view())

    def __del__(self):
        self.mix_timer.stop()
        #QtGui.QMainWindow.__del__(self)

def main():
    app = QtGui.QApplication(sys.argv)
    editor = KqtEditor(app.arguments(), app)
    editor.init()
    editor.show()
    sys.exit(app.exec_())



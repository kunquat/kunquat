#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2011
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


PROGRAM_NAME = 'Kunquat Tracker'
PROGRAM_VERSION = '0.4.2'


def sine():
    phase = 0
    shift = (2 * math.pi * 440) / 48000
    while True:
        yield math.sin(phase)
        phase = (phase + shift) % (2 * math.pi)


class Playback(QtCore.QObject):

    _play_sub = QtCore.pyqtSignal(int, name='playSubsong')
    _play_pat = QtCore.pyqtSignal(int, name='playPattern')
    _play_from = QtCore.pyqtSignal(int, int, int, int, name='playFrom')
    _play_event = QtCore.pyqtSignal(int, str, name='playEvent')
    _stop = QtCore.pyqtSignal(name='stop')

    def __init__(self, parent=None):
        QtCore.QObject.__init__(self, parent)

    def connect(self, play_sub, play_pat, play_from, play_event, stop):
        """Connects the playback control signals to functions."""
        self._play_sub.connect(play_sub)
        self._play_pat.connect(play_pat)
        self._play_from.connect(play_from)
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

    def play_from(self, subsong, section, beats, rem):
        """Plays a subsong starting from specified position."""
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('playFrom(int, int, int, int)'),
                            subsong, section, beats, rem)

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
                               self.play_from, self.play_event, self.stop)
        self.project = project.Project(0)
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
                        (self._play_subsong, None),
                (QtCore.Qt.Key_F6, QtCore.Qt.NoModifier):
                        (self._play_pattern, None),
                (QtCore.Qt.Key_F7, QtCore.Qt.NoModifier):
                        (self._play_from, None),
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
        self.playing = False
        self.mix_timer.start(2)
        self._cur_subsong = -1
        self._cur_section = -1
        self._cur_pattern_offset = ts.Timestamp()
        self._cur_pattern = 0
        self._focus_backup = None
        self._turing = False
        self.sync()
        QtCore.QObject.connect(self.project, QtCore.SIGNAL('sync()'),
                               self.sync)

        """
        self.pa_debug_timer = QtCore.QTimer(self)
        QtCore.QObject.connect(self.pa_debug_timer, QtCore.SIGNAL('timeout()'),
                               self.print_pa_state)
        self.pa_debug_timer.start(1)
        """

    def _undo(self, ev):
        self.project.undo()

    def _redo(self, ev):
        self.project.redo()

    def _prev_ins(self, ev):
        self._instrument.setValue(self._instrument.value() - 1)

    def _next_ins(self, ev):
        self._instrument.setValue(self._instrument.value() + 1)

    def _play_subsong(self, ev):
        self._playback.play_subsong(self._cur_subsong)

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
        if self.playing:
            if not self.bufs[0]:
                self.bufs = self.handle.mix()
                if not self.bufs[0]:
                    self.stop()
                    return
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
                self.bufs = self.handle.mix()
        else:
            self.pa.iterate()

    def pattern_changed(self, num):
        self._cur_pattern = num

    def subsong_changed(self, num):
        self._cur_subsong = num

    def section_changed(self, subsong, section):
        self._cur_subsong = subsong
        self._cur_section = section

    def pattern_offset_changed(self, beats, rem):
        self._cur_pattern_offset = ts.Timestamp(beats, rem)

    def print_pa_state(self):
        print('Context: {0}, Stream: {1}, Error: {2}'.format(
                  self.pa.context_state(), self.pa.stream_state(),
                  self.pa.error()), end='\r')

    def stop(self):
        if self.playing:
            self.project.update_random()
        self.playing = False
        self.handle.nanoseconds = 0
        self._peak_meter.set_peaks(float('-inf'), float('-inf'), 0, 0, 0)
        self.bufs = (None, None)

    def play_subsong(self, subsong):
        self._peak_meter.reset()
        if self.playing:
            self.project.update_random()
        self.handle.nanoseconds = 0
        self.handle.subsong = subsong
        self.playing = True
        self._set_turing(self._turing)

    def play_pattern(self, pattern):
        self._peak_meter.reset()
        if self.playing:
            self.project.update_random()
        self.handle.subsong = self._cur_subsong
        self.handle.nanoseconds = 0
        self.playing = True
        self.handle.trigger(-1, '[">pattern", [{0}]'.format(pattern))
        self._set_turing(self._turing)

    def play_from(self, subsong, section, beats, rem):
        self._peak_meter.reset()
        if self.playing:
            self.project.update_random()
        self.handle.subsong = subsong
        self.handle.nanoseconds = 0
        self.playing = True
        self.handle.trigger(-1, '[">.gs", [{0}]]'.format(section))
        self.handle.trigger(-1, '[">.gr", [[{0}, {1}]]]'.format(beats, rem))
        self.handle.trigger(-1, '[">g", []]')
        self._set_turing(self._turing)

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
            #self.sync()

    def clear(self):
        self.stop()
        self.project.clear()

    def sync(self):
        self._sheet.sync()
        self._instruments.sync()
        self._effects.sync()
        self._connections.sync()
        self._env.sync()

    def busy(self, busy_set):
        if busy_set:
            self._focus_backup = QtGui.QApplication.focusWidget()
        self._top_control.setEnabled(not busy_set)
        self._tabs.setEnabled(not busy_set)
        if not busy_set:
            if self._focus_backup:
                self._focus_backup.setFocus()

    def _set_turing(self, x):
        self._turing = x
        self.handle.trigger(-1, '[">Turing", [{0}]'.format(
                                'true' if self._turing else 'false'))

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

        self._top_control = self.create_top_control()

        self._tabs = QtGui.QTabWidget()
        self._sheet = Sheet(self.project, self._playback,
                            self.subsong_changed, self.section_changed,
                            self.pattern_changed, self.pattern_offset_changed,
                            self._octave, self._instrument)
        self._tabs.addTab(self._sheet, 'Sheet')
        self._instruments = Instruments(self.project,
                                        self._instrument,
                                        self._playback,
                                        self._note_input,
                                        self._scale,
                                        self._octave)
        self._tabs.addTab(self._instruments, 'Instruments')
        self._effects = Effects(self.project, '')
        self._tabs.addTab(self._effects, 'Effects')
        self._connections = Connections(self.project, 'p_connections.json')
        self._tabs.addTab(self._connections, 'Connections')
        self._env = Env(self.project, 'p_environment.json')
        self._tabs.addTab(self._env, 'Environment')

        self._peak_meter = PeakMeter(-96, 0, self.handle.mixing_rate)

        self._status = self.create_bottom_control()
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

        top_layout.addWidget(self._top_control)
        top_layout.addWidget(self._tabs)
        top_layout.addWidget(self._peak_meter)
        top_layout.addWidget(self._status)

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

        new_project = QtGui.QToolButton()
        new_project.setText('Clear Project')
        new_project.setIcon(QtGui.QIcon.fromTheme('document-new'))
        new_project.setAutoRaise(True)
        QtCore.QObject.connect(new_project,
                               QtCore.SIGNAL('clicked()'),
                               self.clear)

        open_project = QtGui.QToolButton()
        open_project.setText('Import Composition')
        open_project.setIcon(QtGui.QIcon.fromTheme('document-open'))
        open_project.setAutoRaise(True)
        QtCore.QObject.connect(open_project,
                               QtCore.SIGNAL('clicked()'),
                               self.import_composition)

        save_project = QtGui.QToolButton()
        save_project.setText('Save Project')
        save_project.setIcon(QtGui.QIcon.fromTheme('document-save'))
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
        play.setIcon(QtGui.QIcon.fromTheme('media-playback-start'))
        play.setAutoRaise(True)
        QtCore.QObject.connect(play, QtCore.SIGNAL('clicked()'),
                               lambda: self._play_subsong(None))

        stop = QtGui.QToolButton()
        stop.setText('Stop')
        stop.setIcon(QtGui.QIcon.fromTheme('media-playback-stop'))
        stop.setAutoRaise(True)
        QtCore.QObject.connect(stop, QtCore.SIGNAL('clicked()'),
                               lambda: self._stop(None))

        seek_back = QtGui.QToolButton()
        seek_back.setText('Seek backwards')
        seek_back.setIcon(QtGui.QIcon.fromTheme('media-seek-backward'))
        seek_back.setAutoRaise(True)

        seek_for = QtGui.QToolButton()
        seek_for.setText('Seek forwards')
        seek_for.setIcon(QtGui.QIcon.fromTheme('media-seek-forward'))
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

        turing = QtGui.QCheckBox('Turing')
        turing.setCheckState(QtCore.Qt.Unchecked)
        #turing.setToolTip('Turing mode')
        QtCore.QObject.connect(turing, QtCore.SIGNAL('stateChanged(int)'),
                               lambda x: self._set_turing(x ==
                                                    QtCore.Qt.Checked))

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
        layout.addWidget(turing)
        return top_control

    def create_bottom_control(self):
        return Status(self.busy)

    def __del__(self):
        self.mix_timer.stop()
        #QtGui.QMainWindow.__del__(self)


class Status(QtGui.QWidget):

    def __init__(self, busy, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._busy = busy
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        self._status_bar = QtGui.QStatusBar()
        self._status_bar.showMessage('')

        self._progress_bar = QtGui.QProgressBar()
        #self._progress_bar.setAlignment(QtCore.Qt.AlignLeft)
        self._progress_bar.hide()

        layout.addWidget(self._status_bar, 1)
        layout.addWidget(self._progress_bar)

        self._step = 0
        self.setSizePolicy(QtGui.QSizePolicy.MinimumExpanding,
                           QtGui.QSizePolicy.Fixed)
        self._busy_timer = QtCore.QTimer(self)
        self._busy_timer.setSingleShot(True)
        QtCore.QObject.connect(self._busy_timer, QtCore.SIGNAL('timeout()'),
                               lambda: self._busy(True))
        self._tasks = 0

    def in_progress(self):
        assert self._tasks >= 0
        return self._tasks > 0

    def sizeHint(self):
        h = max(self._status_bar.sizeHint().height(),
                self._progress_bar.sizeHint().height())
        return QtCore.QSize(200, h)

    def start_task(self, steps):
        self._tasks += 1
        if self._tasks > 1:
            self._progress_bar.setMaximum(self._progress_bar.maximum() +
                                          steps)
            return
        self._busy_timer.start(100)
        self._progress_bar.setRange(0, steps)
        self._progress_bar.reset()
        if steps > 1:
            self._progress_bar.show()

    def step(self, description):
        self._status_bar.showMessage(description)
        if self._step < self._progress_bar.maximum():
            self._progress_bar.setValue(self._step)
            self._step += 1
        #self._status_bar.update()

    def end_task(self):
        if self._tasks > 1:
            self._tasks -= 1
            return
        self._busy_timer.stop()
        self._busy_timer.setSingleShot(True)
        self._busy(False)
        self._status_bar.showMessage('')
        self._progress_bar.hide()
        self._step = 0
        self._tasks -= 1
        assert self._tasks >= 0


def main():
    app = QtGui.QApplication(sys.argv)
    editor = KqtEditor()
    editor.show()
    sys.exit(app.exec_())



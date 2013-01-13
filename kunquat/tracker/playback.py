from PyQt4 import QtCore, QtGui
from pos_display import PosDisplay
from timestamp import Timestamp

import json

class Playback(QtCore.QObject):
    def __init__(self, p, parent=None):
        QtCore.QObject.__init__(self, parent)
        self.p = p
        self._cur_track = -1
        self._cur_section = -1
        self._cur_pattern_offset = Timestamp()
        self._cur_pattern = 0
        self.playing = False

    def play_event(self, channel, event):
        if not self.playing:
            self.playing = True
            self.p.handle.fire(0, ['Ipause', None])
        self.p.handle.fire(channel, event)

    def _play_track(self, ev):
        self.play_track(self._cur_track)

    def get_view(self):
        widget = QtGui.QWidget()
        layout = QtGui.QHBoxLayout(widget)
        bwidget = QtGui.QWidget()
        blayout = QtGui.QHBoxLayout(bwidget)
        layout.setMargin(0)
        layout.setSpacing(0)
        blayout.setMargin(0)
        blayout.setSpacing(0)

        play = QtGui.QToolButton()
        play.setText('Play')
        play.setIcon(QtGui.QIcon.fromTheme('media-playback-start'))
        play.setAutoRaise(True)
        QtCore.QObject.connect(play, QtCore.SIGNAL('clicked()'),
                               lambda: self._play_track(None))

        play_inf = QtGui.QToolButton()
        play_inf.setText('inf')
        play_inf.setAutoRaise(True)
        QtCore.QObject.connect(play_inf, QtCore.SIGNAL('clicked()'),
                lambda: self.play_track(self._cur_track, True))

        stop = QtGui.QToolButton()
        stop.setText('Stop')
        stop.setIcon(QtGui.QIcon.fromTheme('media-playback-stop'))
        stop.setAutoRaise(True)
        QtCore.QObject.connect(stop, QtCore.SIGNAL('clicked()'),
                               lambda: self.stop())

        seek_back = QtGui.QToolButton()
        seek_back.setText('Seek backwards')
        seek_back.setIcon(QtGui.QIcon.fromTheme('media-seek-backward'))
        seek_back.setAutoRaise(True)

        seek_for = QtGui.QToolButton()
        seek_for.setText('Seek forwards')
        seek_for.setIcon(QtGui.QIcon.fromTheme('media-seek-forward'))
        seek_for.setAutoRaise(True)

        state_but = QtGui.QToolButton()
        state_but.setText('State')
        state_but.setIcon(QtGui.QIcon.fromTheme('computer'))
        state_but.setAutoRaise(True)
        QtCore.QObject.connect(state_but, QtCore.SIGNAL('clicked()'),
                               self.p.state_window)

        self._pos_display = PosDisplay(self.p.project)
        self._pos_display.init()

        #subsong_select = QtGui.QLabel('[subsong select]')

        #tempo_factor = QtGui.QLabel('[tempo factor]')

        blayout.addWidget(play)
        blayout.addWidget(play_inf)
        blayout.addWidget(stop)
        blayout.addWidget(seek_back)
        blayout.addWidget(seek_for)
        blayout.addWidget(state_but)

        layout.addWidget(bwidget)
        layout.addWidget(self._pos_display)
        #layout.addWidget(subsong_select)
        #layout.addWidget(tempo_factor)

        return widget

    def subsong_changed(self, num):
        self._cur_track = num


    def stop(self):
        self._pos_display.set_stop()
        if self.playing:
            self.p.project.update_random()
        self.playing = False
        self.p.handle.nanoseconds = 0
        self.p._peak_meter.set_peaks(float('-inf'), float('-inf'), 0, 0, 0)
        self.bufs = (None, None)

    def play_track(self, track, infinite=False):
        self._pos_display.set_play(infinite)
        self.p._peak_meter.reset()
        if self.playing:
            self.p.project.update_random()
        self.p.handle.nanoseconds = 0
        self.p.handle.track = track
        self.playing = True
        self._set_infinite(infinite)

    def play_pattern(self, pattern, infinite=False):
        self._pos_display.set_play(infinite)
        self.p._peak_meter.reset()
        if self.playing:
            self.p.project.update_random()
        self.p.handle.track = self._cur_track
        self.p.handle.nanoseconds = 0
        self.playing = True
        self.p.handle.fire(0, ['Ipattern', pattern])
        self._set_infinite(infinite)

    def play_from(self, track, section, beats, rem, infinite=False):
        self._pos_display.set_play(infinite)
        self.p._peak_meter.reset()
        if self.playing:
            self.p.project.update_random()
        self.p.handle.track = track
        self.p.handle.nanoseconds = 0
        self.playing = True
        self.p.handle.fire(0, ['I.gs', section])
        self.p.handle.fire(0, ['I.gr', [beats, rem]])
        self.p.handle.fire(0, ['Ig', None])
        self._set_infinite(infinite)

    def _set_infinite(self, x):
        #self._infinite = x
        self.p.handle.fire(0, ['I.infinite', x])


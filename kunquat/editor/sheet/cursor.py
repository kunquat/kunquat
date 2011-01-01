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
import string
import sys

from PyQt4 import QtCore

import accessors as acc
import kunquat.editor.kqt_limits as lim
import kunquat.editor.timestamp as ts
import trigger
import trigger_row


class Cursor(QtCore.QObject):

    field_edit = QtCore.pyqtSignal(bool, name='fieldEdit')

    def __init__(self,
                 length,
                 beat_len,
                 accessors,
                 playback_manager,
                 instrument_spin,
                 parent=None):
        QtCore.QObject.__init__(self, parent)
        self.playback_manager = playback_manager
        self._instrument_spin = instrument_spin
        self.init_speed = 1
        self.max_speed = 12
        self.init_trigger_delay = 6
        self.cur_trigger_delay = self.init_trigger_delay
        self.trigger_delay_left = 0
        self.ts = ts.Timestamp()
        self.set_length(length)
        self.set_beat_len(beat_len)
        self.insert = False
        self._edit = False
        self.col = None
        self.set_index(0)
        self.set_view_start(0)
        self.set_accel(1.18)
        self.direction = 0
        self.triggers = []
        self.accessors = accessors
        self.active_accessor = None
        self.valid_value = False
        self.note_input = None
        self.scale = None
        self.pattern_path = None
        self.project = None
        self.inst_num = 0
        self.inst_auto = True

    def clear_delay(self):
        self.trigger_delay_left = 0

    @property
    def edit(self):
        return self._edit

    @edit.setter
    def edit(self, value):
        self._edit = value
        QtCore.QObject.emit(self, QtCore.SIGNAL('fieldEdit(bool)'), value)

    def key_press(self, ev):
        if ev.key() == QtCore.Qt.Key_Up:
            self.set_direction(-1)
            self.step()
        elif ev.key() == QtCore.Qt.Key_Down:
            self.set_direction(1)
            self.step()
        elif ev.key() == QtCore.Qt.Key_PageUp:
            self.set_direction()
            self.set_pos(self.ts - 4)
        elif ev.key() == QtCore.Qt.Key_PageDown:
            self.set_direction()
            self.set_pos(self.ts + 4)
        elif ev.key() == QtCore.Qt.Key_Home:
            self.set_direction()
            self.set_pos(self.ts * 0)
        elif ev.key() == QtCore.Qt.Key_End:
            self.set_direction()
            self.set_pos(self.length)
        elif ev.key() == QtCore.Qt.Key_Left:
            if self.ts in self.col.get_triggers():
                slots = self.col.get_triggers()[self.ts].slots()
                if self.index <= 0:
                    if self.insert:
                        self.index = 0
                    else:
                        self.index = sys.maxsize
                        ev.ignore()
                elif self.index >= slots:
                    self.index = slots - 1
                else:
                    self.index -= 1
            else:
                self.index = sys.maxsize
                ev.ignore()
        elif ev.key() == QtCore.Qt.Key_Right:
            if self.ts in self.col.get_triggers():
                if not self.insert:
                    slots = self.col.get_triggers()[self.ts].slots()
                    if self.index >= slots:
                        self.index = 0
                        ev.ignore()
                    else:
                        self.index += 1
            else:
                ev.ignore()
        elif ev.key() == QtCore.Qt.Key_Insert:
            self.insert = True
            return
        elif ev.key() == QtCore.Qt.Key_Escape:
            self.edit = False
            self.insert = False
            if self.active_accessor:
                self.active_accessor.hide()
                self.active_accessor = None
        elif ev.key() == QtCore.Qt.Key_Delete:
            tr = self.col.get_triggers()
            if self.insert:
                self.insert = False
            elif self.ts in tr:
                row = tr[self.ts]
                tindex, findex = row.get_slot(self)
                if tindex < len(row) and findex == 0:
                    play_note_off = False
                    if row[tindex][0] == 'cn+':
                        play_note_off = True
                    del row[tindex]
                    if row == []:
                        del tr[self.ts]
                    self.project[self.col_path] = self.col.flatten()
                    if play_note_off:
                        self.playback_manager.play_event(
                                self.col.get_num(), '["cn-", []]')
        elif ev.key() == QtCore.Qt.Key_Return:
            if not self.edit:
                self.edit = True
                tr = self.col.get_triggers()
                if self.ts in tr:
                    field, valid_func = tr[self.ts].get_field_info(self)
                else:
                    field = trigger.TriggerType('')
                    valid_func = None
                    self.index = 0
                if not valid_func:
                    valid_func = (trigger.is_global
                                  if self.col.get_num() == -1
                                  else trigger.is_channel)
                self.active_accessor = self.accessors[type(field)]
                self.active_accessor.set_validator_func(valid_func)
                if type(field) == trigger.Note:
                    field = '{0:.1f}'.format(field)
                self.active_accessor.set_value(field)
                self.active_accessor.show()
                self.active_accessor.setFocus()
            return
        else:
            self.direct_edit(ev)
            if ev.isAccepted():
                return
            ev.accept()
            if ev.key() == QtCore.Qt.Key_1:
                self.note_off_key(ev)
                return
            else:
                self.note_on_key(ev)
                return
        self.insert = False

    def direct_edit(self, ev):
        triggers = self.col.get_triggers()
        if self.ts in triggers and not self.insert:
            row = triggers[self.ts]
            tindex, findex = row.get_slot(self)
            if tindex < len(row):
                trig = row[tindex]
                int_keys = string.digits + '-'
                float_keys = int_keys + '.'
                direct = False
                info = trig.get_field_info(findex)[0]
                if isinstance(info, int) and \
                        ev.key() < 256 and chr(ev.key()) in int_keys:
                    direct = True
                elif isinstance(info, float) and \
                        not isinstance(info, trigger.Note) and \
                        ev.key() < 256 and chr(ev.key()) in float_keys:
                    direct = True
                if direct:
                    self.edit = True
                    field, valid_func = row.get_field_info(self)
                    self.active_accessor = self.accessors[type(field)]
                    self.active_accessor.set_validator_func(valid_func)
                    self.active_accessor.set_value(chr(ev.key()))
                    self.active_accessor.show()
                    self.active_accessor.setFocus()
                    return
        ev.ignore()

    def note_off_key(self, ev):
        if self.col.get_num() < 0:
            ev.ignore()
            return
        triggers = self.col.get_triggers()
        play_note_off = False
        if self.ts in triggers and not self.insert:
            row = triggers[self.ts]
            tindex, findex = row.get_slot(self)
            if tindex < len(row):
                trig = row[tindex]
                if trig[0] in ('cn+', 'cn-'):
                    del row[tindex]
                    self.insert = True
                    self.col.set_value(self, trigger.TriggerType('cn-'))
                    self.insert = False
                    self.project[self.col_path] = self.col.flatten()
                    play_note_off = True
            else:
                self.index = row.slots()
                self.col.set_value(self, trigger.TriggerType('cn-'))
                self.project[self.col_path] = self.col.flatten()
                play_note_off = True
        else:
            if self.ts not in triggers:
                self.index = 0
            self.col.set_value(self, trigger.TriggerType('cn-'))
            self.project[self.col_path] = self.col.flatten()
            play_note_off = True
        if play_note_off:
            self.playback_manager.play_event(
                    self.col.get_num(), '["cn-", []]')
        self.insert = False

    def note_on_key(self, ev):
        if not (self.note_input and self.scale and
                self.col.get_num() >= 0):
            ev.ignore()
            return
        try:
            note, octave = self.note_input.get_note(ev.key())
            cents = self.scale.get_cents(note, octave)
        except KeyError:
            ev.ignore()
            return
        triggers = self.col.get_triggers()
        play_note_on = False
        if self.ts in triggers and not self.insert:
            row = triggers[self.ts]
            tindex, findex = row.get_slot(self)
            if tindex < len(row):
                trig = row[tindex]
                if trig[0] == 'cn+':
                    self.col.set_value(self, cents)
                    self.project[self.col_path] = self.col.flatten()
                    play_note_on = True
                elif trig[0] == 'cn-':
                    self.col.set_value(self, trigger.TriggerType('cn+'))
                    self.col.set_value(self, cents)
                    self.project[self.col_path] = self.col.flatten()
                    play_note_on = True
                elif isinstance(trig.get_field_info(findex)[0],
                                trigger.Note):
                    self.col.set_value(self, cents)
                    self.project[self.col_path] = self.col.flatten()
                    # TODO: should we play?
            else:
                self.index = row.slots()
                self.col.set_value(self, trigger.TriggerType('cn+'))
                self.col.set_value(self, cents)
                if self.inst_auto:
                    use_existing_trig = False
                    self.index -= 1
                    if self.index >= 1:
                        ptindex, _ = row.get_slot(self)
                        if row[ptindex][0] == 'c.i':
                            use_existing_trig = True
                    self.index += 1
                    if use_existing_trig:
                        self.index -= 1
                    else:
                        self.insert = True
                        self.col.set_value(self,
                                           trigger.TriggerType('c.i'))
                        self.insert = False
                        self.index += 1
                    self.col.set_value(self, self.inst_num)
                    self.index += 1
                self.project[self.col_path] = self.col.flatten()
                play_note_on = True
        else:
            if self.ts not in triggers:
                self.index = 0
            self.col.set_value(self, trigger.TriggerType('cn+'))
            self.insert = False
            self.col.set_value(self, cents)
            if self.inst_auto:
                use_existing_trig = False
                self.index -= 1
                if self.index >= 1:
                    row = triggers[self.ts]
                    ptindex, _ = row.get_slot(self)
                    if row[ptindex][0] == 'c.i':
                        use_existing_trig = True
                self.index += 1
                if use_existing_trig:
                    self.index -= 1
                else:
                    self.insert = True
                    self.col.set_value(self, trigger.TriggerType('c.i'))
                    self.insert = False
                    self.index += 1
                self.col.set_value(self, self.inst_num)
                self.index += 1
            self.project[self.col_path] = self.col.flatten()
            play_note_on = True
        if play_note_on:
            self.playback_manager.play_event(self.col.get_num(),
                                '["c.i", [{0}]]'.format(self.inst_num))
            self.playback_manager.play_event(self.col.get_num(),
                                        '["cn+", [{0}]]'.format(cents))
        self.insert = False

    def key_release(self, ev):
        if ev.key() in (QtCore.Qt.Key_Up, QtCore.Qt.Key_Down):
            self.set_direction()
        else:
            ev.ignore()

    def set_beat_len(self, beat_len):
        assert beat_len > 0
        self.beat_len = beat_len
        self.set_pos(self.ts)

    def set_col_path(self):
        if self.col:
            if self.col.get_num() == -1:
                self.col_path = '/'.join((self.pattern_path, 'gcol',
                                          'p_global_events.json'))
            else:
                col_dir = 'ccol_{0:02x}'.format(self.col.get_num())
                self.col_path = '/'.join((self.pattern_path, col_dir,
                                          'p_channel_events.json'))

    def set_geometry(self, x, y, w, h):
        padding = 4
        x -= padding
        y -= padding
        w += padding * 2
        h += padding * 2
        self.geom = QtCore.QRect(x, y, w, h)
        for a in self.accessors:
            if a == trigger.Note:
                extra_pad = 30
                cents_geom = QtCore.QRect(x, y, w + extra_pad, h)
                self.accessors[a].setGeometry(cents_geom)
            else:
                self.accessors[a].setGeometry(self.geom)

    def set_length(self, length):
        assert length >= 0
        self.length = length

    def set_path(self, path):
        self.pattern_path = path
        self.set_col_path()

    def set_project(self, project):
        self.project = project

    def set_triggers(self, triggers):
        self.triggers = triggers

    def set_col(self, col):
        assert col
        self.col = col
        self.set_col_path()

    def set_pos(self, timestamp):
        self.ts = min(max(ts.Timestamp(), timestamp), self.length)
        self.pix_pos = self.ts * self.beat_len

    def set_pix_pos(self, pix_pos):
        self.pix_pos = float(max(0, pix_pos))
        self.ts = ts.Timestamp(self.pix_pos / self.beat_len)
        if self.ts > self.length:
            self.set_pos(self.length)

    def get_pos(self):
        return self.ts

    def get_pix_pos(self):
        return self.pix_pos

    def set_index(self, index):
        assert index >= 0
        self.index = index

    def get_index(self):
        return self.index

    def set_view_start(self, start):
        assert start >= 0
        self.view_start = start

    def get_view_start(self):
        return self.view_start

    def set_scale(self, sc):
        self.scale = sc

    def set_input(self, note_input):
        self.note_input = note_input

    def set_value(self):
        assert self.active_accessor
        assert self.active_accessor.hasFocus()
        self.valid_value = True
        value = self.active_accessor.get_value()
        self.col.set_value(self, value)
        if self.active_accessor is self.accessors[int]:
            row = self.col.get_triggers()[self.ts]
            tindex, _ = row.get_slot(self)
            if self.index > 0 and row[tindex][0] == 'c.i':
                self.inst_num = value
                self._instrument_spin.setValue(value)
        self.edit = False
        self.insert = False
        self.active_accessor.hide()
        self.active_accessor = None
        self.project[self.col_path] = self.col.flatten()

    def set_accel(self, accel):
        assert accel >= 1
        self.accel = accel

    def set_direction(self, direction=0):
        assert direction in (-1, 0, 1)
        if self.direction == direction:
            return
        self.direction = direction
        self.cur_speed = self.init_speed * direction
        self.cur_trigger_delay = self.init_trigger_delay
        self.trigger_delay_left = 0

    def step(self):
        if not self.cur_speed:
            return
        if self.trigger_delay_left:
            self.trigger_delay_left -= 1
            return
        orig_pos = self.ts
        self.set_pix_pos(self.pix_pos + self.cur_speed)
        if self.ts == orig_pos:
            self.set_pos(self.ts + ts.Timestamp(0,
                                       1 if self.cur_speed > 0 else -1))

        first, second = ((orig_pos, self.ts) if orig_pos < self.ts
                                             else (self.ts, orig_pos))
        rows = [p for p in self.col.get_triggers() if first <= p <= second
                                                      and p != orig_pos]
        if rows:
            pos = min(rows) if orig_pos < self.ts else max(rows)
            self.set_pos(pos)
            self.trigger_delay_left = self.cur_trigger_delay
            if self.cur_trigger_delay:
                self.cur_trigger_delay -= 1

        if abs(self.cur_speed) == self.max_speed:
            return
        self.cur_speed *= self.accel
        if abs(self.cur_speed) > self.max_speed:
            self.cur_speed = (self.max_speed if self.cur_speed > 0
                                             else -self.max_speed)



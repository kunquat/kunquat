# coding=utf-8


# Copyright 2009 Tomi Jylhä-Ollila
#
# This file is part of Kunquat.
#
# Kunquat is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Kunquat is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.


import pygtk
pygtk.require('2.0')
import gtk, gobject, cairo, pango
from gtk import gdk
import math

import liblo

from Pat_helper import *


evtype = Ev()


class Pat_info:
    
    def __init__(self, num, len):
        self.num = num
        self.len = len
        self.cols = [None for _ in range(COLUMNS + 1)]


class Nt_info:

    def __init__(self, name, notes, note_mods):
        self.name = name
        self.notes = [None for _ in range(notes)]
        self.note_mods = [None for _ in range(note_mods)]


class Pat_view(gtk.Widget):

    def set_tempo(self, tempo):
        self.tempo = tempo

    def set_subsong(self, subsong):
        self.subsong = subsong

    def pat_info(self, path, args, types):
        if 'pat_info' in path:
            self.pdata = Pat_info(args[0], (args[1], args[2]))
        if self.cursor[0][:2] > self.pdata.len:
            self.cursor = (self.pdata.len + (0,), self.cursor[1])
        if 'pat_meta' in path:
            self.pdata.len = (args[1], args[2])
            self.queue_draw()

    def event_info(self, path, args, types):
        if not self.pdata: print('event_info')
        if self.pdata.cols[args[1]] == None:
            self.pdata.cols[args[1]] = {}
        self.pdata.cols[args[1]][(args[2], args[3], args[4])] = args[5:]

    def events_sent(self, path, args, types):
        self.queue_draw()

    def note_table_info(self, path, args, types):
        if args[0] != 0:
            print(args[0])
            return
        self.notes = Nt_info(args[1], args[2], args[3])

    def note_info(self, path, args, types):
        if args[0] != 0: # TODO: viewing of other note tables
            return
        if args[1] < len(self.notes.notes):
            self.notes.notes[args[1]] = args[2]

    def note_mod_info(self, path, args, types):
        if args[0] != 0: # TODO: viewing of other note tables
            return
        if args[1] < len(self.notes.note_mods):
            self.notes.note_mods[args[1]] = args[2]

    def notes_sent(self, path, args, types):
        self.queue_draw()

    def ins_info(self, path, args, types):
        if args[1] != 0:
            self.instruments[args[0]] = args[1]
        elif args[0] in self.instruments:
            del self.instruments[args[0]]

#   def handle_motion_notify(self, widget, event):
#       print('motion')
#       return True

    def handle_button_press(self, widget, event):
        print('button')
        self.grab_focus()
        return True

    def handle_focus_in(self, widget, event):
        print('focus in')
        return True

    def handle_focus_out(self, widget, event):
        print('focus out')
        return True

    def get_snap(self):
        if 0 <= self.snap_state < self.snap_delay:
            self.snap_state = max(-1, self.snap_state - 1)
            if self.snap_state < 0:
                self.snap_delay = max(0, self.snap_delay - self.snap_step)
                self.snap_state = self.snap_delay
            return True
        return False

    def reset_snap(self):
        self.snap_state = self.snap_delay = self.snap_init_delay

    def accel_cursor(self, dir):
        init_move = False
        cond = False
        if dir < 0:
            cond = self.tmove[1] >= 0
        else:
            cond = self.tmove[1] <= 0
        if cond:
            init_move = True
            self.tmove = (self.init_cur_speed, dir)
        elif self.tmove[0] < self.max_cur_speed:
            self.tmove = (self.tmove[0] * self.cur_accel, dir)
        else:
            self.tmove = (self.max_cur_speed, dir)
        return init_move

    def act_up(self, event):
        self.cur_field = self.cur_virtual_field
        ctime = (0L, 0)
        self.cur_inserting = False
        if event.type == gdk.SCROLL:
            ctime = (-1L, 3 * RELTIME_FULL_PART / 4)
        elif event.type == gdk.KEY_PRESS:
            if self.tmove != (0, 0) and self.get_snap():
                return
            else:
                init_move = self.accel_cursor(-1)
                ctime = time_sub(self.cursor[0][:2], self.px_time(self.tmove[0]))
                if ctime == self.cursor[0][:2]:
                    ctime = time_sub(ctime, (0, 1))
                events = []
                if self.pdata.cols[self.cursor[1]]:
                    events = [(k, v) for (k, v) in self.pdata.cols[self.cursor[1]].iteritems()
                            if ctime <= k < self.cursor[0][:2]]
                if events:
                    last = max(events)
                    ctime = last[0][:2]
                    if not init_move:
                        self.snap_state = self.snap_delay - 1
        elif event.type == gdk.KEY_RELEASE:
            self.tmove = (0, 0)
            self.reset_snap()
            return
        self.cursor = (ctime + (self.cur_virtual_ord,), self.cursor[1])
        self.queue_draw()

    def act_down(self, event):
        self.cur_field = self.cur_virtual_field
        ctime = (0L, 0)
        self.cur_inserting = False
        if event.type == gdk.SCROLL:
            ctime = (0L, RELTIME_FULL_PART / 4)
        elif event.type == gdk.KEY_PRESS:
            if self.tmove != (0, 0) and self.get_snap():
                return
            else:
                init_move = self.accel_cursor(1)
                ctime = time_add(self.cursor[0][:2], self.px_time(self.tmove[0]))
                if ctime == self.cursor[0][:2]:
                    ctime = time_add(ctime, (0, 1))
                events = []
                if self.pdata.cols[self.cursor[1]]:
                    events = [(k, v) for (k, v) in self.pdata.cols[self.cursor[1]].iteritems()
                            if self.cursor[0][:2] < k[:2] <= ctime]
                if events:
                    first = min(events)
                    ctime = first[0][:2]
                    if not init_move:
                        self.snap_state = self.snap_delay - 1
        elif event.type == gdk.KEY_RELEASE:
            self.tmove = (0, 0)
            self.reset_snap()
            return
        self.cursor = (ctime + (self.cur_virtual_ord,), self.cursor[1])
        self.queue_draw()

    def act_ins_col_space(self, event):
        self.cur_inserting = False
        ctime = (0L, 0)
        if event.type == gdk.KEY_PRESS:
            init_move = self.accel_cursor(1)
            ctime = self.px_time(self.tmove[0])
            if ctime == (0L, 0):
                ctime = (0L, 1)
            liblo.send(self.engine, '/kunquat/pat_shift_down',
                    self.song_id,
                    self.pdata.num,
                    self.cursor[1],
                    self.cursor[0][0],
                    self.cursor[0][1],
                    ctime[0],
                    ctime[1])
        elif event.type == gdk.KEY_RELEASE:
            self.tmove = (0, 0)
            return

    def act_del_col_space(self, event):
        self.cur_inserting = False
        ctime = (0L, 0)
        if event.type == gdk.KEY_PRESS:
            if self.tmove != (0, 0) and self.get_snap():
                return
            else:
                init_move = self.accel_cursor(1)
                ctime = self.px_time(self.tmove[0])
                if ctime == (0L, 0):
                    ctime = (0L, 1)
                events = []
                end = time_add(self.cursor[0][:2], ctime)
                if self.pdata.cols[self.cursor[1]]:
                    if self.cursor[0] in self.pdata.cols[self.cursor[1]]:
                        liblo.send(self.engine, '/kunquat/pat_del_row',
                                self.song_id,
                                self.pdata.num,
                                self.cursor[1],
                                self.cursor[0][0],
                                self.cursor[0][1])
                        return
                    events = [(k, v) for (k, v) in self.pdata.cols[self.cursor[1]].items()
                            if self.cursor[0][:2] < k[:2] <= end]
                if events:
                    first = min(events)
                    end = first[0][:2]
                    ctime = time_sub(end, self.cursor[0][:2])
                    if not init_move:
                        self.snap_state = self.snap_delay - 1
        elif event.type == gdk.KEY_RELEASE:
            self.tmove = (0, 0)
            self.reset_snap()
            return
        liblo.send(self.engine, '/kunquat/pat_shift_up',
                self.song_id,
                self.pdata.num,
                self.cursor[1],
                self.cursor[0][0],
                self.cursor[0][1],
                ctime[0],
                ctime[1])

    def act_zoom_in(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.zoom(1.5)
        self.queue_draw()

    def act_zoom_out(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.zoom(1.0/1.5)
        self.queue_draw()

    def act_zoom_h_in(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.zoom_h(1.25)
        self.queue_draw()

    def act_zoom_h_out(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.zoom_h(1.0/1.25)
        self.queue_draw()

    def act_left(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.cur_inserting = False
        if (self.cursor[1] == 0
                and self.cursor[0][2] == 0
                and self.cur_field == 0):
            return
        self.cursor_clip()
        new_field = self.cur_field - 1
        new_ord = self.cursor[0][2]
        new_col = self.cursor[1]
        row = []
        last_field = 0
        if new_field < 0:
            new_ord -= 1
            if new_ord < 0:
                new_col -= 1
                if self.pdata.cols[new_col]:
                    row = [(k, v) for (k, v) in self.pdata.cols[new_col].items()
                            if self.cursor[0][:2] == k[:2]]
                new_ord = len(row)
            if (self.pdata.cols[new_col] and self.cursor[0][:2] +
                    (new_ord,) in self.pdata.cols[new_col]):
                ev = self.pdata.cols[new_col][self.cursor[0][:2] + (new_ord,)]
                last_field = self.event_fields[ev[0]] - 1
            new_field = last_field
        self.cursor = (self.cursor[0][:2] + (new_ord,), new_col)
        self.cur_field = new_field
        self.cur_virtual_ord = self.cursor[0][2]
        self.cur_virtual_field = self.cur_field
        self.queue_draw()
#       print(str(self.cursor) +
#               ', vord ' + str(self.cur_virtual_ord) +
#               ', vfield ' + str(self.cur_virtual_field))

    def act_right(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        if self.cur_inserting:
            self.cur_inserting = False
            self.queue_draw()
            return
        if (self.pdata.cols[self.cursor[1]] and
                self.cursor[0] in self.pdata.cols[self.cursor[1]]):
            ev = self.pdata.cols[self.cursor[1]][self.cursor[0]]
            if self.cur_field < self.event_fields[ev[0]] - 1:
                self.cur_field += 1
                self.cur_virtual_field = self.cur_field
            else:
                self.cur_field = 0
                self.cur_virtual_field = 0
                self.cur_virtual_ord += 1
                self.cursor = (self.cursor[0][:2] +
                        (self.cur_virtual_ord,), self.cursor[1])
        else:
            self.cur_virtual_ord = 0
            self.cursor = (self.cursor[0][:2] + (0,), self.cursor[1] + 1)
            self.cur_field = 0
            self.cur_virtual_field = 0
        self.queue_draw()

    def act_bar_up(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.cur_inserting = False
        self.cursor = ((self.cursor[0][0] - 4,
                self.cursor[0][1],
                self.cur_virtual_ord), self.cursor[1])
        self.queue_draw()

    def act_bar_down(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.cur_inserting = False
        self.cursor = ((self.cursor[0][0] + 4,
                self.cursor[0][1],
                self.cur_virtual_ord), self.cursor[1])
        self.queue_draw()

    def act_ch_left(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.cur_inserting = False
        if self.cursor[1] > 0:
            self.cursor = (self.cursor[0], self.cursor[1] - 1)
            self.queue_draw()

    def act_ch_right(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.cur_inserting = False
        if self.cursor[1] < COLUMNS:
            self.cursor = (self.cursor[0], self.cursor[1] + 1)
            self.queue_draw()

    def act_ev_note_on(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        key_name = gdk.keyval_name(self.get_plain_key(event))
        event_args = ()
        edit_method = '/kunquat/pat_ins_event'
        inserting = self.cur_inserting
        self.cur_inserting = False
        if not inserting and (self.pdata.cols[self.cursor[1]]
                and self.cursor[0] in self.pdata.cols[self.cursor[1]]):
            edit_method = '/kunquat/pat_mod_event'
            if self.cur_field > 0:
                ev = self.pdata.cols[self.cursor[1]][self.cursor[0]]
                note = ev[1]
                note_mod = ev[2]
                octave = ev[3]
                ins_num = ev[4]
                if key_name.isdigit() or 'a' <= key_name <= 'f':
                    if self.cur_field == 2:
                        ins_num = long((16 * long(key_name, 16)) + (ins_num % 16))
                    elif self.cur_field == 3:
                        ins_num = long(((ins_num // 16) * 16) + (long(key_name, 16) % 16))
                    elif self.cur_field == 1 and (key_name.isdigit() or
                            'a' <= key_name <= 'c'):
                        octave = long(key_name, 16)
                        if self.ev_note_on_got_minus:
                            self.ev_note_on_got_minus = 0
                            if key_name.isdigit() and int(key_name) <= 3:
                                octave = -octave
                            else:
                                self.queue_draw()
                                return
                    event_args = (note, note_mod, octave, ins_num)
                elif key_name == 'minus':
                    self.ev_note_on_got_minus = 2
                    self.queue_draw()
                    return
                else:
                    self.queue_draw()
                    return
            else:
                note, rel_octave = self.note_keys[key_name]
                event_args = (note, -1L, rel_octave + self.base_octave, long(self.ins_num))
        elif key_name in self.note_keys:
            note, rel_octave = self.note_keys[key_name]
            event_args = (note, -1L, rel_octave + self.base_octave, long(self.ins_num))
        else:
            return
        liblo.send(self.engine, edit_method,
                self.song_id,
                self.pdata.num,
                self.cursor[1],
                self.cursor[0][0],
                self.cursor[0][1],
                self.cursor[0][2],
                evtype.NOTE_ON,
                *event_args)
        if self.cursor[1] > 0:
            liblo.send(self.engine, '/kunquat/play_event',
                    self.song_id,
                    self.cursor[1],
                    evtype.NOTE_ON,
                    *event_args)
        if (event.state & self.note_chord_mod[1]
                and self.cursor[1] < COLUMNS):
            if self.cur_virtual_col == -1:
                self.cur_virtual_col = self.cursor[1]
            self.cursor = (self.cursor[0], self.cursor[1] + 1)
            self.queue_draw()

    def act_ev_note_off(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        key_name = gdk.keyval_name(self.get_plain_key(event))
        if not key_name == '1':
            return
        edit_method = '/kunquat/pat_ins_event'
        if not self.cur_inserting and (self.pdata.cols[self.cursor[1]]
                and self.cursor[0] in self.pdata.cols[self.cursor[1]]):
            edit_method = '/kunquat/pat_mod_event'
        self.cur_inserting = False
        liblo.send(self.engine, edit_method,
                self.song_id,
                self.pdata.num,
                self.cursor[1],
                self.cursor[0][0],
                self.cursor[0][1],
                self.cursor[0][2],
                evtype.NOTE_OFF)
        if self.cursor[1] > 0:
            liblo.send(self.engine, '/kunquat/play_event',
                    self.song_id,
                    self.cursor[1],
                    evtype.NOTE_OFF)

    def act_insert_gap(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        if not (self.pdata.cols[self.cursor[1]]
                and self.cursor[0] in self.pdata.cols[self.cursor[1]]):
            self.cur_inserting = False
            return
        self.cur_inserting = True
        self.queue_draw()

    def act_del_event(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        if self.cur_inserting:
            self.cur_inserting = False
            return
        if (self.pdata.cols[self.cursor[1]]
                and self.cursor[0] in self.pdata.cols[self.cursor[1]]):
            liblo.send(self.engine, '/kunquat/pat_del_event',
                    self.song_id,
                    self.pdata.num,
                    self.cursor[1],
                    self.cursor[0][0],
                    self.cursor[0][1],
                    self.cursor[0][2])
            if self.cursor[1] > 0:
                liblo.send(self.engine, '/kunquat/play_event',
                        self.song_id,
                        self.cursor[1],
                        evtype.NOTE_OFF)

    def act_pat_prev(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.cur_inserting = False
        if not self.pdata or self.pdata.num <= 0:
            return
        liblo.send(self.engine, '/kunquat/get_pattern', self.song_id, self.pdata.num - 1)

    def act_pat_next(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.cur_inserting = False
        if not self.pdata or self.pdata.num >= PATTERNS:
            return
        liblo.send(self.engine, '/kunquat/get_pattern', self.song_id, self.pdata.num + 1)

    def set_octave(self, octave):
        if not -3 <= octave <= 0xc:
            return
        self.base_octave = octave
        self.base_octave_adj.set_value(octave)

    def act_octave_up(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.set_octave(self.base_octave + 1)

    def act_octave_down(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.set_octave(self.base_octave - 1)

    def set_ins(self, num):
        if not 1 <= num <= 255: # TODO: accept zero after implementing
            return
        self.ins_num = num
        self.ins_adj.set_value(num)

    def act_ins_next(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.set_ins(self.ins_num + 1)

    def act_ins_prev(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        self.set_ins(self.ins_num - 1)

    def act_play_song(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        liblo.send(self.engine, '/kunquat/play_subsong',
                self.song_id,
                self.subsong)

    def act_play_pat(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        liblo.send(self.engine, '/kunquat/play_pattern',
                self.song_id,
                self.pdata.num,
                self.tempo)

    def act_stop(self, event):
        if event.type == gdk.KEY_RELEASE:
            return
        liblo.send(self.engine, '/kunquat/stop_song', self.song_id)

    def get_plain_key(self, event):
        keymap = gdk.keymap_get_default()
        keyval, _, _, _ = keymap.translate_keyboard_state(
                event.hardware_keycode, 0, event.group)
        return keyval

    def handle_key(self, widget, event):
        if not event.type == gdk.KEY_PRESS:
            return False
        if self.pdata == None:
            return True
        key_name = gdk.keyval_name(self.get_plain_key(event))
        key_mask = 0
        key_mask |= event.state & gdk.CONTROL_MASK
        key_mask |= event.state & gdk.SHIFT_MASK
        key_mask |= event.state & gdk.MOD1_MASK
        #key_mask |= event.state & gdk.MOD2_MASK # This acts in a weird way...
        key_mask |= event.state & gdk.MOD3_MASK
        key_mask |= event.state & gdk.MOD4_MASK
        key_mask |= event.state & gdk.MOD5_MASK
        if (key_name, key_mask) in self.control_map:
            self.control_map[(key_name, key_mask)](event)
        elif (self.cur_field > 0
                and self.pdata.cols[self.cursor[1]]
                and self.cursor[0] in self.pdata.cols[self.cursor[1]]):
            ev = self.pdata.cols[self.cursor[1]][self.cursor[0]]
            self.act_event[ev[0]](event)
        elif key_name in self.note_keys:
            self.act_ev_note_on(event)
        elif (key_name, key_mask) in self.event_map:
            self.event_map[(key_name, key_mask)](event)
        else:
            print('press %s, %s, %s' % (key_name, event.hardware_keycode, event.state))
        if self.ev_note_on_got_minus == 1:
            self.ev_note_on_got_minus = 0
        return True

    def handle_release(self, widget, event):
        key_name = gdk.keyval_name(self.get_plain_key(event))
        key_mask = 0
        key_mask |= event.state & gdk.CONTROL_MASK
        key_mask |= event.state & gdk.SHIFT_MASK
        key_mask |= event.state & gdk.MOD1_MASK
        #key_mask |= event.state & gdk.MOD2_MASK
        key_mask |= event.state & gdk.MOD3_MASK
        key_mask |= event.state & gdk.MOD4_MASK
        key_mask |= event.state & gdk.MOD5_MASK
#       print('release %s, %s' % (event.keyval, key_name))
        if (key_name, key_mask) in self.control_map:
            self.control_map[(key_name, key_mask)](event)
        if (key_name[:len(self.note_chord_mod[0])] == self.note_chord_mod[0]
                and self.cur_virtual_col >= 0):
            self.cursor = (self.cursor[0], self.cur_virtual_col)
            self.cur_virtual_col = -1
            self.queue_draw()
        return True

    def handle_scroll(self, widget, event):
        dir_mask = event.direction
        key_mask = 0
        key_mask |= event.state & gdk.CONTROL_MASK
        key_mask |= event.state & gdk.SHIFT_MASK
        key_mask |= event.state & gdk.MOD1_MASK
        key_mask |= event.state & gdk.MOD2_MASK
        key_mask |= event.state & gdk.MOD3_MASK
        key_mask |= event.state & gdk.MOD4_MASK
        key_mask |= event.state & gdk.MOD5_MASK
        if (dir_mask, key_mask) in self.control_map:
            self.control_map[(dir_mask, key_mask)](event)
        return True

    def zoom(self, factor):
        new_ppb = self.pixels_per_beat * factor
        if new_ppb < 1:
            new_ppb = 1
        elif new_ppb > RELTIME_FULL_PART * 4:
            new_ppb = RELTIME_FULL_PART * 4
        cur_loc = self.time_px(time_sub(self.cursor[0][:2], self.view_corner[0]))
        self.pixels_per_beat = new_ppb
        new_loc = self.px_time(cur_loc)
        self.view_corner = (time_sub(self.cursor[0][:2], new_loc), self.view_corner[1])

    def zoom_h(self, factor):
        new_ppc = self.col_width * factor
        if new_ppc < self.col_font_size:
            new_ppc = self.col_font_size
        elif new_ppc > self.col_font_size * 20:
            new_ppc = self.col_font_size * 20
        self.col_width = int(new_ppc)

    def do_realize(self):
        self.set_flags(self.flags()
                | gtk.REALIZED
                | gtk.SENSITIVE
                | gtk.PARENT_SENSITIVE
#               | gtk.DOUBLE_BUFFERED
                | gtk.CAN_FOCUS)
        self.window = gdk.Window(
                self.get_parent_window(),
                width = self.allocation.width,
                height = self.allocation.height,
                window_type = gdk.WINDOW_CHILD,
                wclass = gdk.INPUT_OUTPUT,
                event_mask = self.get_events()
                        | gdk.EXPOSURE_MASK
                        | gdk.KEY_PRESS
                        | gdk.KEY_RELEASE
#                       | gdk.POINTER_MOTION_MASK
#                       | gdk.POINTER_MOTION_HINT_MASK
                        | gdk.BUTTON_PRESS_MASK
                        | gdk.SCROLL
                        | gdk.FOCUS_CHANGE)
        self.window.set_user_data(self)
        self.style.attach(self.window)
        self.style.set_background(self.window, gtk.STATE_NORMAL)
        self.window.move_resize(*self.allocation)
        self.connect('key-press-event', self.handle_key)
        self.connect('key-release-event', self.handle_release)
        self.connect('focus-in-event', self.handle_focus_in)
        self.connect('focus-out-event', self.handle_focus_out)
        self.connect('button-press-event', self.handle_button_press)
        self.connect('scroll-event', self.handle_scroll)
#       self.connect('motion-notify-event', self.handle_motion_notify)

    def do_unrealize(self):
        self.window.destroy()
        self.window.set_user_data(None)

    def do_size_request(self, requisition):
        pass

    def do_size_allocate(self, allocation):
        self.allocation = allocation
        if self.flags() & gtk.REALIZED:
            self.window.move_resize(*allocation)

    def do_expose_event(self, event):
        cr = self.window.cairo_create()
        cr.rectangle(event.area.x, event.area.y,
                event.area.width, event.area.height)
        cr.clip()
        self.draw(cr, *self.window.get_size())

    def draw(self, cr, width, height):
        cr.set_source_rgb(*self.ptheme['Background colour'])
        cr.rectangle(0, 0, width, height)
        cr.fill()
        cr.set_line_width(1)

        # Make sure cursor is inside Pattern
        self.cursor_clip()

        # Make sure cursor is visible
        if self.cursor[1] < self.view_corner[1]:
            self.view_corner = (self.view_corner[0], self.cursor[1])
        center_time = self.px_time(height / 2)
        self.view_corner = (time_sub(self.cursor[0][:2], center_time),
                self.view_corner[1])

        # Calculate space
        col_space = width - self.ruler_width
        col_count = col_space // self.col_width
        col_last = self.view_corner[1] + col_count - 1
        if col_last > COLUMNS:
            diff = col_last - COLUMNS
            self.view_corner = (self.view_corner[0], self.view_corner[1] - diff)
            col_last = COLUMNS
            if self.view_corner[1] < 0:
                col_count += self.view_corner[1]
                self.view_corner = (self.view_corner[0], 0)
        if self.cursor[1] > col_last:
            diff = self.cursor[1] - col_last
            self.view_corner = (self.view_corner[0], self.view_corner[1] + diff)
        beat_space = height - self.col_font_size
        beat_count = divmod(beat_space, self.pixels_per_beat)
        beat_count = (int(beat_count[0]),
                int(beat_count[1] * RELTIME_FULL_PART // self.pixels_per_beat))
        beat_last = time_add(self.view_corner[0], beat_count)
        if self.pdata and beat_last > self.pdata.len:
            diff = time_sub(beat_last, self.pdata.len)
            self.view_corner = (time_sub(self.view_corner[0], diff), self.view_corner[1])
            beat_last = self.pdata.len
        if self.view_corner[0] < time_sub((0, 0), self.px_time(self.col_font_size)):
            beat_count = time_add(beat_count,
                    time_sub(self.px_time(self.col_font_size), self.view_corner[0]))
            self.view_corner = (time_sub((0, 0),
                    self.px_time(self.col_font_size)), self.view_corner[1])
            if self.pdata and beat_last <= self.pdata.len:
                beat_last = min(time_add(self.view_corner[0], beat_count), self.pdata.len)
        if self.cursor[0][:2] > beat_last:
            diff = time_sub(self.cursor[0][:2], beat_last)
            self.view_corner = (time_add(self.view_corner[0], diff), self.view_corner[1])
            beat_last = self.cursor[0][:2]

        self.draw_ruler(cr, height, self.view_corner[0], beat_last)

        # Draw columns
        for col_num in range(self.view_corner[1], self.view_corner[1] + col_count):
            self.draw_column(cr, col_num,
                    self.ruler_width + (col_num - self.view_corner[1]) * self.col_width,
                    0, height, self.view_corner[0], beat_last)

    def time_px(self, time):
        px = float(time[0] * RELTIME_FULL_PART + time[1])
        return px * self.pixels_per_beat / RELTIME_FULL_PART

    def px_time(self, px):
        beats = float(px) / self.pixels_per_beat
        return time_normalise((long(math.floor(beats)),
                int((beats - math.floor(beats)) * RELTIME_FULL_PART)))

    def draw_ruler(self, cr, height, start, end):
        if not self.pdata:
            return
        cr.set_source_rgb(*self.ptheme['Ruler bg colour'])
        cr.rectangle(0, self.col_font_size + max(-self.time_px(start), 0),
                self.ruler_width, min(self.time_px(end), height - self.col_font_size))
        cr.fill()
        cr.set_source_rgb(*self.ptheme['Ruler fg colour'])
        mark_time_min = self.px_time(6)
        mark_distance_min = mark_time_min[0] + float(mark_time_min[1]) / RELTIME_FULL_PART
        mark_step_size = 2**math.ceil(math.log(mark_distance_min, 2))
        if mark_step_size == 0:
            mark_step_size = 2**(-1024)
        mark_limit = end[0] + float(end[1]) / RELTIME_FULL_PART
        first_mark = start[0] + float(start[1]) / RELTIME_FULL_PART
        first_mark = math.ceil(first_mark / mark_step_size) * mark_step_size
        mark_pos = max(0, first_mark)
        num_time_min = self.px_time(self.ruler_font_size * 3)
        num_distance_min = num_time_min[0] + float(num_time_min[1]) / RELTIME_FULL_PART
        num_step_size = 2**math.ceil(math.log(num_distance_min, 2))
        num_limit = end[0] + float(end[1]) / RELTIME_FULL_PART
        first_num = start[0] + float(start[1]) / RELTIME_FULL_PART
        first_num = math.ceil(first_num / num_step_size) * num_step_size
        num_pos = max(num_step_size, first_num)
        while mark_pos <= mark_limit:
            distance = self.time_px(time_sub((mark_pos, 0), start))
            distance = distance + self.col_font_size
            cr.move_to(self.ruler_width, distance)
            if mark_pos == 0 or mark_pos >= (self.pdata.len[0] +
                    float(self.pdata.len[1]) / RELTIME_FULL_PART):
                cr.rel_line_to(-self.ruler_width, 0)
            if math.floor(mark_pos) == mark_pos:
                cr.rel_line_to(-6, 0)
            else:
                cr.rel_line_to(-3, 0)
            cr.stroke()
            if mark_pos < num_pos or mark_pos >= (self.pdata.len[0] +
                    float(self.pdata.len[1]) / RELTIME_FULL_PART):
                mark_pos += mark_step_size
                continue
            mark_pos += mark_step_size
            num_disp = '%.3f' % num_pos
            posb = 1000 * num_pos
            if math.floor(posb) == posb:
                while num_disp[-1] == '0':
                    num_disp = num_disp[:-1]
                if num_disp[-1] == '.':
                    num_disp = num_disp[:-1]
            pl = self.create_pango_layout(num_disp)
            pl.set_font_description(self.ruler_font)
            rw, rh = pl.get_size()
            rw //= pango.SCALE
            rh //= pango.SCALE
            distance = self.time_px(time_sub((num_pos, 0), start))
            distance = distance + self.col_font_size - (rh / 2)
            cr.move_to(self.ruler_width - rw - 5, distance)
            cr.update_layout(pl)
            cr.show_layout(pl)
            num_pos += num_step_size

        distance = self.time_px(time_sub(self.cursor[0][:2], start))
        distance = distance + self.col_font_size
        cr.set_source_rgb(*self.ptheme['Cursor colour'])
        cr.move_to(0, distance)
        cr.rel_line_to(self.ruler_width, 0)
        cr.stroke()

        cr.set_source_rgb(*self.ptheme['Border colour'])
        cr.move_to(self.ruler_width - 0.5, 0)
        cr.rel_line_to(0, height)
        cr.stroke()

    def cursor_clip(self):
        if not self.pdata:
            self.cursor = ((0L, 0, 0), 0)
            self.cur_field = 0
            return
        self.cursor = (self.cursor[0][:2] + (self.cur_virtual_ord,), self.cursor[1])
        self.cur_field = self.cur_virtual_field
        if self.cursor[0] < (0, 0, 0):
            self.cursor = ((0L, 0, self.cursor[0][2]), self.cursor[1])
        if self.cursor[1] < 0:
            self.cursor = (self.cursor[0], 0)
        if self.cursor[0][:2] > self.pdata.len:
            self.cursor = (self.pdata.len + (self.cursor[0][2],), self.cursor[1])
        if self.cursor[1] > COLUMNS:
            self.cursor = (self.cursor[0], COLUMNS)
        if self.cursor[0][2] < 0:
            self.cursor = (self.cursor[0][:2] + (0,), self.cursor[1])
        if self.cursor[0][2] > 0:
            ord = 0
            if self.pdata.cols[self.cursor[1]]:
                row = [(k, v) for (k, v) in self.pdata.cols[self.cursor[1]].items()
                        if self.cursor[0][:2] == k[:2]]
                ord = min(self.cursor[0][2], len(row))
            self.cursor = (self.cursor[0][:2] + (ord,), self.cursor[1])
        if self.cur_field < 0:
            self.cur_field = 0
        if (self.pdata.cols[self.cursor[1]]
                and self.cursor[0] in self.pdata.cols[self.cursor[1]]):
            ev = self.pdata.cols[self.cursor[1]][self.cursor[0]]
            if self.cur_field >= self.event_fields[ev[0]]:
                self.cur_field = self.event_fields[ev[0]] - 1
        else:
            self.cur_field = 0

    def draw_column(self, cr, num, x, y, height, start, end):
        if self.pdata and self.pdata.cols[num]:
            visible = [(k, v) for (k, v) in self.pdata.cols[num].iteritems()
                    if start <= k[:2] <= end]
            visible.sort()
            visible_l = [(k, v) for (k, v) in visible if k[2] == 0]
            visible_r = [(k, v) for (k, v) in visible if k[2] != 0]
            prev = None
            offset = 0
            for (k, v) in visible_l:
                rights = [(l, w) for (l, w) in visible_r if l[:2] == k[:2]]
                cur_outside = False
                # Calculate row width
                row_w = self.event_width(v)
                for (l, w) in rights:
                    row_w += self.event_width(w)
                if self.cur_inserting:
                    row_w += self.col_font_size / 3
                # Make sure cursor points at or next to an existing event
                if (self.cursor[0][:2], self.cursor[1]) == (k[:2], num):
                    if self.cursor[0][2] < 0:
                        self.cursor = (self.cursor[0][:2] + (0,), self.cursor[1])
                    elif not rights and self.cursor[0][2] > 0:
                        self.cursor = (self.cursor[0][:2] + (1,), self.cursor[1])
                        row_w += self.col_font_size / 3
                    elif rights and self.cursor[0][2] > max(rights)[0][2]:
                        self.cursor = (self.cursor[0][:2] + (max(rights)[0][2] + 1,),
                                self.cursor[1])
                        row_w += self.col_font_size / 3
                off_w = 0
                cur_w = 0
                if (self.cursor[0][:2], self.cursor[1]) == (k[:2], num):
                    if self.cursor[0][2] > 0:
                        off_w = self.event_width(v)
                        for (l, w) in rights:
                            if l[2] < self.cursor[0][2]:
                                off_w += self.event_width(w)
                            elif l[2] == self.cursor[0][2]:
                                if self.cur_inserting:
                                    cur_w = self.col_font_size / 3
                                else:
                                    cur_w = self.event_width(w)
                        if cur_w == 0:
                            cur_w = self.col_font_size / 3
                            cur_outside = True
                    else:
                        if self.cur_inserting:
                            cur_w = self.col_font_size / 3
                        else:
                            cur_w = self.event_width(v)
                # Make sure cursor is visible (if it's located here)
                if (self.cursor[0][:2], self.cursor[1]) == (k[:2], num):
                    if off_w < self.event_offset:
                        self.event_offset = off_w
                    if off_w + cur_w > self.event_offset + self.col_width:
                        self.event_offset = off_w + cur_w - self.col_width
                    if off_w < self.event_offset:
                        self.event_offset = off_w
                        ce = self.pdata.cols[num][self.cursor[0]]
                        self.event_offset += self.event_field_offset(ce)
                    if self.event_offset + self.col_width > row_w:
                        self.event_offset = row_w - self.col_width
                    if self.event_offset < 0:
                        self.event_offset = 0
                cr.save()
                cr.rectangle(x, 0, self.col_width, height)
                cr.clip()
                py = y + self.col_font_size
                py = self.time_px(time_sub(k[:2], start)) + py
                prev_y = 0
                if prev:
                    prev_y = self.time_px(time_sub(prev, start)) + self.col_font_size
                if (self.cursor[0][:2], self.cursor[1]) == (k[:2], num):
                    self.draw_row(cr, k[:2], num, [(k, v)] + rights,
                            x - self.event_offset, py, prev_y, cur_outside)
                else:
                    self.draw_row(cr, k[:2], num, [(k, v)] + rights,
                            x, py, prev_y, cur_outside)
                actual_offset = 0
                if (self.cursor[0][:2], self.cursor[1]) == (k[:2], num):
                    actual_offset = self.event_offset
                if actual_offset > 0:
                    cr.set_source_rgb(1, 0.2, 0.2)
                    cr.move_to(x, py - 1)
                    cr.rel_line_to(self.col_font_size / 4, 0)
                    cr.rel_line_to(0, -self.col_font_size / 4)
                    cr.close_path()
                    cr.fill()
                if actual_offset + self.col_width < row_w:
                    cr.set_source_rgb(1, 0.2, 0.2)
                    cr.move_to(x + self.col_width, py - 1)
                    cr.rel_line_to(-self.col_font_size / 4, 0)
                    cr.rel_line_to(0, -self.col_font_size / 4)
                    cr.close_path()
                    cr.fill()
                cr.restore()
                prev = k[:2]

        if self.cursor[1] == num:
            distance = self.time_px(time_sub(self.cursor[0][:2], start))
            distance = distance + self.col_font_size
            cr.set_line_width(1.8)
            cr.set_source_rgb(*self.ptheme['Cursor colour'])
            cr.move_to(x, distance)
            cr.rel_line_to(self.col_width, 0)
            cr.stroke()
            cr.set_line_width(1)

        cr.set_source_rgb(*self.ptheme['Column header bg colour'])
        cr.rectangle(x, y, self.col_width, self.col_font_size)
        cr.fill()

        cr.set_source_rgb(*self.ptheme['Column header fg colour'])
        pl = None
        if num == 0:
            pl = self.create_pango_layout('Global')
        else:
            pl = self.create_pango_layout('%02d' % num)
        pl.set_font_description(self.col_font)
        cw, ch = pl.get_size()
        cw //= pango.SCALE
        ch //= pango.SCALE
        cr.move_to(x + 0.5 * self.col_width - 0.5 * cw, y)
        cr.update_layout(pl)
        cr.show_layout(pl)

        cr.set_source_rgb(*self.ptheme['Border colour'])
        cr.move_to(x + self.col_width - 0.5, 0)
        cr.rel_line_to(0, height)
        cr.stroke()

    def draw_row(self, cr, pos, col_num, events, x, y, prev_y, cur_outside):
        offset = 0
        for (k, v) in events:
            if self.cur_inserting and self.cursor == (k, col_num):
                pl = self.create_pango_layout('0')
                _, h = pl.get_size()
                h //= pango.SCALE
                cr.set_source_rgb(*self.ptheme['Cursor colour'])
                cr.rectangle(x + offset, y - h,
                        self.col_font_size / 3, h)
                cr.fill()
                offset += self.col_font_size / 3
            offset += self.draw_event(cr, k, col_num, v, x + offset, y, prev_y)
        if cur_outside:
            pl = self.create_pango_layout('0')
            _, h = pl.get_size()
            h //= pango.SCALE
            cr.set_source_rgb(*self.ptheme['Cursor colour'])
            cr.rectangle(x + offset, y - h,
                    self.col_font_size / 3, h)
            cr.fill()

    def draw_event(self, cr, pos, col_num, data, x, y, prev_y):
        fg_colour = self.ptheme['Event error colour']
        bg_colour = self.ptheme['Background colour']
        cur_set = self.cursor == (pos, col_num) and not self.cur_inserting
        estr, attrs, line_colour = self.event_str[data[0]](data, cur_set)
        pl = self.create_pango_layout(estr)
        pl.set_attributes(attrs)
        w, h = pl.get_size()
        w //= pango.SCALE
        h //= pango.SCALE
        cr.save()
        need_clip = prev_y + h > y and (self.cursor[0][:2],
                self.cursor[1]) != (pos[:2], col_num)
        if (self.cursor[0][:2], self.cursor[1]) != (pos[:2], col_num):
            cr.rectangle(x, prev_y + 1.5,
                    w + (self.col_font_size / 3), y - prev_y)
            cr.clip()
        cr.set_source_rgb(*bg_colour)
        cr.rectangle(x, y - h,
                w + (self.col_font_size / 3), h)
        cr.fill()
        cr.set_source_rgb(*self.ptheme['Event error colour'])
        cr.move_to(x + (self.col_font_size / 6), y - h)
        cr.update_layout(pl)
        cr.show_layout(pl)
        if need_clip:
            gr = cairo.LinearGradient(x, prev_y + 1.5, x, y)
            r, g, b = self.ptheme['Background colour']
            gr.add_color_stop_rgba(0, r, g, b, 0.8)
            gr.add_color_stop_rgba(0.3, r, g, b, 0)
            cr.rectangle(x, prev_y, w + (self.col_font_size / 3), y - prev_y)
            cr.set_source(gr)
            cr.fill()
        cr.restore()
        cr.set_source_rgb(*line_colour)
        cr.move_to(x, y)
        cr.rel_line_to(w + (self.col_font_size / 3), 0)
        cr.stroke()
        if cur_set and self.ev_note_on_got_minus == 2:
            self.ev_note_on_got_minus = 1
        return w + (self.col_font_size / 3)

    def event_width(self, event):
        etext, _, _ = self.event_str[event[0]](event)
        pl = self.create_pango_layout(etext)
        pl.set_font_description(self.col_font)
        cw, _ = pl.get_size()
        cw //= pango.SCALE
        return cw + (self.col_font_size / 3)

    def event_field_offset(self, event):
        etext, cur_offset, _ = self.event_str[event[0]](event, True, True)
        if cur_offset == 0:
            return 0
        pl = self.create_pango_layout(etext[:cur_offset])
        cw, _ = pl.get_size()
        cw //= pango.SCALE
        return cw

    def event_str_set_attrs(self, attrs, fg, starts, ends, errors, cur_set):
        r, g, b = rgb_scale(*fg)
        attrs.insert(pango.AttrForeground(r, g, b, 0, ends[-1]))
        cur_offset = 0
        for i in range(len(starts)):
            r, g, b = fg
            br, bg, bb = self.ptheme['Background colour']
            if errors[i]:
                r, g, b = self.ptheme['Event error colour']
            if cur_set and i == self.cur_field:
                cur_offset = starts[self.cur_field]
                br, bg, bb = self.ptheme['Cursor colour']
                r, g, b = colour_for_bg(r, g, b, br, bg, bb)
            r, g, b = rgb_scale(r, g, b)
            br, bg, bb = rgb_scale(br, bg, bb)
            attrs.insert(pango.AttrForeground(r, g, b,
                    starts[i], ends[i]))
            attrs.insert(pango.AttrBackground(br, bg, bb,
                    starts[i], ends[i]))
        return cur_offset

    def event_str_note_on(self, event, cur_set=False, get_cur_offset=False):
        line_colour = self.ptheme['Note On colour']
        attrs = pango.AttrList()
        if self.notes:
            starts = []
            ends = []
            errors = []
            note = ''
            if (event[1] < len(self.notes.notes)
                    and self.notes.notes[event[1]]):
                starts += [len(note)]
                note += self.notes.notes[event[1]]
                ends += [len(note)]
                errors += [False]
            else:
                starts += [len(note)]
                note += '(' + str(event[1]) + ')'
                ends += [len(note)]
                errors += [True]
            if event[2] >= 0 and False: # TODO: support for note mods
                if (event[2] < len(self.notes.note_mods)
                        and self.notes.note_mods[event[2]]):
                    starts += [ends[-1]]
                    note += self.notes.note_mods[event[2]]
                    ends += [len(note)]
                    errors += [False]
                else:
                    starts += [ends[-1]]
                    note += '(' + str(event[2]) + ')'
                    ends += [len(note)]
                    errors += [True]
            starts += [len(note)]
            ends += [starts[-1] + 1]
            if event[3] < 0 and not (cur_set and self.ev_note_on_got_minus == 2):
                ends[-1] += 1
            starts += [ends[-1] + 1]
            ends += [starts[-1] + 1]
            starts += [ends[-1]]
            ends += [starts[-1] + 1]
            errors += [False]
            if cur_set and self.ev_note_on_got_minus == 2:
                note += '-'
            else:
                note += '%x' % event[3]
            note += ' %02X' % event[4]
            if event[4] in self.instruments:
                errors += [False, False]
            else:
                errors += [True, True]
            cur_offset = self.event_str_set_attrs(attrs,
                    self.ptheme['Note On colour'], starts, ends, errors, cur_set)
            if get_cur_offset:
                attrs = cur_offset
            return (note, attrs, line_colour)
        else:
            starts = []
            ends = []
            errors = [True]
            note = ''
            starts += [len(note)]
            note += '(' + str(event[1]) + ')'
            ends += [len(note)]
#           starts += [ends[-1]]
#           note += '(' + str(event[2]) + ')'
#           ends += [len(note)]
#           errors += [True]
            starts += [ends[-1]]
            ends += [starts[-1] + 1]
            if event[3] < 0:
                ends[-1] += 1
            starts += [ends[-1] + 1]
            ends += [starts[-1] + 1]
            starts += [ends[-1]]
            ends += [starts[-1] + 1]
            note += '%x %02X' % (event[3], event[4])
            errors += [False, False, False]
            cur_offset = self.event_str_set_attrs(attrs,
                    self.ptheme['Note On colour'], starts, ends, errors, cur_set)
            if get_cur_offset:
                attrs = cur_offset
            return (note, attrs, line_colour)

    def event_str_note_off(self, event, cur_set=False, get_cur_offset=False):
        attrs = pango.AttrList()
        r, g, b = self.ptheme['Note Off colour']
        br, bg, bb = self.ptheme['Background colour']
        if cur_set:
            r, g, b = colour_for_bg(r, g, b, *self.ptheme['Cursor colour'])
            br, bg, bb = self.ptheme['Cursor colour']
        r, g, b = rgb_scale(r, g, b)
        br, bg, bb = rgb_scale(br, bg, bb)
        attrs.insert(pango.AttrForeground(r, g, b, 0, 3))
        attrs.insert(pango.AttrBackground(br, bg, bb, 0, 3))
        cur_offset = ()
        if get_cur_offset:
            return ('===', 0, self.ptheme['Note Off colour'])
        return ('===', attrs, self.ptheme['Note Off colour'])

    def do_redraw(self):
        if self.window:
            alloc = self.get_allocation()
            rect = gdk.Rectangle(alloc.x, alloc.y,
                    alloc.width, alloc.height)
            self.window.invalidate_rect(rect, True)
            self.window.process_updates(True)
        return True

    def __init__(self, engine, server, song_id, oct_adj, ins_adj):
        gtk.Widget.__init__(self)
        self.engine = engine
        self.server = server
        self.song_id = song_id

        self.pdata = None
        self.notes = None
        self.ins_adj = ins_adj
        self.base_octave_adj = oct_adj
        self.ins_num = int(self.ins_adj.get_value())
        self.base_octave = int(self.base_octave_adj.get_value())
        self.tempo = 120
        self.subsong = 0

        self.ptheme = {
            'Ruler font': 'Sans 8',
            'Column font': 'Sans 10',
            'Column width': 7,
            'Beat height': 80,
            'Background colour': (0, 0, 0),
            'Border colour': (0.6, 0.6, 0.6),
            'Ruler bg colour': (0, 0.1, 0.2),
            'Ruler fg colour': (0.7, 0.7, 0.7),
            'Column header bg colour': (0, 0.3, 0),
            'Column header fg colour': (0.8, 0.8, 0.8),
            'Cursor colour': (0.7, 0.8, 0.9),
            'Note On colour': (1, 0.9, 0.8),
            'Note Off colour': (0.7, 0.5, 0.4),
            'Note effect colour': (0.8, 0.75, 0.7),
            'Global event colour': (0.7, 0.8, 0.9),
            'General event colour': (0.7, 1, 0.7),
            'Event error colour': (1, 0, 0),
        }
        self.note_keys = {
            'z': (0L, 0L),  'q': (0L, 1L),  'i': (0L, 2L),
            's': (1L, 0L),  '2': (1L, 1L),  '9': (1L, 2L),
            'x': (2L, 0L),  'w': (2L, 1L),  'o': (2L, 2L),
            'd': (3L, 0L),  '3': (3L, 1L),  '0': (3L, 2L),
            'c': (4L, 0L),  'e': (4L, 1L),  'p': (4L, 2L),
            'v': (5L, 0L),  'r': (5L, 1L),
            'g': (6L, 0L),  '5': (6L, 1L),
            'b': (7L, 0L),  't': (7L, 1L),
            'h': (8L, 0L),  '6': (8L, 1L),
            'n': (9L, 0L),  'y': (9L, 1L),
            'j': (10L, 0L), '7': (10L, 1L),
            'm': (11L, 0L), 'u': (11L, 1L),
        }
        self.note_chord_mod = ('Shift', gdk.SHIFT_MASK)
        self.control_map = {
            ('Up', 0): self.act_up,
            ('Down', 0): self.act_down,
            ('Up', self.note_chord_mod[1]): self.act_up,
            ('Down', self.note_chord_mod[1]): self.act_down,
            ('Up', gdk.CONTROL_MASK): self.act_zoom_in,
            ('Down', gdk.CONTROL_MASK): self.act_zoom_out,
            ('Up', gdk.CONTROL_MASK | self.note_chord_mod[1]): self.act_zoom_in,
            ('Down', gdk.CONTROL_MASK | self.note_chord_mod[1]): self.act_zoom_out,
            (gdk.SCROLL_UP, gdk.CONTROL_MASK): self.act_zoom_in,
            (gdk.SCROLL_DOWN, gdk.CONTROL_MASK): self.act_zoom_out,
            ('Left', 0): self.act_left,
            ('Right', 0): self.act_right,
            ('Left', self.note_chord_mod[1]): self.act_left,
            ('Right', self.note_chord_mod[1]): self.act_right,
            ('Left', gdk.CONTROL_MASK): self.act_zoom_h_in,
            ('Right', gdk.CONTROL_MASK): self.act_zoom_h_out,
            ('Tab', 0): self.act_ch_right,
            ('Tab', gdk.SHIFT_MASK): self.act_ch_left,
            ('Page_Up', 0): self.act_bar_up,
            ('Page_Down', 0): self.act_bar_down,
            ('Insert', 0): self.act_insert_gap,
            ('Delete', 0): self.act_del_event,
            ('Insert', gdk.CONTROL_MASK): self.act_ins_col_space,
            ('Delete', gdk.CONTROL_MASK): self.act_del_col_space,
            ('comma', 0): self.act_pat_prev,
            ('period', 0): self.act_pat_next,
            ('plus', gdk.CONTROL_MASK): self.act_octave_up,
            ('minus', gdk.CONTROL_MASK): self.act_octave_down,
            ('plus', gdk.MOD1_MASK): self.act_ins_next,
            ('minus', gdk.MOD1_MASK): self.act_ins_prev,
            ('F5', 0): self.act_play_song,
            ('F6', 0): self.act_play_pat,
            ('F8', 0): self.act_stop,
        }
        self.event_map = {
            ('1', 0): self.act_ev_note_off,
        }
        self.act_event = {
            evtype.NOTE_ON: self.act_ev_note_on,
            evtype.NOTE_OFF: self.act_ev_note_off,
        }

        self.ruler_font = pango.FontDescription(self.ptheme['Ruler font'])
        self.ruler_font_size = self.ruler_font.get_size() // pango.SCALE
        self.ruler_width = self.ruler_font_size * 6
        self.col_font = pango.FontDescription(self.ptheme['Column font'])
        self.col_font_size = int(self.col_font.get_size() * 1.6) // pango.SCALE
        self.col_width = self.ptheme['Column width'] * self.col_font_size

        self.pixels_per_beat = self.ptheme['Beat height']
        # position format is ((beat, part), channel)
        self.view_corner = (time_sub((0, 0), self.px_time(self.col_font_size)), 0)
        self.cursor = ((0L, 0, 0), 0)
        self.cur_field = 0
        self.cur_virtual_col = -1
        self.cur_virtual_ord = 0
        self.cur_virtual_field = 0
        self.cur_inserting = False
        self.event_offset = 0
        self.tmove = (0, 0)
        self.snap_init_delay = 5
        self.snap_delay = self.snap_init_delay
        self.snap_state = self.snap_delay
        self.snap_step = 0.7
        self.init_cur_speed = 0.8
        self.max_cur_speed = 12
        self.cur_accel = 1.3

        self.ev_note_on_got_minus = False

        self.event_str = {
            evtype.NOTE_ON: self.event_str_note_on,
            evtype.NOTE_OFF: self.event_str_note_off,
        }
        self.event_fields = {
            evtype.NOTE_ON: 4, # note, (note_mod,) octave, ins_digit1, ins_digit2
            evtype.NOTE_OFF: 1,
        }

        self.instruments = {}


gobject.type_register(Pat_view)



# coding=utf-8


# Copyright 2008 Tomi Jylhä-Ollila
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
import gtk
import gobject

import liblo

from Pat_view import Pat_view

from Pat_helper import PATTERNS, RELTIME_FULL_PART


class Pattern(gtk.VBox):

	def set_tempo(self, tempo):
		self.pat_view.set_tempo(tempo)

	def set_subsong(self, subsong):
		self.pat_view.set_subsong(subsong)

	def pat_info(self, path, args, types):
		self.pat_view.pat_info(path, args, types)
		self.pat_spin.set_value(args[0])
		if not self.user_set_len:
			self.len_spin.get_adjustment().handler_block(self.hlen)
			self.len_biased = 10000 * args[1] + (args[2] * 10000 / RELTIME_FULL_PART)
			self.len_spin.set_value(float(self.len_biased) / 10000)
			self.len_spin.get_adjustment().handler_unblock(self.hlen)
		else:
			self.user_set_len = False

	def event_info(self, path, args, types):
		self.pat_view.event_info(path, args, types)

	def events_sent(self, path, args, types):
		self.pat_view.events_sent(path, args, types)

	def event_entry(self, widget):
		print('ch: %d, pos: %d' % (widget.channel, widget.pos))

	def note_table_info(self, path, args, types):
		self.pat_view.note_table_info(path, args, types)

	def note_info(self, path, args, types):
		self.pat_view.note_info(path, args, types)

	def note_mod_info(self, path, args, types):
		self.pat_view.note_mod_info(path, args, types)

	def notes_sent(self, path, args, types):
		self.pat_view.notes_sent(path, args, types)

	def ins_info(self, path, args, types):
		self.pat_view.ins_info(path, args, types)

	def pat_changed(self, adj):
		liblo.send(self.engine, '/kunquat/get_pattern', self.song_id, adj.value)

	def len_changed(self, adj):
		self.user_set_len = True
		self.len_biased = long(round(adj.get_value() * 10000))
		beats = self.len_biased // 10000
		rem = int((self.len_biased * RELTIME_FULL_PART // 10000) % RELTIME_FULL_PART)
		liblo.send(self.engine, '/kunquat/set_pat_len',
				self.song_id,
				int(self.pat_spin.get_value()),
				beats,
				rem)

	def octave_changed(self, adj):
		self.pat_view.set_octave(int(adj.get_value()))

	def ins_changed(self, adj):
		self.pat_view.set_ins(int(adj.get_value()))

	def __init__(self, engine, server, song_id):
		self.engine = engine
		self.server = server
		self.song_id = song_id

		gtk.VBox.__init__(self)

		self.user_set_len = False
		self.len_biased = 160000

		hb = gtk.HBox()

		label = gtk.Label('Pattern:')
		hb.pack_start(label, False, False)
		label.show()

		adj = gtk.Adjustment(0, 0, PATTERNS - 1, 1)
		self.pat_spin = gtk.SpinButton(adj)
		adj.connect('value-changed', self.pat_changed)
		hb.pack_start(self.pat_spin, False, False)
		self.pat_spin.show()

		label = gtk.Label('Length:')
		hb.pack_start(label, False, False)
		label.show()

		len_adj = gtk.Adjustment(16, 0, 65536, 0.0001, 1)
		self.len_spin = gtk.SpinButton(len_adj, digits=4)
		self.len_spin.set_snap_to_ticks(True)
		self.len_spin.set_numeric(True)
		self.hlen = len_adj.connect('value-changed', self.len_changed)
		hb.pack_start(self.len_spin, False, False)
		self.len_spin.show()
		
		oct_adj = gtk.Adjustment(4, -3, 0xc, 1)
		self.octave_spin = gtk.SpinButton(oct_adj)
		oct_adj.connect('value-changed', self.octave_changed)
		hb.pack_end(self.octave_spin, False, False)
		self.octave_spin.show()

		label = gtk.Label('Base octave:')
		hb.pack_end(label, False, False)
		label.show()

		ins_adj = gtk.Adjustment(1, 1, 255, 1)
		self.ins_spin = gtk.SpinButton(ins_adj)
		ins_adj.connect('value-changed', self.ins_changed)
		hb.pack_end(self.ins_spin, False, False)
		self.ins_spin.show()

		label = gtk.Label('Instrument:')
		hb.pack_end(label, False, False)
		label.show()

		self.pack_start(hb, False, False)
		hb.show()

		self.pat_view = Pat_view(engine, server, song_id, oct_adj, ins_adj)
		self.pack_end(self.pat_view)
		self.pat_view.show()



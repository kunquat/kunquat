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

from Pat_helper import PATTERNS


class Pattern(gtk.VBox):

	def pat_info(self, path, args, types):
		self.pat_view.pat_info(path, args, types)
		self.pat_spin.set_value(args[0])

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

	def pat_changed(self, adj):
		liblo.send(self.engine, '/kunquat/get_pattern', self.song_id, adj.value)

	def octave_changed(self, adj):
		self.pat_view.set_octave(int(adj.get_value()))

	def ins_changed(self, adj):
		self.pat_view.set_ins(int(adj.get_value()))

	def __init__(self, engine, server, song_id):
		self.engine = engine
		self.server = server
		self.song_id = song_id

		gtk.VBox.__init__(self)

		hb = gtk.HBox()

		adj = gtk.Adjustment(0, 0, PATTERNS - 1, 1)
		self.pat_spin = gtk.SpinButton(adj)
		adj.connect('value-changed', self.pat_changed)
		hb.pack_end(self.pat_spin, False, False)
		self.pat_spin.show()
		
		label = gtk.Label('Pattern:')
		hb.pack_end(label, False, False)
		label.show()

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

		liblo.send(self.engine, '/kunquat/get_note_table', self.song_id)
		liblo.send(self.engine, '/kunquat/get_pattern', self.song_id, 0)



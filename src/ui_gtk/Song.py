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

import Instruments
import Pattern
import Orders
import Note_tables


SONG_TITLE_MAX = 127
SUBSONGS_MAX = 256


class Song(gtk.VBox):

	def request_info(self):
		liblo.send(self.engine, '/kunquat/get_song_info', self.song_id)
		liblo.send(self.engine, '/kunquat/get_insts', self.song_id)
		liblo.send(self.engine, '/kunquat/get_pattern', self.song_id, 0)
		liblo.send(self.engine, '/kunquat/get_note_table', self.song_id, 0)
		liblo.send(self.engine, '/kunquat/get_orders', self.song_id)

	def song_info(self, path, args, types):
		self.mix_vol = args[1]
		self.init_subsong = args[2]
		self.title.handler_block(self.title_handler)
		self.title.set_text(args[0])
		self.title.handler_unblock(self.title_handler)

	def subsong_info(self, path, args, types):
		self.subsong_inits[args[0]] = (args[1], args[2])
		if self.cur_subsong == args[0]:
			self.pattern.set_tempo(args[1])
			if not self.user_set_tempo:
				self.tempo.get_adjustment().handler_block(self.htempo)
				self.tempo.set_value(args[1])
				self.tempo.get_adjustment().handler_unblock(self.htempo)
			else:
				self.user_set_tempo = False

	def order_info(self, path, args, types):
		self.orders.order_info(path, args, types)

	def ins_info(self, path, args, types):
		self.instruments.ins_info(path, args, types)
		self.pattern.ins_info(path, args, types)

	def ins_type_desc(self, path, args, types):
		self.instruments.ins_type_desc(path, args, types)

	def ins_type_field(self, path, args, types):
		self.instruments.ins_type_field(path, args, types)

	def pat_info(self, path, args, types):
		self.pattern.pat_info(path, args, types)

	def event_info(self, path, args, types):
		self.pattern.event_info(path, args, types)

	def events_sent(self, path, args, types):
		self.pattern.events_sent(path, args, types)

	def note_table_info(self, path, args, types):
		self.note_tables.note_table_info(path, args, types)
		self.pattern.note_table_info(path, args, types)

	def note_info(self, path, args, types):
		self.note_tables.note_info(path, args, types)
		self.pattern.note_info(path, args, types)

	def note_mod_info(self, path, args, types):
		self.note_tables.note_mod_info(path, args, types)
		self.pattern.note_mod_info(path, args, types)

	def notes_sent(self, path, args, types):
		self.note_tables.notes_sent(path, args, types)
		self.pattern.notes_sent(path, args, types)

	def player_state(self, path, args, types):
		pass

	def set_play_subsong(self, button):
		liblo.send(self.engine, '/kunquat/play_subsong',
				self.song_id,
				self.cur_subsong)

	def set_stop(self, button):
		liblo.send(self.engine, '/kunquat/stop_song', self.song_id)

	def title_entry(self, entry):
		text = entry.get_text()
		liblo.send(self.engine, '/kunquat/set_song_title', self.song_id, text)

	def subsong_changed(self, adj):
		subsong = int(adj.get_value())
		self.cur_subsong = subsong
		self.tempo.get_adjustment().handler_block(self.htempo)
		self.tempo.set_value(self.subsong_inits[subsong][0])
		self.tempo.get_adjustment().handler_unblock(self.htempo)
		self.pattern.set_tempo(self.subsong_inits[subsong][0])

	def tempo_changed(self, adj):
		tempo = adj.get_value()
		self.user_set_tempo = True
		liblo.send(self.engine, '/kunquat/set_subsong_tempo',
				self.song_id,
				self.cur_subsong,
				tempo)

	def __init__(self, engine, server, song_id):
		self.engine = engine
		self.server = server
		self.song_id = song_id

		gtk.VBox.__init__(self)

		self.subsong_inits = [(120, 0) for _ in range(SUBSONGS_MAX)]

		self.mix_vol = 0.0
		self.init_subsong = 0
		self.cur_subsong = 0

		self.user_set_tempo = False

		stop_button = gtk.Button(' Stop ')
		stop_button.connect('clicked', self.set_stop)

		info_bar = gtk.HBox()
		label = gtk.Label('Title:')
		self.title = gtk.Entry(SONG_TITLE_MAX)
		self.title_handler = self.title.connect('changed', self.title_entry)
		info_bar.pack_start(stop_button, False, False)
		stop_button.show()
		info_bar.pack_start(label, False, False)
		label.show()
		info_bar.pack_end(self.title)
		self.title.show()

		self.pack_start(info_bar, False, False)
		info_bar.show()

		subsong_bar = gtk.HBox()
		play_button = gtk.Button(' Play ')
		play_button.connect('clicked', self.set_play_subsong)
		subsong_bar.pack_start(play_button, False, False)
		play_button.show()
		label = gtk.Label('Subsong:')
		subsong_bar.pack_start(label, False, False)
		label.show()
		sub_adj = gtk.Adjustment(0, 0, SUBSONGS_MAX - 1, 1)
		self.subsong = gtk.SpinButton(sub_adj)
		sub_adj.connect('value-changed', self.subsong_changed)
		subsong_bar.pack_start(self.subsong, False, False)
		self.subsong.show()
		label = gtk.Label('Initial tempo:')
		subsong_bar.pack_start(label, False, False)
		label.show()
		tempo_adj = gtk.Adjustment(120, 16, 480, 0.1)
		self.tempo = gtk.SpinButton(tempo_adj, digits=1)
		self.tempo.set_snap_to_ticks(True)
		self.tempo.set_numeric(True)
		self.htempo = tempo_adj.connect('value-changed', self.tempo_changed)
		subsong_bar.pack_start(self.tempo, False, False)
		self.tempo.show()

		self.pack_start(subsong_bar, False, False)
		subsong_bar.show()

		nb = gtk.Notebook()

		self.instruments = Instruments.Instruments(engine, server, song_id)
		self.pattern = Pattern.Pattern(engine, server, song_id)
		self.orders = Orders.Orders(engine, server, song_id)
		self.note_tables = Note_tables.Note_tables(engine, server, song_id)

		label = gtk.Label('Pattern editor')
		nb.append_page(self.pattern, label)
		self.pattern.show()
		label.show()
		label = gtk.Label('Instruments')
		nb.append_page(self.instruments, label)
		self.instruments.show()
		label.show()
		label = gtk.Label('Order lists')
		nb.append_page(self.orders, label)
		self.orders.show()
		label.show()
		label = gtk.Label('Note tables')
		nb.append_page(self.note_tables, label)
		self.note_tables.show()
		label.show()

		self.pack_start(nb)
		nb.show()


